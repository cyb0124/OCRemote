use super::factory::{Factory, FactoryConfig};
use super::server::Server;
use super::{access::*, item::*, process::*, recipe::*, side::*, storage::*};
use std::{cell::RefCell, rc::Rc, time::Duration};

pub fn build_factory() -> Rc<RefCell<Factory>> {
    FactoryConfig {
        server: Server::new(1847),
        min_cycle_time: Duration::from_secs(1),
        log_clients: vec!["1a"],
        bus_accesses: vec![SidedAccess { client: "1a", addr: "538", side: EAST }],
        backups: vec![(Filter::Label("Potato"), 32)],
    }
    .build(|factory| {
        factory.add_storage(ChestConfig {
            accesses: vec![InvAccess { client: "1a", addr: "538", bus_side: EAST, inv_side: UP }],
        });
        factory.add_process(BufferedConfig {
            name: "output",
            accesses: vec![InvAccess { client: "1a", addr: "677", bus_side: EAST, inv_side: UP }],
            slot_filter: None,
            to_extract: extract_all(),
            recipes: vec![],
            max_recipe_inputs: 0,
            stocks: vec![],
        });
        factory.add_process(BufferedConfig {
            name: "stock",
            accesses: vec![InvAccess { client: "1a", addr: "c65", bus_side: WEST, inv_side: UP }],
            slot_filter: None,
            to_extract: None,
            recipes: vec![],
            max_recipe_inputs: 0,
            stocks: vec![
                BufferedInput::new(Filter::Label("Bio Fuel"), 64),
                BufferedInput::new(Filter::Label("Fluxed Phyto-Gro"), 64),
            ],
        });
        factory.add_process(InputlessConfig {
            accesses: vec![InvAccess { client: "1a", addr: "f59", bus_side: EAST, inv_side: UP }],
            slot_filter: None,
            outputs: vec![Output { item: Filter::Label("Cobblestone"), n_wanted: 64 }],
        });
        factory.add_process(SlottedConfig {
            name: "manufactory",
            accesses: vec![InvAccess { client: "1a", addr: "2e2", bus_side: WEST, inv_side: UP }],
            input_slots: vec![0],
            to_extract: None,
            recipes: vec![
                SlottedRecipe {
                    outputs: vec![Output { item: Filter::Label("Sand"), n_wanted: 64 }],
                    inputs: vec![SlottedInput::new(Filter::Label("Cobblestone"), 1, vec![0])],
                    max_per_slot: 8,
                },
                SlottedRecipe {
                    outputs: vec![Output { item: Filter::Label("Niter"), n_wanted: 64 }],
                    inputs: vec![SlottedInput::new(Filter::Label("Sandstone"), 1, vec![0])],
                    max_per_slot: 8,
                },
                SlottedRecipe {
                    outputs: vec![Output { item: Filter::Label("Pulverized Charcoal"), n_wanted: 64 }],
                    inputs: vec![SlottedInput::new(Filter::Label("Charcoal"), 1, vec![0])],
                    max_per_slot: 8,
                },
            ],
        });
        factory.add_process(BufferedConfig {
            name: "crafter",
            accesses: vec![InvAccess { client: "1a", addr: "0c7", bus_side: EAST, inv_side: UP }],
            slot_filter: None,
            to_extract: None,
            recipes: vec![
                BufferedRecipe {
                    outputs: vec![Output { item: Filter::Label("Sandstone"), n_wanted: 64 }],
                    inputs: vec![BufferedInput::new(Filter::Label("Sand"), 4)],
                    max_inputs: i32::MAX,
                },
                BufferedRecipe {
                    outputs: vec![Output { item: Filter::Label("Rich Phyto-Gro"), n_wanted: 64 }],
                    inputs: vec![
                        BufferedInput::new(Filter::Label("Pulverized Charcoal"), 1),
                        BufferedInput::new(Filter::Label("Niter"), 1),
                        BufferedInput::new(Filter::Label("Rich Slag"), 1),
                    ],
                    max_inputs: i32::MAX,
                },
                BufferedRecipe {
                    outputs: vec![Output { item: Filter::Label("Compass"), n_wanted: 64 }],
                    inputs: vec![
                        BufferedInput::new(Filter::Label("Iron Ingot"), 4),
                        BufferedInput::new(Filter::Label("Redstone"), 1),
                    ],
                    max_inputs: i32::MAX,
                },
                BufferedRecipe {
                    outputs: vec![Output { item: Filter::Label("Redstone"), n_wanted: 64 }],
                    inputs: vec![BufferedInput::new(Filter::Label("Redstone Essence"), 9)],
                    max_inputs: i32::MAX,
                },
            ],
            max_recipe_inputs: i32::MAX,
            stocks: vec![],
        });
        factory.add_process(SlottedConfig {
            name: "charger",
            accesses: vec![InvAccess { client: "1a", addr: "007", bus_side: WEST, inv_side: UP }],
            input_slots: vec![0],
            to_extract: None,
            recipes: vec![SlottedRecipe {
                outputs: vec![Output { item: Filter::Label("Fluxed Phyto-Gro"), n_wanted: 64 }],
                inputs: vec![SlottedInput::new(Filter::Label("Rich Phyto-Gro"), 1, vec![0])],
                max_per_slot: i32::MAX,
            }],
        });
        factory.add_process(ScatteringConfig {
            name: "crusher",
            accesses: vec![InvAccess { client: "1a", addr: "525", bus_side: EAST, inv_side: UP }],
            input_slots: vec![0, 1, 2, 3, 4, 5, 6],
            to_extract: None,
            recipes: vec![ScatteringRecipe::new(
                vec![Output { item: Filter::Label("Bio Fuel"), n_wanted: 64 }],
                ScatteringInput::new(Filter::Label("Potato")),
            )],
            max_per_slot: 4,
        });
        factory.add_process(ScatteringConfig {
            name: "furnace",
            accesses: vec![InvAccess { client: "1a", addr: "346", bus_side: WEST, inv_side: UP }],
            input_slots: vec![0, 1, 2, 3, 4, 5, 6],
            to_extract: None,
            recipes: vec![ScatteringRecipe::new(
                vec![Output { item: Filter::Label("Charcoal"), n_wanted: 64 }],
                ScatteringInput::new(Filter::Label("Birch Wood")),
            )],
            max_per_slot: 4,
        });
        factory.add_process(SlottedConfig {
            name: "phyto",
            accesses: vec![InvAccess { client: "1a", addr: "693", bus_side: WEST, inv_side: UP }],
            input_slots: vec![0],
            to_extract: None,
            recipes: vec![
                SlottedRecipe {
                    outputs: vec![Output { item: Filter::Label("Potato"), n_wanted: 64 }],
                    inputs: vec![SlottedInput::new(Filter::Label("Potato"), 1, vec![0]).allow_backup()],
                    max_per_slot: 4,
                },
                SlottedRecipe {
                    outputs: vec![Output { item: Filter::Label("Redstone Essence"), n_wanted: 64 }],
                    inputs: vec![SlottedInput::new(Filter::Label("Redstone Seeds"), 1, vec![0])],
                    max_per_slot: 4,
                },
                SlottedRecipe {
                    outputs: vec![Output { item: Filter::Label("Birch Wood"), n_wanted: 64 }],
                    inputs: vec![SlottedInput::new(Filter::Label("Birch Sapling"), 1, vec![0])],
                    max_per_slot: 4,
                },
            ],
        });
        factory.add_process(SlottedConfig {
            name: "induction",
            accesses: vec![InvAccess { client: "1a", addr: "0f5", bus_side: EAST, inv_side: UP }],
            input_slots: vec![0, 1],
            to_extract: None,
            recipes: vec![SlottedRecipe {
                outputs: vec![Output { item: Filter::Label("Rich Slag"), n_wanted: 64 }],
                inputs: vec![
                    SlottedInput::new(Filter::Label("Sand"), 1, vec![0]),
                    SlottedInput::new(Filter::Label("Compass"), 1, vec![1]),
                ],
                max_per_slot: 8,
            }],
        });
        factory.add_process(BufferedConfig {
            name: "trash",
            accesses: vec![InvAccess { client: "1a", addr: "c65", bus_side: WEST, inv_side: NORTH }],
            slot_filter: None,
            to_extract: None,
            recipes: vec![
                BufferedRecipe {
                    outputs: vec![],
                    inputs: vec![BufferedInput::new(Filter::Label("Poisonous Potato"), 1).extra_backup(64)],
                    max_inputs: i32::MAX,
                },
                BufferedRecipe {
                    outputs: vec![],
                    inputs: vec![BufferedInput::new(Filter::Label("Redstone Seeds"), 1).extra_backup(64)],
                    max_inputs: i32::MAX,
                },
                BufferedRecipe {
                    outputs: vec![],
                    inputs: vec![BufferedInput::new(Filter::Label("Birch Sapling"), 1).extra_backup(64)],
                    max_inputs: i32::MAX,
                },
            ],
            max_recipe_inputs: i32::MAX,
            stocks: vec![],
        })
    })
}
