use super::access::BusAccess;
use super::factory::{Factory, FactoryConfig};
use super::item::Filter;
use super::server::Server;
use super::side::*;
use std::{cell::RefCell, rc::Rc, time::Duration};

pub fn build_factory() -> Rc<RefCell<Factory>> {
    let factory = FactoryConfig {
        server: Server::new(1847),
        min_cycle_time: Duration::from_secs(1),
        log_clients: vec!["1a"],
        bus_accesses: vec![BusAccess {
            client: "1a",
            addr: "e50",
            side: UP,
        }],
    }
    .into_factory();
    {
        let mut factory = factory.borrow_mut();
        factory.add_backup(Filter::Label("Potato"), 32);
    }
    factory
}
