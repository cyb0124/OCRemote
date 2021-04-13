use super::factory::{Factory, FactoryConfig};
use super::server::Server;
use super::{access::*, item::*, process::*, recipe::*, side::*, storage::*};
use std::{cell::RefCell, rc::Rc, time::Duration};

pub fn build_factory() -> Rc<RefCell<Factory>> {
    FactoryConfig {
        server: Server::new(1848),
        min_cycle_time: Duration::from_secs(1),
        log_clients: vec!["1a"],
        bus_accesses: vec![
            SidedAccess { client: "1a", addr: "0a7", side: EAST },
            SidedAccess { client: "1b", addr: "0a7", side: EAST },
            SidedAccess { client: "1c", addr: "0a7", side: EAST },
        ],
        backups: vec![(Filter::Label("Potato"), 32)],
    }
    .build(|factory| {
        factory.add_storage(ChestConfig {
            accesses: vec![
                InvAccess { client: "1a", addr: "0a7", inv_side: DOWN, bus_side: EAST },
                InvAccess { client: "1b", addr: "0a7", inv_side: DOWN, bus_side: EAST },
                InvAccess { client: "1c", addr: "0a7", inv_side: DOWN, bus_side: EAST },
            ],
        });
        factory.add_storage(ChestConfig {
            accesses: vec![
                InvAccess { client: "1a", addr: "0a7", inv_side: UP, bus_side: EAST },
                InvAccess { client: "1b", addr: "0a7", inv_side: UP, bus_side: EAST },
                InvAccess { client: "1c", addr: "0a7", inv_side: UP, bus_side: EAST },
            ],
        });
        factory.add_process(SlottedConfig {
            name: "bioGenerator",
            accesses: vec![
                InvAccess { client: "1a", addr: "0a7", inv_side: WEST, bus_side: EAST },
                InvAccess { client: "1b", addr: "0a7", inv_side: WEST, bus_side: EAST },
                InvAccess { client: "1c", addr: "0a7", inv_side: WEST, bus_side: EAST },
            ],
            input_slots: vec![0],
            to_extract: None,
            recipes: vec![SlottedRecipe {
                outputs: vec![],
                inputs: vec![SlottedInput::new(Filter::Label("Bio Fuel"), 2, vec![0])],
                max_per_slot: 8,
            }],
        });
        factory.add_process(InputlessConfig {
            accesses: vec![
                InvAccess { client: "1a", addr: "42c", inv_side: UP, bus_side: WEST },
                InvAccess { client: "1b", addr: "42c", inv_side: UP, bus_side: WEST },
                InvAccess { client: "1c", addr: "42c", inv_side: UP, bus_side: WEST },
            ],
            slot_filter: None,
            outputs: vec![Output { item: Filter::Label("Cobblestone"), n_wanted: 64 }],
        });
        factory.add_process(ScatteringConfig {
            name: "furnace",
            accesses: vec![
                InvAccess { client: "1a", addr: "42c", inv_side: DOWN, bus_side: WEST },
                InvAccess { client: "1b", addr: "42c", inv_side: DOWN, bus_side: WEST },
                InvAccess { client: "1c", addr: "42c", inv_side: DOWN, bus_side: WEST },
            ],
            input_slots: vec![0, 1, 2, 3, 4, 5, 6],
            to_extract: None,
            recipes: vec![
                ScatteringRecipe::new(
                    vec![Output { item: Filter::Label("Baked Potato"), n_wanted: 64 }],
                    ScatteringInput::new(Filter::Label("Potato")),
                ),
                ScatteringRecipe::new(
                    vec![Output { item: Filter::Label("Charcoal"), n_wanted: 64 }],
                    ScatteringInput::new(Filter::Label("Oak Wood")),
                ),
                ScatteringRecipe::new(
                    vec![Output { item: Filter::Label("Graphite Ingot"), n_wanted: 64 }],
                    ScatteringInput::new(Filter::Label("Charcoal")),
                ),
                ScatteringRecipe::new(
                    vec![Output { item: Filter::Label("Stone"), n_wanted: 64 }],
                    ScatteringInput::new(Filter::Label("Cobblestone")),
                ),
                ScatteringRecipe::new(
                    vec![Output { item: Filter::Label("Glass"), n_wanted: 64 }],
                    ScatteringInput::new(Filter::Label("Sand")),
                ),
                ScatteringRecipe::new(
                    vec![Output { item: Filter::Label("Aluminum Ingot"), n_wanted: 64 }],
                    ScatteringInput::new(Filter::Label("Pulverized Aluminum")),
                ),
                ScatteringRecipe::new(
                    vec![Output { item: Filter::Label("Iron Ingot"), n_wanted: 64 }],
                    ScatteringInput::new(Filter::Label("Pulverized Iron")),
                ),
                ScatteringRecipe::new(
                    vec![Output { item: Filter::Label("Lead Ingot"), n_wanted: 64 }],
                    ScatteringInput::new(Filter::Label("Pulverized Lead")),
                ),
                ScatteringRecipe::new(
                    vec![Output { item: Filter::Label("Silver Ingot"), n_wanted: 64 }],
                    ScatteringInput::new(Filter::Label("Pulverized Silver")),
                ),
                ScatteringRecipe::new(
                    vec![Output { item: Filter::Label("Gold Ingot"), n_wanted: 64 }],
                    ScatteringInput::new(Filter::Label("Pulverized Gold")),
                ),
                ScatteringRecipe::new(
                    vec![Output { item: Filter::Label("Copper Ingot"), n_wanted: 64 }],
                    ScatteringInput::new(Filter::Label("Pulverized Copper")),
                ),
                ScatteringRecipe::new(
                    vec![Output { item: Filter::Label("Copper Tin"), n_wanted: 64 }],
                    ScatteringInput::new(Filter::Label("Pulverized Tin")),
                ),
            ],
            max_per_slot: 4,
        });
        factory.add_process(SlottedConfig {
            name: "output",
            accesses: vec![
                InvAccess { client: "1a", addr: "42c", inv_side: EAST, bus_side: WEST },
                InvAccess { client: "1b", addr: "42c", inv_side: EAST, bus_side: WEST },
                InvAccess { client: "1c", addr: "42c", inv_side: EAST, bus_side: WEST },
            ],
            input_slots: vec![],
            to_extract: extract_all(),
            recipes: vec![],
        });
        factory.add_process(BufferedConfig {
            name: "autoCompressor",
            accesses: vec![
                InvAccess { client: "1a", addr: "8da", inv_side: WEST, bus_side: EAST },
                InvAccess { client: "1b", addr: "8da", inv_side: WEST, bus_side: EAST },
                InvAccess { client: "1c", addr: "8da", inv_side: WEST, bus_side: EAST },
            ],
            slot_filter: Some(Box::new(|slot| slot < 12)),
            to_extract: None,
            recipes: vec![
                BufferedRecipe {
                    outputs: vec![Output { item: Filter::Label("Compressed Sand"), n_wanted: 64 }],
                    inputs: vec![BufferedInput::new(Filter::Label("Sand"), 9)],
                    max_inputs: i32::MAX,
                },
                BufferedRecipe {
                    outputs: vec![Output { item: Filter::Label("Redstone"), n_wanted: 64 }],
                    inputs: vec![BufferedInput::new(Filter::Label("Redstone Essence"), 9)],
                    max_inputs: i32::MAX,
                },
                BufferedRecipe {
                    outputs: vec![Output { item: Filter::Label("Aluminum Ore"), n_wanted: 64 }],
                    inputs: vec![BufferedInput::new(Filter::Label("Aluminium Ore Piece"), 4)],
                    max_inputs: i32::MAX,
                },
                BufferedRecipe {
                    outputs: vec![Output { item: Filter::Label("Iron Ore"), n_wanted: 64 }],
                    inputs: vec![BufferedInput::new(Filter::Label("Iron Ore Piece"), 4)],
                    max_inputs: i32::MAX,
                },
                BufferedRecipe {
                    outputs: vec![Output { item: Filter::Label("Lead Ore"), n_wanted: 64 }],
                    inputs: vec![BufferedInput::new(Filter::Label("Lead Ore Piece"), 4)],
                    max_inputs: i32::MAX,
                },
                BufferedRecipe {
                    outputs: vec![Output { item: Filter::Label("Silver Ore"), n_wanted: 64 }],
                    inputs: vec![BufferedInput::new(Filter::Label("Silver Ore Piece"), 4)],
                    max_inputs: i32::MAX,
                },
                BufferedRecipe {
                    outputs: vec![Output { item: Filter::Label("Gold Ore"), n_wanted: 64 }],
                    inputs: vec![BufferedInput::new(Filter::Label("Gold Ore Piece"), 4)],
                    max_inputs: i32::MAX,
                },
                BufferedRecipe {
                    outputs: vec![Output { item: Filter::Label("Copper Ore"), n_wanted: 64 }],
                    inputs: vec![BufferedInput::new(Filter::Label("Copper Ore Piece"), 4)],
                    max_inputs: i32::MAX,
                },
                BufferedRecipe {
                    outputs: vec![Output { item: Filter::Label("Tin Ore"), n_wanted: 64 }],
                    inputs: vec![BufferedInput::new(Filter::Label("Tin Ore Piece"), 4)],
                    max_inputs: i32::MAX,
                },
            ],
            max_recipe_inputs: i32::MAX,
            stocks: vec![],
        });
        factory.add_process(BufferedConfig {
            name: "manufactory",
            accesses: vec![
                InvAccess { client: "1a", addr: "b4c", inv_side: WEST, bus_side: EAST },
                InvAccess { client: "1b", addr: "b4c", inv_side: WEST, bus_side: EAST },
                InvAccess { client: "1c", addr: "b4c", inv_side: WEST, bus_side: EAST },
            ],
            slot_filter: Some(Box::new(|slot| slot < 12)),
            to_extract: None,
            recipes: vec![
                BufferedRecipe {
                    outputs: vec![Output { item: Filter::Label("Pulverized Charcoal"), n_wanted: 64 }],
                    inputs: vec![BufferedInput::new(Filter::Label("Charcoal"), 1)],
                    max_inputs: i32::MAX,
                },
                BufferedRecipe {
                    outputs: vec![Output { item: Filter::Label("Oak Wood Planks"), n_wanted: 64 }],
                    inputs: vec![BufferedInput::new(Filter::Label("Oak Wood"), 1)],
                    max_inputs: i32::MAX,
                },
                BufferedRecipe {
                    outputs: vec![Output { item: Filter::Label("Niter"), n_wanted: 64 }],
                    inputs: vec![BufferedInput::new(Filter::Label("Sandstone"), 1)],
                    max_inputs: i32::MAX,
                },
                BufferedRecipe {
                    outputs: vec![Output { item: Filter::Label("Pulverized Aluminum"), n_wanted: 64 }],
                    inputs: vec![BufferedInput::new(Filter::Label("Aluminum Ore"), 1)],
                    max_inputs: i32::MAX,
                },
                BufferedRecipe {
                    outputs: vec![Output { item: Filter::Label("Pulverized Iron"), n_wanted: 64 }],
                    inputs: vec![BufferedInput::new(Filter::Label("Iron Ore"), 1)],
                    max_inputs: i32::MAX,
                },
                BufferedRecipe {
                    outputs: vec![Output { item: Filter::Label("Pulverized Lead"), n_wanted: 64 }],
                    inputs: vec![BufferedInput::new(Filter::Label("Lead Ore"), 1)],
                    max_inputs: i32::MAX,
                },
                BufferedRecipe {
                    outputs: vec![Output { item: Filter::Label("Pulverized Silver"), n_wanted: 64 }],
                    inputs: vec![BufferedInput::new(Filter::Label("Silver Ore"), 1)],
                    max_inputs: i32::MAX,
                },
                BufferedRecipe {
                    outputs: vec![Output { item: Filter::Label("Pulverized Gold"), n_wanted: 64 }],
                    inputs: vec![BufferedInput::new(Filter::Label("Gold Ore"), 1)],
                    max_inputs: i32::MAX,
                },
                BufferedRecipe {
                    outputs: vec![Output { item: Filter::Label("Pulverized Copper"), n_wanted: 64 }],
                    inputs: vec![BufferedInput::new(Filter::Label("Copper Ore"), 1)],
                    max_inputs: i32::MAX,
                },
                BufferedRecipe {
                    outputs: vec![Output { item: Filter::Label("Pulverized Tin"), n_wanted: 64 }],
                    inputs: vec![BufferedInput::new(Filter::Label("Tin Ore"), 1)],
                    max_inputs: i32::MAX,
                },
            ],
            max_recipe_inputs: 8,
            stocks: vec![],
        });
        factory.add_process(WorkbenchConfig {
            name: "workbench",
            accesses: vec![WorkbenchAccess {
                client: "1b",
                input_addr: "b4c",
                output_addr: "8da",
                input_bus_side: EAST,
                output_bus_side: EAST,
                non_consumable_side: UP,
            }],
            recipes: vec![
                CraftingGridRecipe {
                    outputs: vec![Output { item: Filter::Label("Sandstone"), n_wanted: 64 }],
                    inputs: vec![SlottedInput::new(Filter::Label("Sand"), 4, vec![0, 1, 3, 4])],
                    non_consumables: vec![],
                    max_sets: 16,
                },
                CraftingGridRecipe {
                    outputs: vec![Output { item: Filter::Label("Oak Wood"), n_wanted: 64 }],
                    inputs: vec![SlottedInput::new(Filter::Label("Wood Essence"), 3, vec![0, 1, 2])],
                    non_consumables: vec![],
                    max_sets: 4,
                },
                CraftingGridRecipe {
                    outputs: vec![Output { item: Filter::Label("Rich Phyto-Gro"), n_wanted: 64 }],
                    inputs: vec![
                        SlottedInput::new(Filter::Label("Pulverized Charcoal"), 1, vec![0]),
                        SlottedInput::new(Filter::Label("Niter"), 1, vec![1]),
                        SlottedInput::new(Filter::Label("Rich Slag"), 1, vec![2]),
                    ],
                    non_consumables: vec![],
                    max_sets: 4,
                },
                CraftingGridRecipe {
                    outputs: vec![Output { item: Filter::Label("§aPrudentium Essence"), n_wanted: 64 }],
                    inputs: vec![SlottedInput::new(Filter::Label("§eInferium Essence"), 4, vec![1, 3, 5, 7])],
                    non_consumables: vec![NonConsumable { storage_slot: 0, crafting_grid_slot: 4 }],
                    max_sets: 16,
                },
                CraftingGridRecipe {
                    outputs: vec![Output { item: Filter::Label("§6Intermedium Essence"), n_wanted: 64 }],
                    inputs: vec![SlottedInput::new(Filter::Label("§aPrudentium Essence"), 4, vec![1, 3, 5, 7])],
                    non_consumables: vec![NonConsumable { storage_slot: 0, crafting_grid_slot: 4 }],
                    max_sets: 16,
                },
                CraftingGridRecipe {
                    outputs: vec![Output { item: Filter::Label("§bSuperium Essence"), n_wanted: 64 }],
                    inputs: vec![SlottedInput::new(Filter::Label("§6Intermedium Essence"), 4, vec![1, 3, 5, 7])],
                    non_consumables: vec![NonConsumable { storage_slot: 0, crafting_grid_slot: 4 }],
                    max_sets: 16,
                },
                CraftingGridRecipe {
                    outputs: vec![Output { item: Filter::Label("Compass"), n_wanted: 64 }],
                    inputs: vec![
                        SlottedInput::new(Filter::Label("Iron Ingot"), 4, vec![1, 3, 5, 7]),
                        SlottedInput::new(Filter::Label("Redstone"), 1, vec![4]),
                    ],
                    non_consumables: vec![],
                    max_sets: 16,
                },
            ],
        });
        factory.add_process(BufferedConfig {
            name: "phyto",
            accesses: vec![
                InvAccess { client: "1a", addr: "b4c", inv_side: SOUTH, bus_side: EAST },
                InvAccess { client: "1b", addr: "b4c", inv_side: SOUTH, bus_side: EAST },
                InvAccess { client: "1c", addr: "b4c", inv_side: SOUTH, bus_side: EAST },
            ],
            slot_filter: None,
            to_extract: None,
            recipes: vec![
                BufferedRecipe {
                    outputs: vec![Output { item: Filter::Label("Potato"), n_wanted: 64 }],
                    inputs: vec![BufferedInput::new(Filter::Label("Potato"), 1).allow_backup()],
                    max_inputs: i32::MAX,
                },
                BufferedRecipe {
                    outputs: vec![Output { item: Filter::Label("Redstone Essence"), n_wanted: 64 }],
                    inputs: vec![BufferedInput::new(Filter::Label("Redstone Seeds"), 1)],
                    max_inputs: i32::MAX,
                },
                BufferedRecipe {
                    outputs: vec![Output { item: Filter::Label("Wood Essence"), n_wanted: 64 }],
                    inputs: vec![BufferedInput::new(Filter::Label("Wood Seeds"), 1)],
                    max_inputs: i32::MAX,
                },
                BufferedRecipe {
                    outputs: vec![Output { item: Filter::Label("§eInferium Essence"), n_wanted: 64 }],
                    inputs: vec![BufferedInput::new(Filter::Name("mysticalagriculture:tier3_inferium_seeds"), 1)],
                    max_inputs: i32::MAX,
                },
            ],
            max_recipe_inputs: 17,
            stocks: vec![BufferedInput::new(Filter::Label("Fluxed Phyto-Gro"), 64)],
        });
        factory.add_process(ScatteringConfig {
            name: "crusher",
            accesses: vec![
                InvAccess { client: "1a", addr: "a80", inv_side: DOWN, bus_side: SOUTH },
                InvAccess { client: "1b", addr: "a80", inv_side: DOWN, bus_side: SOUTH },
                InvAccess { client: "1c", addr: "a80", inv_side: DOWN, bus_side: SOUTH },
            ],
            input_slots: vec![0, 1, 2, 3, 4, 5, 6],
            to_extract: None,
            recipes: vec![
                ScatteringRecipe::new(
                    vec![Output { item: Filter::Label("Bio Fuel"), n_wanted: 64 }],
                    ScatteringInput::new(Filter::Label("Potato")),
                ),
                ScatteringRecipe::new(
                    vec![Output { item: Filter::Label("Sand"), n_wanted: 64 }],
                    ScatteringInput::new(Filter::Label("Gravel")),
                ),
                ScatteringRecipe::new(
                    vec![Output { item: Filter::Label("Gravel"), n_wanted: 64 }],
                    ScatteringInput::new(Filter::Label("Cobblestone")),
                ),
            ],
            max_per_slot: 4,
        });
        factory.add_process(SlottedConfig {
            name: "charger",
            accesses: vec![
                InvAccess { client: "1a", addr: "a80", inv_side: UP, bus_side: SOUTH },
                InvAccess { client: "1b", addr: "a80", inv_side: UP, bus_side: SOUTH },
                InvAccess { client: "1c", addr: "a80", inv_side: UP, bus_side: SOUTH },
            ],
            input_slots: vec![0],
            to_extract: None,
            recipes: vec![SlottedRecipe {
                outputs: vec![Output { item: Filter::Label("Fluxed Phyto-Gro"), n_wanted: 64 }],
                inputs: vec![SlottedInput::new(Filter::Label("Rich Phyto-Gro"), 1, vec![0])],
                max_per_slot: 8,
            }],
        });
        factory.add_process(SlottedConfig {
            name: "ironSieve",
            accesses: vec![
                InvAccess { client: "1a", addr: "bb0", inv_side: UP, bus_side: NORTH },
                InvAccess { client: "1b", addr: "bb0", inv_side: UP, bus_side: NORTH },
                InvAccess { client: "1c", addr: "bb0", inv_side: UP, bus_side: NORTH },
            ],
            input_slots: vec![0],
            to_extract: None,
            recipes: vec![SlottedRecipe {
                outputs: vec![
                    Output { item: Filter::Label("Aluminium Ore Piece"), n_wanted: 64 },
                    Output { item: Filter::Label("Iron Ore Piece"), n_wanted: 64 },
                    Output { item: Filter::Label("Lead Ore Piece"), n_wanted: 64 },
                    Output { item: Filter::Label("Silver Ore Piece"), n_wanted: 64 },
                    Output { item: Filter::Label("Gold Ore Piece"), n_wanted: 64 },
                    Output { item: Filter::Label("Copper Ore Piece"), n_wanted: 64 },
                    Output { item: Filter::Label("Tin Ore Piece"), n_wanted: 64 },
                ],
                inputs: vec![SlottedInput::new(Filter::Label("Compressed Sand"), 1, vec![0])],
                max_per_slot: 8,
            }],
        });
        factory.add_process(SlottedConfig {
            name: "inductionSmelter",
            accesses: vec![
                InvAccess { client: "1a", addr: "bb0", inv_side: DOWN, bus_side: NORTH },
                InvAccess { client: "1b", addr: "bb0", inv_side: DOWN, bus_side: NORTH },
                InvAccess { client: "1c", addr: "bb0", inv_side: DOWN, bus_side: NORTH },
            ],
            input_slots: vec![0, 1],
            to_extract: None,
            recipes: vec![
                SlottedRecipe {
                    outputs: vec![Output { item: Filter::Label("Rich Slag"), n_wanted: 64 }],
                    inputs: vec![
                        SlottedInput::new(Filter::Label("Sand"), 1, vec![0]),
                        SlottedInput::new(Filter::Label("Compass"), 1, vec![1]),
                    ],
                    max_per_slot: 5,
                },
                SlottedRecipe {
                    outputs: vec![Output { item: Filter::Label("Bronze Ingot"), n_wanted: 64 }],
                    inputs: vec![
                        SlottedInput::new(Filter::Label("Copper Ingot"), 3, vec![0]),
                        SlottedInput::new(Filter::Label("Tin Ingot"), 1, vec![1]),
                    ],
                    max_per_slot: 15,
                },
                SlottedRecipe {
                    outputs: vec![Output { item: Filter::Label("Steel Ingot"), n_wanted: 64 }],
                    inputs: vec![
                        SlottedInput::new(Filter::Label("Iron Ingot"), 1, vec![0]),
                        SlottedInput::new(Filter::Label("Pulverized Charcoal"), 4, vec![1]),
                    ],
                    max_per_slot: 20,
                },
            ],
        });
    })
}
