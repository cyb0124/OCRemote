#![feature(arc_new_cyclic)]
#![feature(never_type)]

mod access;
mod action;
mod async_helpers;
mod factory;
mod item;
mod lua_value;
mod process;
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
