use super::item::ItemStack;
use super::lua_value::{table_to_vec, vec_to_table, Table, Value};
use std::{
    cell::RefCell,
    convert::TryInto,
    future::Future,
    pin::Pin,
    rc::Rc,
    task::{Context, Poll, Waker},
};

pub trait Action {
    type Output;
    fn build_request(self) -> Value;
    fn parse_response(response: Value) -> Result<Self::Output, String>;
}

struct ActionState<T: Action> {
    result: Option<Result<T::Output, String>>,
    waker: Option<Waker>,
    action: Option<T>,
}

pub trait ActionRequest {
    fn build_request(&mut self) -> Value;
    fn on_fail(&mut self, reason: String);
    fn on_response(&mut self, result: Value) -> Result<(), String>;
}

impl<T: Action> ActionRequest for ActionState<T> {
    fn build_request(&mut self) -> Value { self.action.take().unwrap().build_request() }

    fn on_fail(&mut self, reason: String) {
        self.result = Some(Err(reason));
        if let Some(waker) = self.waker.take() {
            waker.wake()
        }
    }

    fn on_response(&mut self, result: Value) -> Result<(), String> {
        let result = T::parse_response(result);
        let ret = if let Err(ref e) = result {
            Err(e.clone())
        } else {
            Ok(())
        };
        self.result = Some(result);
        if let Some(waker) = self.waker.take() {
            waker.wake()
        }
        ret
    }
}

pub struct ActionFuture<T: Action>(Rc<RefCell<ActionState<T>>>);

impl<T: Action> Clone for ActionFuture<T> {
    fn clone(&self) -> Self { ActionFuture(self.0.clone()) }
}

impl<T: Action> Future for ActionFuture<T> {
    type Output = Result<T::Output, String>;

    fn poll(self: Pin<&mut Self>, cx: &mut Context<'_>) -> Poll<Self::Output> {
        let this = self.get_mut();
        let mut this = this.0.borrow_mut();
        if let Some(result) = this.result.take() {
            Poll::Ready(result)
        } else {
            this.waker = Some(cx.waker().clone());
            Poll::Pending
        }
    }
}

impl<T: Action> From<T> for ActionFuture<T> {
    fn from(action: T) -> Self {
        ActionFuture(Rc::new(RefCell::new(ActionState {
            result: None,
            waker: None,
            action: Some(action),
        })))
    }
}

impl<T: Action + 'static> From<ActionFuture<T>> for Rc<RefCell<dyn ActionRequest>> {
    fn from(future: ActionFuture<T>) -> Self { future.0 }
}

#[derive(Clone)]
pub struct Print {
    pub text: String,
    pub color: u32,
    pub beep: Option<f64>,
}

impl Action for Print {
    type Output = ();

    fn build_request(self) -> Value {
        let mut result = Table::new();
        result.insert("op".into(), "print".into());
        result.insert("color".into(), self.color.into());
        result.insert("text".into(), self.text.into());
        if let Some(beep) = self.beep {
            result.insert("beep".into(), beep.into());
        }
        result.into()
    }

    fn parse_response(_response: Value) -> Result<(), String> { Ok(()) }
}

pub struct List {
    pub addr: &'static str,
    pub side: u8,
}

impl Action for List {
    type Output = Vec<Option<ItemStack>>;

    fn build_request(self) -> Value {
        let mut result = Table::new();
        result.insert("op".into(), "list".into());
        result.insert("side".into(), self.side.into());
        result.insert("inv".into(), self.addr.into());
        result.into()
    }

    fn parse_response(response: Value) -> Result<Vec<Option<ItemStack>>, String> {
        table_to_vec(response.try_into()?)?
            .into_iter()
            .map(|x| match x {
                Value::N => Ok(None),
                x => ItemStack::parse(x).map(|x| Some(x)),
            })
            .collect()
    }
}

pub struct ListME {
    pub addr: &'static str,
}

impl Action for ListME {
    type Output = Vec<ItemStack>;

    fn build_request(self) -> Value {
        let mut result = Table::new();
        result.insert("op".into(), "listME".into());
        result.insert("inv".into(), self.addr.into());
        result.into()
    }

    fn parse_response(response: Value) -> Result<Vec<ItemStack>, String> {
        table_to_vec(response.try_into()?)?
            .into_iter()
            .map(|x| ItemStack::parse(x))
            .collect()
    }
}

pub struct XferME {
    pub me_addr: &'static str,
    pub me_slot: usize,
    pub filter: Value,
    pub size: i32,
    pub transposer_addr: &'static str,
    pub transposer_args: Vec<Value>,
}

impl Action for XferME {
    type Output = ();

    fn build_request(self) -> Value {
        let mut result = Table::new();
        result.insert("op".into(), "xferME".into());
        result.insert("me".into(), self.me_addr.into());
        result.insert("entry".into(), self.me_slot.into());
        result.insert("filter".into(), self.filter);
        result.insert("size".into(), self.size.into());
        result.insert("inv".into(), self.transposer_addr.into());
        result.insert("args".into(), vec_to_table(self.transposer_args).into());
        result.into()
    }

    fn parse_response(_response: Value) -> Result<(), String> { Ok(()) }
}

pub struct Call {
    pub addr: &'static str,
    pub func: &'static str,
    pub args: Vec<Value>,
}

impl Action for Call {
    type Output = Value;

    fn build_request(self) -> Value {
        let mut result = Table::new();
        result.insert("op".into(), "call".into());
        result.insert("inv".into(), self.addr.into());
        result.insert("fn".into(), self.func.into());
        result.insert("args".into(), vec_to_table(self.args).into());
        result.into()
    }

    fn parse_response(response: Value) -> Result<Value, String> { Ok(response) }
}
