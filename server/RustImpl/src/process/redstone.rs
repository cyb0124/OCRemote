use super::super::access::SidedAccess;
use super::super::action::{ActionFuture, Call, Print};
use super::super::factory::Factory;
use super::super::item::Filter;
use super::super::lua_value::call_result;
use super::super::util::{alive, spawn, AbortOnDrop};
use super::{IntoProcess, Process};
use std::{
    cell::RefCell,
    rc::{Rc, Weak},
};

pub trait RedstoneOutput {
    fn get_value(&self, factory: &Factory) -> i32;
}

impl<T: Fn(&Factory) -> i32> RedstoneOutput for T {
    fn get_value(&self, factory: &Factory) -> i32 { self(factory) }
}

pub struct EmitWhenWantItem {
    pub name: &'static str,
    pub item: Filter,
    pub n_wanted: i32,
}

impl RedstoneOutput for EmitWhenWantItem {
    fn get_value(&self, factory: &Factory) -> i32 {
        if factory.search_n_stored(&self.item) < self.n_wanted {
            factory.log(Print { text: format!("{}: on", self.name), color: 0xFF4FFF, beep: None });
            15
        } else {
            0
        }
    }
}

pub struct RedstoneEmitterConfig<T> {
    pub accesses: Vec<SidedAccess>,
    pub output: T,
}

pub struct RedstoneEmitterProcess<T> {
    weak: Weak<RefCell<RedstoneEmitterProcess<T>>>,
    config: RedstoneEmitterConfig<T>,
    prev_value: Option<i32>,
}

impl<T: RedstoneOutput + 'static> IntoProcess for RedstoneEmitterConfig<T> {
    type Output = RedstoneEmitterProcess<T>;
    fn into_process(self, _factory: &Weak<RefCell<Factory>>) -> Rc<RefCell<Self::Output>> {
        Rc::new_cyclic(|weak| RefCell::new(Self::Output { weak: weak.clone(), config: self, prev_value: None }))
    }
}

impl<T: RedstoneOutput + 'static> Process for RedstoneEmitterProcess<T> {
    fn run(&self, factory: &Factory) -> AbortOnDrop<Result<(), String>> {
        let value = self.config.output.get_value(factory);
        if Some(value) == self.prev_value {
            spawn(async { Ok(()) })
        } else {
            let server = factory.borrow_server();
            let access = server.load_balance(&self.config.accesses).1;
            let action = ActionFuture::from(Call {
                addr: access.addr,
                func: "setOutput",
                args: vec![access.side.into(), value.into()],
            });
            server.enqueue_request_group(access.client, vec![action.clone().into()]);
            let weak = self.weak.clone();
            spawn(async move {
                action.await?;
                alive_mut!(weak, this);
                this.prev_value = Some(value);
                Ok(())
            })
        }
    }
}

pub trait RedstoneCondition {
    fn should_run(&self, factory: &Factory, value: i32) -> bool;
}

impl<T: Fn(&Factory, i32) -> bool> RedstoneCondition for T {
    fn should_run(&self, factory: &Factory, value: i32) -> bool { self(factory, value) }
}

pub struct RedstoneConditionalConfig<T, U> {
    pub name: Option<&'static str>,
    pub accesses: Vec<SidedAccess>,
    pub condition: T,
    pub child: U,
}

pub struct RedstoneConditionalProcess<T, U> {
    weak: Weak<RefCell<RedstoneConditionalProcess<T, U>>>,
    factory: Weak<RefCell<Factory>>,
    name: Option<&'static str>,
    accesses: Vec<SidedAccess>,
    condition: T,
    child: Rc<RefCell<U>>,
}

impl<T: RedstoneCondition + 'static, U: IntoProcess> IntoProcess for RedstoneConditionalConfig<T, U> {
    type Output = RedstoneConditionalProcess<T, U::Output>;
    fn into_process(self, factory: &Weak<RefCell<Factory>>) -> Rc<RefCell<Self::Output>> {
        Rc::new_cyclic(|weak| {
            RefCell::new(Self::Output {
                weak: weak.clone(),
                factory: factory.clone(),
                name: self.name,
                accesses: self.accesses,
                condition: self.condition,
                child: self.child.into_process(factory),
            })
        })
    }
}

impl<T: RedstoneCondition + 'static, U: Process + 'static> Process for RedstoneConditionalProcess<T, U> {
    fn run(&self, factory: &Factory) -> AbortOnDrop<Result<(), String>> {
        let server = factory.borrow_server();
        let access = server.load_balance(&self.accesses).1;
        let action = ActionFuture::from(Call { addr: access.addr, func: "getInput", args: vec![access.side.into()] });
        server.enqueue_request_group(access.client, vec![action.clone().into()]);
        let weak = self.weak.clone();
        spawn(async move {
            let value = call_result(action.await?)?;
            alive!(weak, this);
            upgrade!(this.factory, factory);
            if this.condition.should_run(factory, value) {
                this.child.borrow().run(factory).into_future().await?
            } else if let Some(name) = this.name {
                factory.log(Print { text: format!("{}: skipped", name), color: 0xFF0000, beep: None })
            }
            Ok(())
        })
    }
}
