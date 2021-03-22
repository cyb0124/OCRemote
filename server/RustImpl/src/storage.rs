use super::item::{Item, ItemStack};
use super::utils::AbortOnDrop;
use std::{cell::Cell, cmp::Ordering, rc::Rc};

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
    pub n_avail: Cell<i32>,
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
