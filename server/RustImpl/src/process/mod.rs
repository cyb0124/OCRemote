use super::access::InvAccess;
use super::action::{ActionFuture, Call, List};
use super::factory::{Factory, Reservation};
use super::item::ItemStack;
use super::util::{alive, join_tasks, spawn};
use abort_on_drop::ChildTask;
use flexstr::{local_str, LocalStr};
use std::{
    cell::RefCell,
    iter::once,
    rc::{Rc, Weak},
};

pub trait Process: 'static {
    fn run(&self, factory: &Factory) -> ChildTask<Result<(), LocalStr>>;
}

pub trait IntoProcess {
    type Output: Process;
    fn into_process(self, factory: &Factory) -> Rc<RefCell<Self::Output>>;
}

impl<T: Process> IntoProcess for T {
    type Output = T;
    fn into_process(self, _: &Factory) -> Rc<RefCell<Self::Output>> { Rc::new(RefCell::new(self)) }
}

pub type SlotFilter = Box<dyn Fn(usize) -> bool>;
pub type ExtractFilter = Box<dyn Fn(&Factory, usize, &ItemStack) -> bool>;
pub fn extract_all() -> Option<ExtractFilter> { Some(Box::new(|_, _, _| true)) }

pub trait Inventory: 'static {
    fn get_accesses(&self) -> &Vec<InvAccess>;
    fn get_weak(&self) -> &Weak<RefCell<Self>>;
    fn get_factory(&self) -> &Weak<RefCell<Factory>>;
}

macro_rules! impl_inventory {
    ($i:ident) => {
        impl Inventory for $i {
            fn get_accesses(&self) -> &Vec<InvAccess> { &self.config.accesses }
            fn get_weak(&self) -> &Weak<RefCell<Self>> { &self.weak }
            fn get_factory(&self) -> &Weak<RefCell<Factory>> { &self.factory }
        }
    };
}

fn list_inv<T>(this: &T, factory: &Factory) -> ActionFuture<List>
where
    T: Inventory,
{
    let server = factory.borrow_server();
    let access = server.load_balance(this.get_accesses()).1;
    let action = ActionFuture::from(List { addr: access.addr.clone(), side: access.inv_side });
    server.enqueue_request_group(&access.client, vec![action.clone().into()]);
    action
}

fn extract_output<T>(this: &T, factory: &mut Factory, slot: usize, size: i32) -> ChildTask<Result<(), LocalStr>>
where
    T: Inventory,
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
                addr: access.addr.clone(),
                func: local_str!("transferItem"),
                args: vec![
                    access.inv_side.into(),
                    access.bus_side.into(),
                    size.into(),
                    (slot + 1).into(),
                    (bus_slot + 1).into(),
                ],
            });
            server.enqueue_request_group(&access.client, vec![action.clone().into()])
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
) -> ChildTask<Result<(), LocalStr>>
where
    T: Inventory,
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
                        addr: access.addr.clone(),
                        func: local_str!("transferItem"),
                        args: vec![
                            access.bus_side.into(),
                            access.inv_side.into(),
                            size.into(),
                            (bus_slot + 1).into(),
                            (inv_slot + 1).into(),
                        ],
                    });
                    server.enqueue_request_group(&access.client, vec![action.clone().into()]);
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

mod blocking_fluid_output;
mod blocking_output;
mod buffered;
mod crafting_grid;
mod fluid_slotted;
mod manual_ui;
mod misc;
mod multi_inv_slotted;
mod reactor;
mod redstone;
mod scattering;
mod slotted;
pub use blocking_fluid_output::*;
pub use blocking_output::*;
pub use buffered::*;
pub use crafting_grid::*;
pub use fluid_slotted::*;
pub use manual_ui::*;
pub use misc::*;
pub use multi_inv_slotted::*;
pub use reactor::*;
pub use redstone::*;
pub use scattering::*;
pub use slotted::*;
