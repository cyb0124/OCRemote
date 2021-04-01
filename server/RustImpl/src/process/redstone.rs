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

pub struct RedstoneEmitterConfig<T> {
    pub accesses: Vec<SidedAccess>,
    pub value: T,
}

pub struct RedstoneEmitterProcess<T> {
    weak: Weak<RefCell<RedstoneEmitterProcess<T>>>,
    config: RedstoneEmitterConfig<T>,
    prev_value: Option<i32>,
}

impl<T: Fn(&Factory) -> i32 + 'static> IntoProcess for RedstoneEmitterConfig<T> {
    type Output = RedstoneEmitterProcess<T>;
    fn into_process(self, _factory: &Weak<RefCell<Factory>>) -> Rc<RefCell<Self::Output>> {
        Rc::new_cyclic(|weak| RefCell::new(Self::Output { weak: weak.clone(), config: self, prev_value: None }))
    }
}

impl<T: Fn(&Factory) -> i32 + 'static> Process for RedstoneEmitterProcess<T> {
    fn run(&self, factory: &Factory) -> AbortOnDrop<Result<(), String>> {
        let value = (self.config.value)(factory);
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

pub fn emit_when_need_item(name: &'static str, item: Filter, n_needed: i32) -> impl Fn(&Factory) -> i32 + 'static {
    move |factory| {
        if n_needed <= 0 {
            return 0;
        }
        if let Some((_, info)) = factory.search_item(&item) {
            if info.borrow().n_stored >= n_needed {
                return 0;
            }
        }
        factory.log(Print { text: format!("{}: on", name), color: 0xFF4FFF, beep: None });
        15
    }
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

impl<T, U> IntoProcess for RedstoneConditionalConfig<T, U>
where
    T: Fn(&Factory, i32) -> bool + 'static,
    U: IntoProcess,
{
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

impl<T, U> Process for RedstoneConditionalProcess<T, U>
where
    T: Fn(&Factory, i32) -> bool + 'static,
    U: Process + 'static,
{
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
            if (this.condition)(factory, value) {
                this.child.borrow().run(factory).into_future().await?
            } else if let Some(name) = this.name {
                factory.log(Print { text: format!("{}: skipped", name), color: 0xFF0000, beep: None })
            }
            Ok(())
        })
    }
}
