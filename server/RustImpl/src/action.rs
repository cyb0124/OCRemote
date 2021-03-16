use super::lua_value::Value;
use std::{
    cell::RefCell,
    future::Future,
    pin::Pin,
    rc::Rc,
    task::{Context, Poll, Waker},
};

pub trait Request {
    fn make_request(&self) -> Value;
}

pub struct RequestState<T: Request + ?Sized> {
    result: Option<Result<Value, String>>,
    waker: Option<Waker>,
    pub request: T,
}

pub fn complete_request(
    request: &Rc<RefCell<RequestState<dyn Request>>>,
    result: Result<Value, String>,
) {
    let mut request = request.borrow_mut();
    request.result = Some(result);
    if let Some(waker) = request.waker.take() {
        waker.wake()
    }
}

trait Action: Request {
    type Output;
    fn parse_response(response: Value) -> Result<Self::Output, String>;
}

struct ActionFuture<T: Action + ?Sized>(Rc<RefCell<RequestState<T>>>);

impl<T: Action + ?Sized> Future for ActionFuture<T> {
    type Output = Result<T::Output, String>;

    fn poll(self: Pin<&mut Self>, cx: &mut Context<'_>) -> Poll<Self::Output> {
        let this = self.get_mut();
        let mut this = this.0.borrow_mut();
        if let Some(result) = this.result.take() {
            Poll::Ready(result.and_then(T::parse_response))
        } else {
            this.waker = Some(cx.waker().clone());
            Poll::Pending
        }
    }
}
