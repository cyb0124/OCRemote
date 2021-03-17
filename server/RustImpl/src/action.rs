use super::lua_value::{Key, Table, Value};
use std::{
    cell::RefCell,
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

struct ActionState<T: Action + ?Sized> {
    result: Option<Result<T::Output, String>>,
    waker: Option<Waker>,
    action: T,
}

pub trait ActionRequest {
    fn make_request(&self) -> Value;
    fn on_fail(&mut self, reason: String);
    fn on_response(&mut self, result: Value) -> Result<(), String>;
}

impl<T: Action + ?Sized> ActionRequest for ActionState<T> {
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

pub struct ActionFuture<T: Action + ?Sized>(Rc<RefCell<ActionState<T>>>);

impl<T: Action + ?Sized> Clone for ActionFuture<T> {
    fn clone(&self) -> Self {
        ActionFuture(self.0.clone())
    }
}

impl<T: Action + ?Sized> Future for ActionFuture<T> {
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
    pub beep: Option<f64>,
}

impl Action for Print {
    type Output = ();

    fn make_request(&self) -> Value {
        let mut result = Table::default();
        result.insert(Key::S("op".to_owned()), Value::S("print".to_owned()));
        result.insert(Key::S("color".to_owned()), Value::F(self.color as f64));
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
