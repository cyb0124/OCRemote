use std::future::Future;
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

pub async fn join_all(
    tasks: Vec<AbortOnDrop<Result<(), Option<String>>>>,
) -> Result<(), Option<String>> {
    let mut result: Result<(), String> = Ok(());
    for task in tasks {
        if let Err(e) = task.into_future().await {
            let e = e.ok_or(None)?;
            if let Err(ref mut result) = result {
                result.push_str("; ");
                result.push_str(&e)
            } else {
                result = Err(e)
            }
        }
    }
    result.map_err(|e| Some(e))
}
