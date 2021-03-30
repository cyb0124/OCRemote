use super::access::InvAccess;
use super::action::{ActionFuture, Call};
use super::factory::Factory;
use super::item::ItemStack;
use super::util::{alive, spawn, AbortOnDrop};
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

pub trait ExtractableProcess {
    fn get_accesses(&self) -> &Vec<InvAccess>;
    fn get_weak(&self) -> &Weak<RefCell<Self>>;
    fn get_factory(&self) -> &Weak<RefCell<Factory>>;
}

macro_rules! impl_extractable_process {
    ($i:ident) => {
        impl ExtractableProcess for $i {
            fn get_accesses(&self) -> &Vec<InvAccess> { &self.config.accesses }
            fn get_weak(&self) -> &Weak<RefCell<Self>> { &self.weak }
            fn get_factory(&self) -> &Weak<RefCell<Factory>> { &self.factory }
        }
    };
}

fn extract_output<T>(this: &T, factory: &mut Factory, slot: usize, size: i32) -> AbortOnDrop<Result<(), String>>
where
    T: ExtractableProcess + 'static,
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

mod buffered;
mod scattering;
mod slotted;
pub use buffered::*;
pub use scattering::*;
pub use slotted::*;
