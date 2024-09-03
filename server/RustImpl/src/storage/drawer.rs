use super::super::access::InvAccess;
use super::super::action::{ActionFuture, Call, List};
use super::super::factory::Factory;
use super::super::item::{Filter, Item, ItemStack};
use super::super::util::{alive, spawn};
use super::{DepositResult, Extractor, IntoStorage, Provider, Storage};
use abort_on_drop::ChildTask;
use flexstr::{local_str, LocalStr};
use std::{
    cell::RefCell,
    rc::{Rc, Weak},
};

pub struct DrawerConfig {
    pub accesses: Vec<InvAccess>,
    pub filters: Vec<Filter>,
}

pub struct DrawerStorage {
    weak: Weak<RefCell<DrawerStorage>>,
    config: DrawerConfig,
    factory: Weak<RefCell<Factory>>,
}

struct DrawerExtractor {
    weak: Weak<RefCell<DrawerStorage>>,
    inv_slot: usize,
}

impl IntoStorage for DrawerConfig {
    type Output = DrawerStorage;
    fn into_storage(self, factory: &Factory) -> Rc<RefCell<Self::Output>> {
        Rc::new_cyclic(|weak| RefCell::new(Self::Output { weak: weak.clone(), config: self, factory: factory.weak.clone() }))
    }
}

impl Storage for DrawerStorage {
    fn update(&self, factory: &Factory) -> ChildTask<Result<(), LocalStr>> {
        let server = factory.borrow_server();
        let access = server.load_balance(&self.config.accesses).1;
        let action = ActionFuture::from(List { addr: access.addr.clone(), side: access.inv_side });
        server.enqueue_request_group(&access.client, vec![action.clone().into()]);
        let weak = self.weak.clone();
        spawn(async move {
            let stacks = action.await?;
            alive!(weak, this);
            upgrade_mut!(this.factory, factory);
            for (inv_slot, stack) in stacks.into_iter().enumerate() {
                if let Some(stack) = stack {
                    factory.register_stored_item(stack.item).provide(Provider {
                        priority: i32::MIN,
                        n_provided: stack.size.into(),
                        extractor: Rc::new(DrawerExtractor { weak: weak.clone(), inv_slot }),
                    });
                }
            }
            Ok(())
        })
    }

    fn cleanup(&mut self) {}

    fn deposit_priority(&mut self, item: &Rc<Item>) -> Option<i32> {
        for filter in &self.config.filters {
            if filter.apply(item) {
                return Some(i32::MAX);
            }
        }
        None
    }

    fn deposit(&mut self, factory: &Factory, stack: &ItemStack, bus_slot: usize) -> DepositResult {
        let n_deposited = stack.size;
        let server = factory.borrow_server();
        let access = server.load_balance(&self.config.accesses).1;
        let action = ActionFuture::from(Call {
            addr: access.addr.clone(),
            func: local_str!("transferItem"),
            args: vec![access.bus_side.into(), access.inv_side.into(), n_deposited.into(), (bus_slot + 1).into()],
        });
        server.enqueue_request_group(&access.client, vec![action.clone().into()]);
        let task = spawn(async move { action.await.map(|_| ()) });
        DepositResult { n_deposited, task }
    }
}

impl Extractor for DrawerExtractor {
    fn extract(&self, factory: &Factory, size: i32, bus_slot: usize) -> ChildTask<Result<(), LocalStr>> {
        upgrade!(self.weak, this);
        let server = factory.borrow_server();
        let access = server.load_balance(&this.config.accesses).1;
        let action = ActionFuture::from(Call {
            addr: access.addr.clone(),
            func: local_str!("transferItem"),
            args: vec![access.inv_side.into(), access.bus_side.into(), size.into(), (self.inv_slot + 1).into(), (bus_slot + 1).into()],
        });
        server.enqueue_request_group(&access.client, vec![action.clone().into()]);
        spawn(async move { action.await.map(|_| ()) })
    }
}
