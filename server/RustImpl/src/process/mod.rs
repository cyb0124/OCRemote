use super::access::InvAccess;
use super::action::{ActionFuture, Call, List};
use super::factory::{Factory, Reservation};
use super::item::ItemStack;
use super::util::{alive, join_tasks, spawn, AbortOnDrop};
use std::{
    cell::RefCell,
    iter::once,
    rc::{Rc, Weak},
};

pub trait Process {
    fn run(&self, factory: &Factory) -> AbortOnDrop<Result<(), String>>;
}

pub trait IntoProcess {
    fn into_process(self, factory: Weak<RefCell<Factory>>) -> Rc<RefCell<dyn Process>>;
}

pub type SlotFilter = Box<dyn Fn(usize) -> bool>;
pub type ExtractFilter = Box<dyn Fn(usize, &ItemStack) -> bool>;
pub fn extract_all() -> Option<ExtractFilter> { Some(Box::new(|_, _| true)) }

pub trait InvProcess {
    fn get_accesses(&self) -> &Vec<InvAccess>;
    fn get_weak(&self) -> &Weak<RefCell<Self>>;
    fn get_factory(&self) -> &Weak<RefCell<Factory>>;
}

macro_rules! impl_inv_process {
    ($i:ident) => {
        impl InvProcess for $i {
            fn get_accesses(&self) -> &Vec<InvAccess> { &self.config.accesses }
            fn get_weak(&self) -> &Weak<RefCell<Self>> { &self.weak }
            fn get_factory(&self) -> &Weak<RefCell<Factory>> { &self.factory }
        }
    };
}

fn list_inv<T>(this: &T, factory: &Factory) -> ActionFuture<List>
where
    T: InvProcess + 'static,
{
    let server = factory.borrow_server();
    let access = server.load_balance(this.get_accesses()).1;
    let action = ActionFuture::from(List { addr: access.addr, side: access.inv_side });
    server.enqueue_request_group(access.client, vec![action.clone().into()]);
    action
}

fn extract_output<T>(this: &T, factory: &mut Factory, slot: usize, size: i32) -> AbortOnDrop<Result<(), String>>
where
    T: InvProcess + 'static,
{
    let bus_slot = factory.bus_allocate();
    let weak = this.get_weak().clone();
    spawn(async move {
        let bus_slot = bus_slot.await?;
        let action;
        {
            alive!(weak, this);
            alive!(this.get_factory(), factory);
            let server = factory.borrow_server();
            let access = server.load_balance(this.get_accesses()).1;
            action = ActionFuture::from(Call {
                addr: access.addr,
                func: "transferItem",
                args: vec![
                    access.inv_side.into(),
                    access.bus_side.into(),
                    size.into(),
                    (slot + 1).into(),
                    (bus_slot + 1).into(),
                ],
            });
            server.enqueue_request_group(access.client, vec![action.clone().into()])
        }
        let result = action.await.map(|_| ());
        alive!(weak, this);
        upgrade_mut!(this.get_factory(), factory);
        factory.bus_deposit(once(bus_slot));
        result
    })
}

fn scattering_insert<T, U>(
    this: &T,
    factory: &mut Factory,
    reservation: Reservation,
    insertions: U,
) -> AbortOnDrop<Result<(), String>>
where
    T: InvProcess + 'static,
    U: IntoIterator<Item = (usize, i32)> + 'static,
{
    let bus_slot = factory.bus_allocate();
    let weak = this.get_weak().clone();
    spawn(async move {
        let bus_slot = bus_slot.await?;
        let task = async {
            let extraction = {
                alive!(weak, this);
                upgrade!(this.get_factory(), factory);
                reservation.extract(factory, bus_slot)
            };
            extraction.await?;
            let mut tasks = Vec::new();
            {
                alive!(weak, this);
                upgrade!(this.get_factory(), factory);
                let server = factory.borrow_server();
                for (inv_slot, size) in insertions.into_iter() {
                    let access = server.load_balance(this.get_accesses()).1;
                    let action = ActionFuture::from(Call {
                        addr: access.addr,
                        func: "transferItem",
                        args: vec![
                            access.bus_side.into(),
                            access.inv_side.into(),
                            size.into(),
                            (bus_slot + 1).into(),
                            (inv_slot + 1).into(),
                        ],
                    });
                    server.enqueue_request_group(access.client, vec![action.clone().into()]);
                    tasks.push(spawn(async move { action.await.map(|_| ()) }))
                }
            }
            join_tasks(tasks).await?;
            alive!(weak, this);
            upgrade_mut!(this.get_factory(), factory);
            factory.bus_free(bus_slot);
            Ok(())
        };
        let result = task.await;
        if result.is_err() {
            alive!(weak, this);
            upgrade_mut!(this.get_factory(), factory);
            factory.bus_deposit(once(bus_slot))
        }
        result
    })
}

mod buffered;
mod crafting_robot;
mod inputless;
mod reactor;
mod scattering;
mod slotted;
pub use buffered::*;
pub use crafting_robot::*;
pub use inputless::*;
pub use reactor::*;
pub use scattering::*;
pub use slotted::*;
