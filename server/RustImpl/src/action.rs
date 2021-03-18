use super::item::ItemStack;
use super::lua_value::{table_to_vec, Key, Table, Value};
use num_traits::cast::FromPrimitive;
use ordered_float::NotNan;
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
    fn make_request(&self) -> Value;
    fn parse_response(response: Value) -> Result<Self::Output, String>;
}

struct ActionState<T: Action> {
    result: Option<Result<T::Output, String>>,
    waker: Option<Waker>,
    action: T,
}

pub trait ActionRequest {
    fn make_request(&self) -> Value;
    fn on_fail(&mut self, reason: String);
    fn on_response(&mut self, result: Value) -> Result<(), String>;
}

impl<T: Action> ActionRequest for ActionState<T> {
    fn make_request(&self) -> Value {
        self.action.make_request()
    }

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
    fn clone(&self) -> Self {
        ActionFuture(self.0.clone())
    }
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
            action,
        })))
    }
}

impl<T: Action + 'static> From<ActionFuture<T>> for Rc<RefCell<dyn ActionRequest>> {
    fn from(future: ActionFuture<T>) -> Self {
        future.0
    }
}

pub struct Print {
    pub text: String,
    pub color: u32,
    pub beep: Option<NotNan<f64>>,
}

impl Action for Print {
    type Output = ();

    fn make_request(&self) -> Value {
        let mut result = Table::new();
        result.insert(Key::S("op".to_owned()), Value::S("print".to_owned()));
        result.insert(
            Key::S("color".to_owned()),
            Value::F(NotNan::from_u32(self.color).unwrap()),
        );
        result.insert(Key::S("text".to_owned()), Value::S(self.text.clone()));
        if let Some(beep) = self.beep {
            result.insert(Key::S("beep".to_owned()), Value::F(beep));
        }
        Value::T(result)
    }

    fn parse_response(_response: Value) -> Result<(), String> {
        Ok(())
    }
}

pub struct List {
    pub addr: &'static str,
    pub side: u8,
}

impl Action for List {
    type Output = Vec<Option<ItemStack>>;

    fn make_request(&self) -> Value {
        let mut result = Table::new();
        result.insert(Key::S("op".to_owned()), Value::S("list".to_owned()));
        result.insert(
            Key::S("side".to_owned()),
            Value::F(NotNan::from_u8(self.side).unwrap()),
        );
        result.insert(Key::S("inv".to_owned()), Value::S(self.addr.to_owned()));
        Value::T(result)
    }

    fn parse_response(response: Value) -> Result<Vec<Option<ItemStack>>, String> {
        table_to_vec(response.try_into()?)?
            .into_iter()
            .map(|x| match x {
                Value::N => Ok(None),
                x @ Value::T(_) => Ok(Some(ItemStack::parse(x)?)),
                x => Err(format!("invalid item: {:?}", x)),
            })
            .collect()
    }
}
