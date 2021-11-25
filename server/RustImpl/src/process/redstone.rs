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

pub type RedstoneOutput = Box<dyn Fn(&Factory) -> i32>;
pub fn emit_when_want_item(name: &'static str, item: Filter, n_wanted: i32) -> RedstoneOutput {
    Box::new(move |factory| {
        if factory.search_n_stored(&item) < n_wanted {
            factory.log(Print { text: format!("{}: on", name), color: 0xFF4FFF, beep: None });
            15
        } else {
            0
        }
    })
}

pub struct RedstoneEmitterConfig {
    pub accesses: Vec<SidedAccess>,
    pub output: RedstoneOutput,
}

pub struct RedstoneEmitterProcess {
    weak: Weak<RefCell<RedstoneEmitterProcess>>,
    config: RedstoneEmitterConfig,
    prev_value: Option<i32>,
}

impl IntoProcess for RedstoneEmitterConfig {
    type Output = RedstoneEmitterProcess;
    fn into_process(self, _factory: &Weak<RefCell<Factory>>) -> Rc<RefCell<Self::Output>> {
        Rc::new_cyclic(|weak| RefCell::new(Self::Output { weak: weak.clone(), config: self, prev_value: None }))
    }
}

impl Process for RedstoneEmitterProcess {
    fn run(&self, factory: &Factory) -> AbortOnDrop<Result<(), String>> {
        let value = (self.config.output)(factory);
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

pub struct RedstoneConditionalConfig<T: IntoProcess> {
    pub name: Option<&'static str>,
    pub accesses: Vec<SidedAccess>,
    pub condition: Box<dyn Fn(i32) -> bool>,
    pub child: T,
}

pub struct RedstoneConditionalProcess<T: Process> {
    weak: Weak<RefCell<RedstoneConditionalProcess<T>>>,
    factory: Weak<RefCell<Factory>>,
    name: Option<&'static str>,
    accesses: Vec<SidedAccess>,
    condition: Box<dyn Fn(i32) -> bool>,
    child: Rc<RefCell<T>>,
}

impl<T: IntoProcess> IntoProcess for RedstoneConditionalConfig<T> {
    type Output = RedstoneConditionalProcess<T::Output>;
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

impl<T: Process> Process for RedstoneConditionalProcess<T> {
    fn run(&self, factory: &Factory) -> AbortOnDrop<Result<(), String>> {
        let server = factory.borrow_server();
        let access = server.load_balance(&self.accesses).1;
        let action = ActionFuture::from(Call { addr: access.addr, func: "getInput", args: vec![access.side.into()] });
        server.enqueue_request_group(access.client, vec![action.clone().into()]);
        let weak = self.weak.clone();
        spawn(async move {
            let value = call_result(action.await?)?;
            let task = {
                alive!(weak, this);
                upgrade!(this.factory, factory);
                if (this.condition)(value) {
                    this.child.borrow().run(factory)
                } else {
                    if let Some(name) = this.name {
                        factory.log(Print { text: format!("{}: skipped", name), color: 0xFF4FFF, beep: None })
                    }
                    return Ok(());
                }
            };
            task.into_future().await
        })
    }
}
