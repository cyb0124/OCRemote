use std::{
    cell::RefCell,
    future::Future,
    pin::Pin,
    rc::{Rc, Weak},
    task::{Context, Poll, Waker},
};
use tokio::task::{spawn_local, JoinHandle};

pub struct AbortOnDrop<T>(Option<JoinHandle<T>>);

impl<T> Drop for AbortOnDrop<T> {
    fn drop(&mut self) {
        if let Some(ref x) = self.0 {
            x.abort()
        }
    }
}

impl<T> AbortOnDrop<T> {
    pub async fn into_future(mut self) -> T {
        self.0.take().unwrap().await.unwrap()
    }
}

pub fn spawn<T: 'static>(future: impl Future<Output = T> + 'static) -> AbortOnDrop<T> {
    AbortOnDrop(Some(spawn_local(future)))
}

pub async fn join_all(tasks: Vec<AbortOnDrop<Result<(), String>>>) -> Result<(), String> {
    let mut result: Result<(), String> = Ok(());
    for task in tasks {
        if let Err(e) = task.into_future().await {
            if let Err(ref mut result) = result {
                result.push_str("; ");
                result.push_str(&e)
            } else {
                result = Err(e)
            }
        }
    }
    result
}

struct LocalOneShotState<T> {
    result: Option<Result<T, String>>,
    waker: Option<Waker>,
}

fn send<T>(state: &Weak<RefCell<LocalOneShotState<T>>>, result: Result<T, String>) {
    if let Some(state) = state.upgrade() {
        let mut state = state.borrow_mut();
        state.result = Some(result);
        if let Some(waker) = state.waker.take() {
            waker.wake()
        }
    }
}

pub struct LocalReceiver<T>(Rc<RefCell<LocalOneShotState<T>>>);

impl<T> Future for LocalReceiver<T> {
    type Output = Result<T, String>;

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

pub struct LocalSender<T>(Option<Weak<RefCell<LocalOneShotState<T>>>>);

impl<T> Drop for LocalSender<T> {
    fn drop(&mut self) {
        if let Some(ref state) = self.0 {
            send(state, Err("sender died".to_owned()))
        }
    }
}

impl<T> LocalSender<T> {
    pub fn send(self, result: Result<T, String>) {
        send(self.0.as_ref().unwrap(), result)
    }
}

pub fn make_local_one_shot<T>() -> (LocalSender<T>, LocalReceiver<T>) {
    let state = Rc::new(RefCell::new(LocalOneShotState {
        result: None,
        waker: None,
    }));
    (
        LocalSender(Some(Rc::downgrade(&state))),
        LocalReceiver(state),
    )
}

pub fn alive<T>(weak: &Weak<T>) -> Result<Rc<T>, String> {
    weak.upgrade().ok_or("node died".to_owned())
}