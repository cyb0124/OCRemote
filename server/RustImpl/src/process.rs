use super::factory::Factory;
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
