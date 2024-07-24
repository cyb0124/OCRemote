use super::factory::{Factory, FactoryConfig};
use super::server::Server;
use super::{access::*, config_util::*, process::*, recipe::*, side::*, storage::*};
use std::{cell::RefCell, rc::Rc, time::Duration};

pub fn build_factory() -> Rc<RefCell<Factory>> {
    FactoryConfig {
        server: Server::new(1847),
        min_cycle_time: Duration::from_secs(1),
        log_clients: vec![s("main")],
        bus_accesses: vec![SidedAccess { client: s("main"), addr: s("03b"), side: NORTH }],
        fluid_bus_accesses: vec![],
        fluid_bus_capacity: 0,
        backups: vec![],
        fluid_backups: vec![],
    }
    .build(|factory| {
        factory.add_storage(MEConfig {
            accesses: vec![MEAccess {
                client: s("main"),
                transposer_addr: s("03b"),
                bus_side: NORTH,
                me_side: WEST,
                me_addr: s("813"),
                me_slot: 0,
            }],
        });
        factory.add_process(CraftingRobotConfig {
            name: s("craftingGrid"),
            accesses: vec![CraftingRobotAccess { client: s("crafter-1"), bus_side: FRONT }],
            recipes: vec![CraftingGridRecipe {
                outputs: Output::new(label("Stick"), 16),
                inputs: vec![CraftingGridInput::new(label("Oak Wood Planks"), vec![0, 3])],
                max_sets: 32,
                non_consumables: vec![],
            }],
        })
    })
}
