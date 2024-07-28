use crate::factory::{Factory, FactoryConfig, FluidStorageConfig};
use crate::{access::*, config_util::*, process::*, recipe::*, side::*, storage::*};
use crate::{server::Server, Tui};
use std::{cell::RefCell, rc::Rc, time::Duration};

pub fn build_factory(tui: Rc<Tui>) -> Rc<RefCell<Factory>> {
    FactoryConfig {
        tui: tui.clone(),
        server: Server::new(tui, 1847),
        min_cycle_time: Duration::from_secs(1),
        log_clients: vec![s("main")],
        bus_accesses: vec![SidedAccess { client: s("main"), addr: s("3cd"), side: WEST }],
        fluid_bus_accesses: vec![FluidAccess {
            client: s("main"),
            tanks: vec![EachTank { addr: s("3cd"), side: EAST }],
        }],
        fluid_bus_capacity: 16_000,
        backups: vec![],
        fluid_backups: vec![],
    }
    .build(|factory| {
        factory.add_process(ManualUiConfig { accesses: vec![] });
        factory.add_storage(ChestConfig {
            accesses: vec![InvAccess { client: s("main"), addr: s("a53"), bus_side: EAST, inv_side: WEST }],
        });
        factory.add_fluid_storage(FluidStorageConfig {
            accesses: vec![TankAccess {
                client: s("main"),
                buses: vec![EachBusOfTank { addr: s("3cd"), bus_side: EAST, tank_side: DOWN }],
            }],
            fluid: s("water"),
        });
        factory.add_process(BlockingFluidOutputConfig {
            accesses: vec![TankAccess {
                client: s("main"),
                buses: vec![EachBusOfTank { addr: s("4e7"), bus_side: WEST, tank_side: SOUTH }],
            }],
            outputs: vec![FluidOutput { fluid: s("water"), n_wanted: 8_000 }],
        })
    })
}
