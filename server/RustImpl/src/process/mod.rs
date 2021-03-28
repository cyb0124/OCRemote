mod slotted;
pub use slotted::*;

use super::factory::Factory;
use super::item::ItemStack;
use super::util::AbortOnDrop;
use std::{
    cell::RefCell,
    rc::{Rc, Weak},
};

pub trait Process {
    fn run(&self) -> AbortOnDrop<Result<(), String>>;
}

pub trait IntoProcess {
    fn into_process(self, factory: Weak<RefCell<Factory>>) -> Rc<RefCell<dyn Process>>;
}

pub type SlotFilter = Box<dyn Fn(usize) -> bool>;
pub type ExtractFilter = Box<dyn Fn(usize, &ItemStack) -> bool>;
pub fn extract_all(_slot: usize, _stack: &ItemStack) -> bool { true }
