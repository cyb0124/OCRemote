use super::super::access::MEAccess;
use super::super::action::{ActionFuture, Call, ListME, XferME};
use super::super::factory::Factory;
use super::super::item::{Item, ItemStack};
use super::super::util::{alive, spawn};
use super::{DepositResult, Extractor, IntoStorage, Provider, Storage};
use abort_on_drop::ChildTask;
use flexstr::{local_str, LocalStr};
use fnv::FnvHashMap;
use std::{
    cell::RefCell,
    rc::{Rc, Weak},
};

pub struct MEConfig {
    accesses: Vec<MEAccess>,
}

pub struct MEStorage {
    weak: Weak<RefCell<MEStorage>>,
    config: MEConfig,
    factory: Weak<RefCell<Factory>>,
    access_for_item: FnvHashMap<Rc<Item>, usize>,
}

struct MEExtractor {
    weak: Weak<RefCell<MEStorage>>,
    item: Rc<Item>,
}

impl IntoStorage for MEConfig {
    type Output = MEStorage;
    fn into_storage(self, factory: &Weak<RefCell<Factory>>) -> Rc<RefCell<Self::Output>> {
        Rc::new_cyclic(|weak| {
            RefCell::new(Self::Output {
                weak: weak.clone(),
                config: self,
                factory: factory.clone(),
                access_for_item: FnvHashMap::default(),
            })
        })
    }
}

impl Storage for MEStorage {
    fn update(&self, factory: &Factory) -> ChildTask<Result<(), LocalStr>> {
        let server = factory.borrow_server();
        let access = server.load_balance(&self.config.accesses).1;
        let action = ActionFuture::from(ListME { addr: access.me_addr.clone() });
        server.enqueue_request_group(&access.client, vec![action.clone().into()]);
        let weak = self.weak.clone();
        spawn(async move {
            let stacks = action.await?;
            alive!(weak, this);
            upgrade_mut!(this.factory, factory);
            for mut stack in stacks.into_iter() {
                Rc::get_mut(&mut stack.item).unwrap().others.remove(&"isCraftable".into());
                factory.register_stored_item(stack.item.clone()).provide(Provider {
                    priority: i32::MAX,
                    n_provided: stack.size.into(),
                    extractor: Rc::new(MEExtractor { weak: weak.clone(), item: stack.item }),
                })
            }
            Ok(())
        })
    }

    fn cleanup(&mut self) { self.access_for_item.clear() }

    fn deposit_priority(&mut self, _item: &Rc<Item>) -> Option<i32> { Some(i32::MIN) }

    fn deposit(&mut self, factory: &Factory, stack: &ItemStack, bus_slot: usize) -> DepositResult {
        let n_deposited = stack.size;
        let server = factory.borrow_server();
        let access = server.load_balance(&self.config.accesses).1;
        let action = ActionFuture::from(Call {
            addr: access.transposer_addr.clone(),
            func: local_str!("transferItem"),
            args: vec![
                access.bus_side.into(),
                access.me_side.into(),
                n_deposited.into(),
                (bus_slot + 1).into(),
                9.into(),
            ],
        });
        server.enqueue_request_group(&access.client, vec![action.clone().into()]);
        let task = spawn(async move { action.await.map(|_| ()) });
        DepositResult { n_deposited, task }
    }
}

impl Extractor for MEExtractor {
    fn extract(&self, factory: &Factory, size: i32, bus_slot: usize) -> ChildTask<Result<(), LocalStr>> {
        upgrade_mut!(self.weak, this);
        let server = factory.borrow_server();
        let accesses = &this.config.accesses;
        let access = *this.access_for_item.entry(self.item.clone()).or_insert_with(|| server.load_balance(accesses).0);
        let access = &this.config.accesses[access];
        let action = ActionFuture::from(XferME {
            me_addr: access.me_addr.clone(),
            me_slot: access.me_slot,
            filter: self.item.serialize(),
            size,
            transposer_addr: access.transposer_addr.clone(),
            transposer_args: vec![
                access.me_side.into(),
                access.bus_side.into(),
                size.into(),
                (access.me_slot + 1).into(),
                (bus_slot + 1).into(),
            ],
        });
        server.enqueue_request_group(&access.client, vec![action.clone().into()]);
        spawn(async move { action.await.map(|_| ()) })
    }
}
