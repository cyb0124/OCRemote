#![feature(arc_new_cyclic)]

mod access;
mod action;
mod factory;
mod item;
mod lua_value;
mod process;
mod recipe;
mod server;
mod side;
mod storage;
mod util;

use access::BusAccess;
use factory::Factory;
use item::Filter;
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
            vec!["1a"],
            vec![BusAccess {
                client: "1a",
                addr: "e50",
                side: UP,
            }],
        );
        {
            let mut factory = factory.borrow_mut();
            factory.add_backup(Filter::Label("Potato"), 32);
        }
        ctrl_c().await.unwrap()
    });
    tasks.await
}
