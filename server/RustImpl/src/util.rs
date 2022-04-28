use abort_on_drop::ChildTask;
use flexstr::{local_str, LocalStr};
use std::{
    cell::RefCell,
    future::Future,
    pin::Pin,
    rc::{Rc, Weak},
    task::{Context, Poll, Waker},
};
use tokio::task::spawn_local;

pub fn spawn<T: 'static>(future: impl Future<Output = T> + 'static) -> ChildTask<T> { spawn_local(future).into() }

pub async fn join_tasks(tasks: Vec<ChildTask<Result<(), LocalStr>>>) -> Result<(), LocalStr> {
    let mut result: Result<(), Vec<LocalStr>> = Ok(());
    for task in tasks {
        if let Err(e) = task.await.unwrap() {
            if let Err(ref mut result) = result {
                result.push(local_str!("; "));
                result.push(e)
            } else {
                result = Err(vec![e])
            }
        }
    }
    result.map_err(|x| x.into_iter().collect())
}

pub async fn join_outputs<T>(tasks: Vec<ChildTask<Result<T, LocalStr>>>) -> Result<Vec<T>, LocalStr> {
    let mut result: Result<Vec<T>, Vec<LocalStr>> = Ok(Vec::new());
    for task in tasks {
        match task.await.unwrap() {
            Err(e) => {
                if let Err(ref mut result) = result {
                    result.push(local_str!("; "));
                    result.push(e)
                } else {
                    result = Err(vec![e])
                }
            }
            Ok(output) => {
                if let Ok(ref mut result) = result {
                    result.push(output)
                }
            }
        }
    }
    result.map_err(|x| x.into_iter().collect())
}

struct LocalOneShotState<T> {
    result: Option<Result<T, LocalStr>>,
    waker: Option<Waker>,
}

fn send<T>(state: Weak<RefCell<LocalOneShotState<T>>>, result: Result<T, LocalStr>) {
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
    type Output = Result<T, LocalStr>;
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
        if let Some(state) = self.0.take() {
            send(state, Err(local_str!("sender died")))
        }
    }
}

impl<T> LocalSender<T> {
    pub fn send(mut self, result: Result<T, LocalStr>) { send(self.0.take().unwrap(), result) }
}

pub fn make_local_one_shot<T>() -> (LocalSender<T>, LocalReceiver<T>) {
    let state = Rc::new(RefCell::new(LocalOneShotState { result: None, waker: None }));
    (LocalSender(Some(Rc::downgrade(&state))), LocalReceiver(state))
}

macro_rules! upgrade {
    ($e:expr, $v:ident) => {
        let $v = $e.upgrade().unwrap();
        let $v = $v.borrow();
        let $v = &*$v;
    };
}

macro_rules! upgrade_mut {
    ($e:expr, $v:ident) => {
        let $v = $e.upgrade().unwrap();
        let mut $v = $v.borrow_mut();
        let $v = &mut *$v;
    };
}

pub fn alive<T>(weak: &Weak<T>) -> Result<Rc<T>, LocalStr> { weak.upgrade().ok_or_else(|| local_str!("owner died")) }

macro_rules! alive {
    ($e:expr, $v:ident) => {
        let $v = alive(&$e)?;
        let $v = $v.borrow();
        let $v = &*$v;
    };
}

macro_rules! alive_mut {
    ($e:expr, $v:ident) => {
        let $v = alive(&$e)?;
        let mut $v = $v.borrow_mut();
        let $v = &mut *$v;
    };
}
