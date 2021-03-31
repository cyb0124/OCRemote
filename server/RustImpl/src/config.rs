use super::access::BusAccess;
use super::factory::{Factory, FactoryConfig};
use super::item::Filter;
use super::server::Server;
use super::{process::*, recipe::*, side::*, storage::*};
use std::{cell::RefCell, rc::Rc, time::Duration};

pub fn build_factory() -> Rc<RefCell<Factory>> {
    FactoryConfig {
        server: Server::new(1847),
        min_cycle_time: Duration::from_secs(1),
        log_clients: vec!["1a"],
        bus_accesses: vec![BusAccess { client: "1a", addr: "e50", side: UP }],
        backups: vec![(Filter::Label("Potato"), 32)],
    }
    .build(|factory| todo!())
}
