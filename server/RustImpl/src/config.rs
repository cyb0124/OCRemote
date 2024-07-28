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
        for (fluid, bus_of_tank) in [
            ("water", EachBusOfTank { addr: s("3cd"), bus_side: EAST, tank_side: DOWN }),
            ("biomass", EachBusOfTank { addr: s("4e7"), bus_side: WEST, tank_side: DOWN }),
            ("bioethanol", EachBusOfTank { addr: s("3cd"), bus_side: EAST, tank_side: SOUTH }),
        ] {
            factory.add_fluid_storage(FluidStorageConfig {
                accesses: vec![TankAccess { client: s("main"), buses: vec![bus_of_tank] }],
                fluid: s(fluid),
            })
        }
        /*
        for (fluid, qty, bus_of_tank) in
            [("bioethanol", 16_000, EachBusOfTank { addr: s("4e7"), bus_side: WEST, tank_side: EAST })]
        {
            factory.add_process(FluidSlottedConfig {
                name: s("fluidStock"),
                input_slots: vec![],
                input_tanks: vec![vec![0]],
                accesses: vec![InvTankAccess { client: s("main"), invs: vec![], tanks: vec![vec![bus_of_tank]] }],
                to_extract: None,
                fluid_extract: None,
                strict_priority: false,
                recipes: vec![FluidSlottedRecipe {
                    outputs: ignore_outputs(1.),
                    inputs: vec![],
                    fluids: vec![FluidSlottedInput::new(s(fluid), vec![(0, 1)])],
                    max_sets: qty,
                }],
            })
        }
        */
        factory.add_process(SlottedConfig {
            name: s("output"),
            accesses: vec![InvAccess { client: s("main"), addr: s("a53"), bus_side: EAST, inv_side: NORTH }],
            input_slots: vec![],
            to_extract: extract_all(),
            strict_priority: false,
            recipes: vec![],
        });
        factory.add_process(FluidSlottedConfig {
            name: s("distillery-1"),
            input_slots: vec![],
            input_tanks: vec![vec![0]],
            accesses: vec![InvTankAccess {
                client: s("main"),
                invs: vec![],
                tanks: vec![vec![EachBusOfTank { addr: s("4e7"), bus_side: WEST, tank_side: NORTH }]],
            }],
            to_extract: None,
            fluid_extract: fluid_extract_slots(|_, i| i == 1),
            strict_priority: false,
            recipes: vec![FluidSlottedRecipe {
                outputs: FluidOutput::new(s("bioethanol"), 8_000),
                inputs: vec![],
                fluids: vec![FluidSlottedInput::new(s("biomass"), vec![(0, 40)])],
                max_sets: 64,
            }],
        });
        factory.add_process(FluidSlottedConfig {
            name: s("brewery"),
            input_slots: vec![vec![5]],
            input_tanks: vec![vec![0]],
            accesses: vec![InvTankAccess {
                client: s("main"),
                invs: vec![EachInvAccess { addr: s("3cd"), bus_side: WEST, inv_side: NORTH }],
                tanks: vec![vec![EachBusOfTank { addr: s("3cd"), bus_side: EAST, tank_side: NORTH }]],
            }],
            to_extract: None,
            fluid_extract: fluid_extract_slots(|_, i| i == 1),
            strict_priority: false,
            recipes: vec![FluidSlottedRecipe {
                outputs: FluidOutput::new(s("biomass"), 8_000),
                inputs: vec![MultiInvSlottedInput::new(label("Potato"), vec![(0, 5, 1)])],
                fluids: vec![FluidSlottedInput::new(s("water"), vec![(0, 40)])],
                max_sets: 8,
            }],
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
