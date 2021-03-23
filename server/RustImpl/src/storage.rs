use super::access::InvAccess;
use super::action::{ActionFuture, List};
use super::factory::Factory;
use super::item::{Filter, Item, ItemStack};
use super::util::{alive, spawn, AbortOnDrop};
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
    fn deposit(&mut self, stack: &ItemStack, bus_slot: usize) -> DepositResult;
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

struct Chest {
    accesses: Vec<InvAccess>,
    content: Vec<Option<ItemStack>>,
    slot_to_deposit: usize,
}

struct Drawer {
    accesses: Vec<InvAccess>,
    filters: Vec<Filter>,
}

/*
impl Storage for Drawer {
    fn update(&self) -> AbortOnDrop<Result<(), String>> {
        let server = factory.server.borrow();
        let access = server.load_balance(&self.accesses);
        let action = ActionFuture::from(List {
            addr: access.addr,
            side: access.inv_side,
        });
        server.enqueue_request_group(access.client, vec![action.clone().into()]);
        let factory = factory.weak.clone();
        spawn(async move {
            let stacks = action.await?;
            let factory = alive(&factory)?;
            let factory = factory.borrow_mut();
            for (slot, stack) in stacks.into_iter().enumerate() {
                if let Some(stack) = stack {
                    factory.register_stored_item(stack.item).provide();
                }
            }
            Ok(())
        })
    }

    fn cleanup(&mut self) {}

    fn deposit_priority(&mut self, item: &Item) -> Option<i32> {}

    fn deposit(&mut self, stack: &ItemStack, bus_slot: usize) -> DepositResult {}
}
*/
