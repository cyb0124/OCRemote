#![feature(arc_new_cyclic)]

mod access;
mod action;
mod factory;
mod item;
mod lua_value;
mod server;
mod side;
mod storage;

use access::BusAccess;
use factory::Factory;
use server::Server;
use side::*;
use std::time::Duration;
use tokio::{signal::ctrl_c, task::LocalSet};

#[tokio::main(flavor = "current_thread")]
async fn main() {
    let tasks = LocalSet::new();
    tasks.spawn_local(async {
        let factory = Factory::new(
            Server::new(1847),
            Duration::from_secs(1),
            vec!["south"],
            vec![BusAccess {
                client: "south",
                addr: "127",
                side: UP,
            }],
        );
        ctrl_c().await.unwrap()
    });
    tasks.await
}
