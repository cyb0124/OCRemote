#![feature(arc_new_cyclic)]

mod action;
mod factory;
mod item;
mod lua_value;
mod server;
mod side;

use factory::Factory;
use server::Server;
use std::time::Duration;
use tokio::{signal::ctrl_c, task::LocalSet};

#[tokio::main(flavor = "current_thread")]
async fn main() {
    let tasks = LocalSet::new();
    tasks.spawn_local(async {
        let factory = Factory::new(Server::new(1847), Duration::from_secs(1), vec!["center"]);
        ctrl_c().await.unwrap()
    });
    tasks.await
}
