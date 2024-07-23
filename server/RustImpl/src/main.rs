#[macro_use]
pub mod util;
#[macro_use]
pub mod recipe;
pub mod access;
pub mod action;
pub mod config;
pub mod config_util;
pub mod factory;
pub mod item;
pub mod lua_value;
pub mod process;
pub mod server;
pub mod side;
pub mod storage;

use config::build_factory;
use tokio::{signal::ctrl_c, task::LocalSet};

#[tokio::main(flavor = "current_thread")]
async fn main() {
    let tasks = LocalSet::new();
    tasks.spawn_local(async {
        let _factory = build_factory();
        ctrl_c().await.unwrap()
    });
    tasks.await
}
