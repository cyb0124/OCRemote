use super::factory::{Factory, FactoryConfig};
use super::server::Server;
use super::{access::*, config_util::*, process::*, recipe::*, side::*, storage::*};
use std::{cell::RefCell, rc::Rc, time::Duration};

pub fn build_factory() -> Rc<RefCell<Factory>> {
    FactoryConfig {
        server: Server::new(1847),
        min_cycle_time: Duration::from_secs(1),
        log_clients: vec![s("1a")],
        bus_accesses: vec![SidedAccess { client: s("1a"), addr: s("538"), side: EAST }],
        backups: vec![(label("Potato"), 32)],
    }
    .build(|factory| {
        factory.add_storage(ChestConfig {
            accesses: vec![InvAccess { client: s("1a"), addr: s("538"), bus_side: EAST, inv_side: UP }],
        });
        factory.add_process(BufferedConfig {
            name: s("output"),
            accesses: vec![InvAccess { client: s("1a"), addr: s("677"), bus_side: EAST, inv_side: UP }],
            slot_filter: None,
            to_extract: extract_all(),
            recipes: vec![],
            max_recipe_inputs: 0,
            stocks: vec![],
        });
        factory.add_process(BufferedConfig {
            name: s("stock"),
            accesses: vec![InvAccess { client: s("1a"), addr: s("c65"), bus_side: WEST, inv_side: UP }],
            slot_filter: None,
            to_extract: None,
            recipes: vec![],
            max_recipe_inputs: 0,
            stocks: vec![BufferedInput::new(label("Bio Fuel"), 64), BufferedInput::new(label("Fluxed Phyto-Gro"), 64)],
        });
        factory.add_process(InputlessConfig {
            accesses: vec![InvAccess { client: s("1a"), addr: s("f59"), bus_side: EAST, inv_side: UP }],
            slot_filter: None,
            outputs: vec![Output { item: label("Cobblestone"), n_wanted: 64 }],
        });
        factory.add_process(SlottedConfig {
            name: s("manufactory"),
            accesses: vec![InvAccess { client: s("1a"), addr: s("2e2"), bus_side: WEST, inv_side: UP }],
            input_slots: vec![0],
            to_extract: None,
            recipes: vec![
                SlottedRecipe {
                    outputs: Output::new(label("Sand"), 64),
                    inputs: vec![SlottedInput::new(label("Cobblestone"), vec![(0, 1)])],
                    max_sets: 8,
                },
                SlottedRecipe {
                    outputs: Output::new(label("Niter"), 64),
                    inputs: vec![SlottedInput::new(label("Sandstone"), vec![(0, 1)])],
                    max_sets: 8,
                },
                SlottedRecipe {
                    outputs: Output::new(label("Pulverized Charcoal"), 64),
                    inputs: vec![SlottedInput::new(label("Charcoal"), vec![(0, 1)])],
                    max_sets: 8,
                },
            ],
        });
        factory.add_process(BufferedConfig {
            name: s("crafter"),
            accesses: vec![InvAccess { client: s("1a"), addr: s("0c7"), bus_side: EAST, inv_side: UP }],
            slot_filter: None,
            to_extract: None,
            recipes: vec![
                BufferedRecipe {
                    outputs: Output::new(label("Sandstone"), 64),
                    inputs: vec![BufferedInput::new(label("Sand"), 4)],
                    max_inputs: i32::MAX,
                },
                BufferedRecipe {
                    outputs: Output::new(label("Rich Phyto-Gro"), 64),
                    inputs: vec![
                        BufferedInput::new(label("Pulverized Charcoal"), 1),
                        BufferedInput::new(label("Niter"), 1),
                        BufferedInput::new(label("Rich Slag"), 1),
                    ],
                    max_inputs: i32::MAX,
                },
                BufferedRecipe {
                    outputs: Output::new(label("Compass"), 64),
                    inputs: vec![BufferedInput::new(label("Iron Ingot"), 4), BufferedInput::new(label("Redstone"), 1)],
                    max_inputs: i32::MAX,
                },
                BufferedRecipe {
                    outputs: Output::new(label("Redstone"), 64),
                    inputs: vec![BufferedInput::new(label("Redstone Essence"), 9)],
                    max_inputs: i32::MAX,
                },
            ],
            max_recipe_inputs: i32::MAX,
            stocks: vec![],
        });
        factory.add_process(SlottedConfig {
            name: s("charger"),
            accesses: vec![InvAccess { client: s("1a"), addr: s("007"), bus_side: WEST, inv_side: UP }],
            input_slots: vec![0],
            to_extract: None,
            recipes: vec![SlottedRecipe {
                outputs: Output::new(label("Fluxed Phyto-Gro"), 64),
                inputs: vec![SlottedInput::new(label("Rich Phyto-Gro"), vec![(0, 1)])],
                max_sets: i32::MAX,
            }],
        });
        factory.add_process(ScatteringConfig {
            name: s("crusher"),
            accesses: vec![InvAccess { client: s("1a"), addr: s("525"), bus_side: EAST, inv_side: UP }],
            input_slots: vec![0, 1, 2, 3, 4, 5, 6],
            to_extract: None,
            recipes: vec![ScatteringRecipe::new(
                Output::new(label("Bio Fuel"), 64),
                ScatteringInput::new(label("Potato")),
            )],
            max_per_slot: 4,
        });
        factory.add_process(ScatteringConfig {
            name: s("furnace"),
            accesses: vec![InvAccess { client: s("1a"), addr: s("346"), bus_side: WEST, inv_side: UP }],
            input_slots: vec![0, 1, 2, 3, 4, 5, 6],
            to_extract: None,
            recipes: vec![ScatteringRecipe::new(
                Output::new(label("Charcoal"), 64),
                ScatteringInput::new(label("Birch Wood")),
            )],
            max_per_slot: 4,
        });
        factory.add_process(SlottedConfig {
            name: s("phyto"),
            accesses: vec![InvAccess { client: s("1a"), addr: s("693"), bus_side: WEST, inv_side: UP }],
            input_slots: vec![0],
            to_extract: None,
            recipes: vec![
                SlottedRecipe {
                    outputs: Output::new(label("Potato"), 64),
                    inputs: vec![SlottedInput::new(label("Potato"), vec![(0, 1)]).allow_backup()],
                    max_sets: 4,
                },
                SlottedRecipe {
                    outputs: Output::new(label("Redstone Essence"), 64),
                    inputs: vec![SlottedInput::new(label("Redstone Seeds"), vec![(0, 1)])],
                    max_sets: 4,
                },
                SlottedRecipe {
                    outputs: Output::new(label("Birch Wood"), 64),
                    inputs: vec![SlottedInput::new(label("Birch Sapling"), vec![(0, 1)])],
                    max_sets: 4,
                },
            ],
        });
        factory.add_process(SlottedConfig {
            name: s("induction"),
            accesses: vec![InvAccess { client: s("1a"), addr: s("0f5"), bus_side: EAST, inv_side: UP }],
            input_slots: vec![0, 1],
            to_extract: None,
            recipes: vec![SlottedRecipe {
                outputs: Output::new(label("Rich Slag"), 64),
                inputs: vec![
                    SlottedInput::new(label("Sand"), vec![(0, 1)]),
                    SlottedInput::new(label("Compass"), vec![(1, 1)]),
                ],
                max_sets: 8,
            }],
        });
        factory.add_process(BufferedConfig {
            name: s("trash"),
            accesses: vec![InvAccess { client: s("1a"), addr: s("c65"), bus_side: WEST, inv_side: NORTH }],
            slot_filter: None,
            to_extract: None,
            recipes: vec![
                BufferedRecipe {
                    outputs: ignore_outputs(0.),
                    inputs: vec![BufferedInput::new(label("Poisonous Potato"), 1).extra_backup(64)],
                    max_inputs: i32::MAX,
                },
                BufferedRecipe {
                    outputs: ignore_outputs(0.),
                    inputs: vec![BufferedInput::new(label("Redstone Seeds"), 1).extra_backup(64)],
                    max_inputs: i32::MAX,
                },
                BufferedRecipe {
                    outputs: ignore_outputs(0.),
                    inputs: vec![BufferedInput::new(label("Birch Sapling"), 1).extra_backup(64)],
                    max_inputs: i32::MAX,
                },
            ],
            max_recipe_inputs: i32::MAX,
            stocks: vec![],
        })
    })
}
