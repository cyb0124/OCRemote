use super::async_helpers::AbortOnDrop;
use super::item::{Item, ItemStack};

pub struct DepositResult {
    n_deposited: i32,
    task: AbortOnDrop<Result<(), Option<String>>>,
}

pub trait Storage {
    fn update(&self) -> AbortOnDrop<Result<(), Option<String>>>;
    fn cleanup(&mut self);
    fn deposit_priority(&mut self, item: &Item) -> Option<i32>;
    fn deposit(&mut self, stack: &ItemStack, bus_slot: usize) -> DepositResult;
}
