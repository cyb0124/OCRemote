use super::factory::Factory;
use super::item::{Item, ItemStack};
use abort_on_drop::ChildTask;
use flexstr::LocalStr;
use std::{
    cell::{Cell, RefCell},
    cmp::Ordering,
    rc::Rc,
};

pub struct DepositResult {
    pub n_deposited: i32,
    pub task: ChildTask<Result<(), LocalStr>>,
}

pub trait Storage: 'static {
    fn update(&self, factory: &Factory) -> ChildTask<Result<(), LocalStr>>;
    fn cleanup(&mut self);
    fn deposit_priority(&mut self, item: &Rc<Item>) -> Option<i32>;
    fn deposit(&mut self, factory: &Factory, stack: &ItemStack, bus_slot: usize) -> DepositResult;
}

pub trait IntoStorage {
    type Output: Storage;
    fn into_storage(self, factory: &Factory) -> Rc<RefCell<Self::Output>>;
}

pub trait Extractor {
    fn extract(&self, factory: &Factory, size: i32, bus_slot: usize) -> ChildTask<Result<(), LocalStr>>;
}

pub struct Provider {
    priority: i32,
    pub n_provided: Cell<i32>,
    pub extractor: Rc<dyn Extractor>,
}

impl PartialEq<Provider> for Provider {
    fn eq(&self, other: &Self) -> bool { self.priority == other.priority }
}

impl Eq for Provider {}

impl PartialOrd<Provider> for Provider {
    fn partial_cmp(&self, other: &Self) -> Option<Ordering> { Some(self.cmp(other)) }
}

impl Ord for Provider {
    fn cmp(&self, other: &Self) -> Ordering { self.priority.cmp(&other.priority) }
}

mod chest;
mod drawer;
mod me;
pub use chest::*;
pub use drawer::*;
pub use me::*;
