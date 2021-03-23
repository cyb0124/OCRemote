use super::access::InvAccess;
use super::action::{ActionFuture, Call, List};
use super::factory::Factory;
use super::item::{Filter, Item, ItemStack};
use super::lua_value::Value;
use super::util::{alive, spawn, AbortOnDrop};
use num_traits::cast::FromPrimitive;
use ordered_float::NotNan;
use std::{
    cell::{Cell, RefCell},
    cmp::Ordering,
    rc::{Rc, Weak},
};

pub struct DepositResult {
    pub n_deposited: i32,
    pub task: AbortOnDrop<Result<(), String>>,
}

pub trait Storage {
    fn update(&self) -> AbortOnDrop<Result<(), String>>;
    fn cleanup(&mut self);
    fn deposit_priority(&mut self, item: &Item) -> Option<i32>;
    fn deposit(&self, stack: &ItemStack, bus_slot: usize) -> DepositResult;
}

pub trait IntoStorage {
    fn into_storage(self, factory: Weak<RefCell<Factory>>) -> Rc<RefCell<dyn Storage>>;
}

pub trait Extractor {
    fn extract(&self, size: i32, bus_slot: usize) -> AbortOnDrop<Result<(), String>>;
}

pub struct Provider {
    priority: i32,
    pub n_provided: Cell<i32>,
    pub extractor: Rc<dyn Extractor>,
}

impl PartialEq<Provider> for Provider {
    fn eq(&self, other: &Self) -> bool {
        self.priority == other.priority
    }
}

impl Eq for Provider {}

impl PartialOrd<Provider> for Provider {
    fn partial_cmp(&self, other: &Self) -> Option<Ordering> {
        Some(self.cmp(other))
    }
}

impl Ord for Provider {
    fn cmp(&self, other: &Self) -> Ordering {
        self.priority.cmp(&other.priority)
    }
}

pub struct ChestConfig {
    pub accesses: Vec<InvAccess>,
    pub content: Vec<Option<ItemStack>>,
    pub slot_to_deposit: usize,
}

pub struct DrawerConfig {
    pub accesses: Vec<InvAccess>,
    pub filters: Vec<Filter>,
}

struct DrawerStorage {
    weak: Weak<RefCell<DrawerStorage>>,
    config: DrawerConfig,
    factory: Weak<RefCell<Factory>>,
}

struct DrawerExtractor {
    weak: Weak<RefCell<DrawerStorage>>,
    slot: usize,
}

impl IntoStorage for DrawerConfig {
    fn into_storage(self, factory: Weak<RefCell<Factory>>) -> Rc<RefCell<dyn Storage>> {
        Rc::new_cyclic(|weak| {
            RefCell::new(DrawerStorage {
                weak: weak.clone(),
                config: self,
                factory,
            })
        })
    }
}

impl Storage for DrawerStorage {
    fn update(&self) -> AbortOnDrop<Result<(), String>> {
        let weak = self.weak.clone();
        spawn(async move {
            let action;
            {
                let this = alive(&weak)?;
                let this = this.borrow();
                let factory = alive(&this.factory)?;
                let factory = factory.borrow();
                let server = factory.borrow_server();
                let access = server.load_balance(&this.config.accesses);
                action = ActionFuture::from(List {
                    addr: access.addr,
                    side: access.inv_side,
                });
                server.enqueue_request_group(access.client, vec![action.clone().into()]);
            }
            let stacks = action.await?;
            let this = alive(&weak)?;
            let this = this.borrow();
            let factory = alive(&this.factory)?;
            let mut factory = factory.borrow_mut();
            for (slot, stack) in stacks.into_iter().enumerate() {
                if let Some(stack) = stack {
                    factory.register_stored_item(stack.item).provide(Provider {
                        priority: i32::MIN,
                        n_provided: stack.size.into(),
                        extractor: Rc::new(DrawerExtractor {
                            weak: weak.clone(),
                            slot,
                        }),
                    });
                }
            }
            Ok(())
        })
    }

    fn cleanup(&mut self) {}

    fn deposit_priority(&mut self, item: &Item) -> Option<i32> {
        for filter in &self.config.filters {
            if filter.apply(item) {
                return Some(i32::MAX);
            }
        }
        None
    }

    fn deposit(&self, stack: &ItemStack, bus_slot: usize) -> DepositResult {
        let weak = self.weak.clone();
        let n_deposited = stack.size;
        let task = spawn(async move {
            let action;
            {
                let this = alive(&weak)?;
                let this = this.borrow();
                let factory = alive(&this.factory)?;
                let factory = factory.borrow();
                let server = factory.borrow_server();
                let access = server.load_balance(&this.config.accesses);
                action = ActionFuture::from(Call {
                    addr: access.addr,
                    func: "transferItem",
                    args: vec![
                        access.bus_side.into(),
                        access.inv_side.into(),
                        n_deposited.into(),
                        (bus_slot + 1).into(),
                    ],
                });
                server.enqueue_request_group(access.client, vec![action.clone().into()]);
            }
            action.await.map(|_| ())
        });
        DepositResult { n_deposited, task }
    }
}

impl Extractor for DrawerExtractor {
    fn extract(&self, size: i32, bus_slot: usize) -> AbortOnDrop<Result<(), String>> {
        todo!()
    }
}
