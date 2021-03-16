#![feature(arc_new_cyclic)]
#![feature(once_cell)]

mod action;
mod lua_value;
mod server;
mod side;

use server::Server;
use tokio::signal::ctrl_c;
use tokio::task::LocalSet;

#[tokio::main(flavor = "current_thread")]
async fn main() {
    let tasks = LocalSet::new();
    tasks.spawn_local(async {
        let server = Server::new(1847);
        ctrl_c().await.unwrap()
    });
    tasks.await
}
