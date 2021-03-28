#![feature(arc_new_cyclic)]

#[macro_use]
mod util;
#[macro_use]
mod recipe;
mod access;
mod action;
mod config;
mod factory;
mod item;
mod lua_value;
mod process;
mod server;
mod side;
mod storage;

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
