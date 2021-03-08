#![feature(arc_new_cyclic)]

mod server;

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
