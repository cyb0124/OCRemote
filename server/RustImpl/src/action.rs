use super::lua_value::Value;
use std::{
    cell::RefCell,
    future::Future,
    pin::Pin,
    rc::Rc,
    task::{Context, Poll, Waker},
};

trait Action {
    type Output;
    fn make_request(&self) -> Value;
    fn parse_response(response: Value) -> Result<Self::Output, String>;
}

struct ActionState<T: Action + ?Sized> {
    result: Option<Result<T::Output, String>>,
    waker: Option<Waker>,
    request: T,
}

pub trait ActionRequest {
    fn make_request(&self) -> Value;
    fn on_fail(&mut self, reason: String);
    fn on_response(&mut self, result: Value) -> Result<(), String>;
}

impl<T: Action + ?Sized> ActionRequest for ActionState<T> {
    fn make_request(&self) -> Value {
        self.request.make_request()
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

struct ActionFuture<T: Action + ?Sized>(Rc<RefCell<ActionState<T>>>);

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
