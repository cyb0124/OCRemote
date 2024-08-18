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
        bus_accesses: vec![
            SidedAccess { client: s("main"), addr: s("7c4"), side: SOUTH },
            SidedAccess { client: s("crafter"), addr: s("inventory_controller"), side: FRONT },
            SidedAccess { client: s("c1"), addr: s("9f3"), side: SOUTH },
            SidedAccess { client: s("c2"), addr: s("72e"), side: WEST },
        ],
        fluid_bus_accesses: vec![
            FluidAccess { client: s("main"), tanks: vec![EachTank { addr: s("e47"), side: SOUTH }]},
            FluidAccess { client: s("c1"), tanks: vec![EachTank { addr: s("cd4"), side: SOUTH }]},
            FluidAccess { client: s("c2"), tanks: vec![EachTank { addr: s("92b"), side: NORTH }]},
        ],
        fluid_bus_capacity: 64_000,
        backups: vec![(label("Glow Flower"), 16), (label("Oxygen Cell"), 30)],
        fluid_backups: vec![(s("oxygen"), 30_000)],
    }
    .build(|factory| {
        factory.add_process(ManualUiConfig { accesses: vec![] });
        factory.add_process(LowAlert::new(label("Salt"), 16));
        factory.add_process(LowAlert::new(label("Iron Dust"), 16));
        factory.add_process(LowAlert::new(label("Empty Cell"), 16));
        factory.add_process(LowAlert::new(label("Phenol Cell"), 16));
        factory.add_process(LowAlert::new(label("Sulfur Dust"), 16));
        factory.add_process(LowAlert::new(label("Carbon Dust"), 16));
        factory.add_process(LowAlert::new(label("Silver Dust"), 16));
        factory.add_process(LowAlert::new(label("Silicon Dioxide Dust"), 16));
        factory.add_process(LowAlert::new(label("Crushed Ilmenite Ore"), 16));
        factory.add_process(LowAlert::new(label("Copper Ingot"), 16));
        factory.add_storage(ChestConfig {
            accesses: vec![InvAccess { client: s("main"), addr: s("09c"), bus_side: NORTH, inv_side: SOUTH }],
        });
        factory.add_storage(DrawerConfig { accesses: vec![InvAccess{ client: s("main"), addr: s("c47"), bus_side: NORTH, inv_side: SOUTH }],
         filters: vec![label("Sesame Seeds")] });
        let hydrogen_output = || FluidOutput::new(s("hydrogen"), 16_000).or(Output::new(label("Hydrogen Cell"), 65));
        let hcl_output = || FluidOutput::new(s("hydrochloricacid_gt5u"), 33_000).or(Output::new(label("Hydrochloric Acid Cell"), 16));
        let plastic_cb = || custom("plasticCB", |x| x.label == "Plastic Circuit Board" && x.damage == 32007);
        let plastic_pcb = || custom("plasticPCB", |x| x.label == "Plastic Circuit Board" && x.damage == 32106);
        for (fluid, client, bus_of_tank) in [
            ("water", "main", EachBusOfTank { addr: s("608"), bus_side: SOUTH, tank_side: UP }),
            ("hydrogen", "main", EachBusOfTank { addr: s("608"), bus_side: SOUTH, tank_side: NORTH }),
            ("solution.greenvitriol", "main", EachBusOfTank { addr: s("608"), bus_side: SOUTH, tank_side: EAST }),
            ("bioethanol", "main", EachBusOfTank { addr: s("e47"), bus_side: SOUTH, tank_side: EAST }),
            ("molten.polyvinylchloride", "main", EachBusOfTank { addr: s("e47"), bus_side: SOUTH, tank_side: NORTH }),
            ("molten.epoxid", "main", EachBusOfTank { addr: s("269"), bus_side: EAST, tank_side: SOUTH }),
            ("saltwater", "main", EachBusOfTank { addr: s("269"), bus_side: EAST, tank_side: WEST }),
            ("ammonia", "main", EachBusOfTank { addr: s("269"), bus_side: EAST, tank_side: UP }),
            ("biomass", "c1", EachBusOfTank { addr: s("cd4"), bus_side: SOUTH, tank_side: WEST }),
            ("ic2distilledwater", "c1", EachBusOfTank { addr: s("cd4"), bus_side: SOUTH, tank_side: UP }),
            ("molten.plastic", "c1", EachBusOfTank { addr: s("cd4"), bus_side: SOUTH, tank_side: NORTH }),
            ("oxygen", "c1", EachBusOfTank { addr: s("cd4"), bus_side: SOUTH, tank_side: EAST }),
            ("seedoil", "c1", EachBusOfTank { addr: s("6e5"), bus_side: SOUTH, tank_side: WEST }),
            ("sulfuricacid", "c1", EachBusOfTank { addr: s("6e5"), bus_side: SOUTH, tank_side: NORTH }),
            ("vinylchloride", "c1", EachBusOfTank { addr: s("328"), bus_side: SOUTH, tank_side: WEST }),
            ("dilutedsulfuricacid", "c1", EachBusOfTank { addr: s("328"), bus_side: SOUTH, tank_side: UP }),
            ("nitrogen", "c1", EachBusOfTank { addr: s("328"), bus_side: SOUTH, tank_side: EAST }),
            ("nitricacid", "c1", EachBusOfTank { addr: s("328"), bus_side: SOUTH, tank_side: NORTH }),
            ("molten.silicone", "c1", EachBusOfTank { addr: s("500"), bus_side: NORTH, tank_side: SOUTH }),
            ("carbondioxide", "c1", EachBusOfTank { addr: s("500"), bus_side: NORTH, tank_side: WEST }),
            ("ethenone", "c1", EachBusOfTank { addr: s("500"), bus_side: NORTH, tank_side: UP }),
            ("tetranitromethane", "c1", EachBusOfTank { addr: s("e81"), bus_side: SOUTH, tank_side: UP }),
            ("biodiesel", "c1", EachBusOfTank { addr: s("e81"), bus_side: SOUTH, tank_side: WEST }),
            ("aceticacid", "c1", EachBusOfTank { addr: s("f9d"), bus_side: EAST, tank_side: UP }),
            ("dilutedhydrochloricacid_gt5u", "c1", EachBusOfTank { addr: s("449"), bus_side: UP, tank_side: DOWN }),
            ("methane", "c1", EachBusOfTank { addr: s("449"), bus_side: UP, tank_side: EAST }),
            ("acetone", "c1", EachBusOfTank { addr: s("83e"), bus_side: NORTH, tank_side: SOUTH }),
            ("chlorine", "c1", EachBusOfTank { addr: s("83e"), bus_side: NORTH, tank_side: EAST }),
            ("ironiiichloride", "c1", EachBusOfTank { addr: s("83e"), bus_side: NORTH, tank_side: UP }),
            ("hydrochloricacid_gt5u", "c1", EachBusOfTank { addr: s("83e"), bus_side: NORTH, tank_side: WEST }),
            ("lubricant", "c2", EachBusOfTank { addr: s("9a6"), bus_side: SOUTH, tank_side: UP }),
            ("nitrofuel", "c2", EachBusOfTank { addr: s("9a6"), bus_side: SOUTH, tank_side: NORTH }),
            ("ethylene", "c2", EachBusOfTank { addr: s("92b"), bus_side: NORTH, tank_side: SOUTH }),
            ("liquid_epichlorhydrin", "c2", EachBusOfTank { addr: s("92b"), bus_side: NORTH, tank_side: UP }),
            ("glycerol", "c2", EachBusOfTank { addr: s("f6a"), bus_side: SOUTH, tank_side: UP }),
            ("air", "c2", EachBusOfTank { addr: s("0c0"), bus_side: EAST, tank_side: NORTH }),
            ("phosphoricacid_gt5u", "c2", EachBusOfTank { addr: s("0c0"), bus_side: EAST, tank_side: UP }),
        ] {
            factory.add_fluid_storage(FluidStorageConfig {
                accesses: vec![TankAccess { client: s(client), buses: vec![bus_of_tank] }],
                fluid: s(fluid),
            })
        }
        for access in [InvAccess { client: s("main"), addr: s("c47"), bus_side: NORTH, inv_side: UP }] {
            factory.add_process(SlottedConfig {
                name: s("output"),
                accesses: vec![access],
                input_slots: vec![],
                to_extract: extract_all(),
                strict_priority: false,
                recipes: vec![],
            })
        }
        for (client, bus_of_tank) in [
            ("c1", EachBusOfTank { addr: s("449"), bus_side: UP, tank_side: NORTH }),     // DT
            ("c1", EachBusOfTank { addr: s("f9d"), bus_side: EAST, tank_side: NORTH }),   // DT
            ("c2", EachBusOfTank { addr: s("9a6"), bus_side: SOUTH, tank_side: WEST }),   // LCR-1
            ("c2", EachBusOfTank { addr: s("92b"), bus_side: NORTH, tank_side: EAST }),   // LCR-1
            ("c2", EachBusOfTank { addr: s("f6a"), bus_side: SOUTH, tank_side: NORTH }),  // LCR-24
            ("c2", EachBusOfTank { addr: s("92b"), bus_side: NORTH, tank_side: WEST }),   // LCR-24
        ] {
            factory.add_process(FluidSlottedConfig {
                name: s("fluidOutput"),
                input_slots: vec![],
                input_tanks: vec![vec![0]],
                accesses: vec![InvTankAccess { client: s(client), invs: vec![], tanks: vec![vec![bus_of_tank]]}],
                to_extract: None,
                fluid_extract: fluid_extract_all(),
                strict_priority: false,
                recipes: vec![],
            })
        }
        for (access, item, qty) in
            [(InvAccess { client: s("main"), addr: s("c47"), bus_side: NORTH, inv_side: WEST }, "Glow Flower Seed", 64)]
        {
            factory.add_process(BufferedConfig {
                name: s("stock"),
                accesses: vec![access],
                slot_filter: None,
                to_extract: None,
                recipes: vec![],
                max_recipe_inputs: 0,
                stocks: vec![BufferedInput::new(label(item), qty)],
            })
        }
        for (fluid, per_set, max_sets, client, bus_of_tank) in [
            ("ammonia", 1_000, 256, "c1", EachBusOfTank { addr: s("6e5"), bus_side: SOUTH, tank_side: EAST }),
            ("oxygen", 1_000, 16, "c2", EachBusOfTank { addr: s("9a6"), bus_side: SOUTH, tank_side: EAST }),
            ("nitrofuel", 1_000, 16, "main", EachBusOfTank { addr: s("e47"), bus_side: SOUTH, tank_side: UP }),
        ] {
            factory.add_process(FluidSlottedConfig {
                name: s("fluidStock"),
                input_slots: vec![],
                input_tanks: vec![vec![0]],
                accesses: vec![InvTankAccess { client: s(client), invs: vec![], tanks: vec![vec![bus_of_tank]] }],
                to_extract: None,
                fluid_extract: None,
                strict_priority: false,
                recipes: vec![FluidSlottedRecipe {
                    outputs: ignore_outputs(1.),
                    inputs: vec![],
                    fluids: vec![FluidSlottedInput::new(s(fluid), vec![(0, per_set)])],
                    max_sets,
                }],
            })
        }
        factory.add_process(CraftingRobotConfig {
            name: s("craftingGrid"),
            accesses: vec![CraftingRobotAccess { client: s("crafter"), bus_side: FRONT }],
            recipes: vec![
                CraftingGridRecipe {
                    outputs: ignore_outputs(1.),
                    inputs: vec![CraftingGridInput::new(label("Small Pile of Wood Pulp"), vec![0, 1, 3, 4])],
                    max_sets: 64,
                    non_consumables: vec![],
                },
                CraftingGridRecipe {
                    outputs: Output::new(label("Glow Flower Seed"), 16).and(Output::new(label("Glow Flower"), 1024).not()),
                    inputs: vec![CraftingGridInput::new(label("Glow Flower"), vec![0]).allow_backup()],
                    max_sets: 64,
                    non_consumables: vec![],
                },
                CraftingGridRecipe {
                    outputs: Output::new(label("Tiny Pile of Sodium Hydroxide Dust"), 16),
                    inputs: vec![CraftingGridInput::new(label("Sodium Hydroxide Dust"), vec![0])],
                    max_sets: 7,
                    non_consumables: vec![],
                },
            ],
        });
        factory.add_process(FluidSlottedConfig {
            name: s("DT"),
            input_slots: vec![],
            input_tanks: vec![vec![0]],
            accesses: vec![InvTankAccess {
                client: s("c1"),
                invs: vec![],
                tanks: vec![vec![EachBusOfTank { addr: s("e81"), bus_side: SOUTH, tank_side: EAST }]],
            }],
            to_extract: None,
            fluid_extract: None,
            strict_priority: false,
            recipes: vec![
                FluidSlottedRecipe {
                    outputs: ignore_outputs(64.),
                    inputs: vec![],
                    fluids: vec![FluidSlottedInput::new(s("dilutedsulfuricacid"), vec![(0, 3_000)]).extra_backup(1)],
                    max_sets: 16,
                },
                FluidSlottedRecipe {
                    outputs: ignore_outputs(64.),
                    inputs: vec![],
                    fluids: vec![FluidSlottedInput::new(s("dilutedhydrochloricacid_gt5u"), vec![(0, 2_000)]).extra_backup(1)],
                    max_sets: 16,
                },
                FluidSlottedRecipe {
                    outputs: FluidOutput::new(s("bioethanol"), 16_000),
                    inputs: vec![],
                    fluids: vec![FluidSlottedInput::new(s("biomass"), vec![(0, 1_000)])],
                    max_sets: 16,
                },
                FluidSlottedRecipe {
                    outputs: FluidOutput::new(s("ic2distilledwater"), 16_000),
                    inputs: vec![],
                    fluids: vec![FluidSlottedInput::new(s("water"), vec![(0, 1_000)])],
                    max_sets: 16,
                },
            ],
        });
        factory.add_process(FluidSlottedConfig {
            name: s("plateSolidifier"),
            input_slots: vec![],
            input_tanks: vec![vec![0]],
            accesses: vec![InvTankAccess {
                client: s("main"),
                invs: vec![],
                tanks: vec![vec![EachBusOfTank { addr: s("e47"), bus_side: SOUTH, tank_side: WEST }]],
            }],
            to_extract: None,
            fluid_extract: None,
            strict_priority: false,
            recipes: vec![
                FluidSlottedRecipe {
                    outputs: Output::new(label("Epoxid Sheet"), 16),
                    inputs: vec![],
                    fluids: vec![FluidSlottedInput::new(s("molten.epoxid"), vec![(0, 144)])],
                    max_sets: 16,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(label("Polyvinyl Chloride Sheet"), 16),
                    inputs: vec![],
                    fluids: vec![FluidSlottedInput::new(s("molten.polyvinylchloride"), vec![(0, 144)])],
                    max_sets: 16,
                },
            ],
        });
        factory.add_process(FluidSlottedConfig {
            name: s("chemicalBath"),
            input_slots: vec![vec![5]],
            input_tanks: vec![vec![0]],
            accesses: vec![InvTankAccess {
                client: s("c2"),
                invs: vec![EachInvAccess { addr: s("655"), bus_side: WEST, inv_side: NORTH }],
                tanks: vec![vec![EachBusOfTank { addr: s("655"), bus_side: EAST, tank_side: NORTH }]],
            }],
            to_extract: None,
            fluid_extract: fluid_extract_slots(|_, i| i == 1),
            strict_priority: false,
            recipes: vec![
                FluidSlottedRecipe {
                    outputs: Output::new(label("Mulch"), 16),
                    inputs: vec![MultiInvSlottedInput::new(label("Bio Chaff"), vec![(0, 5, 1)])],
                    fluids: vec![FluidSlottedInput::new(s("water"), vec![(0, 750)])],
                    max_sets: 8,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(label("Rutile Dust"), 64),
                    inputs: vec![MultiInvSlottedInput::new(label("Crushed Ilmenite Ore"), vec![(0, 5, 1)])],
                    fluids: vec![FluidSlottedInput::new(s("sulfuricacid"), vec![(0, 1_000)])],
                    max_sets: 8,
                },
            ],
        });
        factory.add_process(FluidSlottedConfig {
            name: s("brewery"),
            input_slots: vec![vec![5]],
            input_tanks: vec![vec![0]],
            accesses: vec![InvTankAccess {
                client: s("main"),
                invs: vec![EachInvAccess { addr: s("7c4"), bus_side: SOUTH, inv_side: EAST }],
                tanks: vec![vec![EachBusOfTank { addr: s("7c4"), bus_side: NORTH, tank_side: EAST }]],
            }],
            to_extract: None,
            fluid_extract: fluid_extract_slots(|_, i| i == 1),
            strict_priority: false,
            recipes: vec![FluidSlottedRecipe {
                outputs: FluidOutput::new(s("biomass"), 16_000),
                inputs: vec![MultiInvSlottedInput::new(label("Mulch"), vec![(0, 5, 8)])],
                fluids: vec![FluidSlottedInput::new(s("ic2distilledwater"), vec![(0, 375)])],
                max_sets: 8,
            },FluidSlottedRecipe {
                outputs: FluidOutput::new(s("lubricant"), 32_000),
                inputs: vec![MultiInvSlottedInput::new(label("Redstone"), vec![(0, 5, 1)])],
                fluids: vec![FluidSlottedInput::new(s("seedoil"), vec![(0, 750)])],
                max_sets: 8,
            }],
        });
        factory.add_process(FluidSlottedConfig {
            name: s("fluidToCell"),
            input_slots: vec![vec![0]],
            input_tanks: vec![vec![0]],
            accesses: vec![InvTankAccess {
                client: s("main"),
                invs: vec![EachInvAccess { addr: s("039"), bus_side: SOUTH, inv_side: WEST }],
                tanks: vec![vec![EachBusOfTank { addr: s("039"), bus_side: NORTH, tank_side: WEST }]],
            }],
            to_extract: multi_inv_extract_all(),
            fluid_extract: None,
            strict_priority: false,
            recipes: vec![
                FluidSlottedRecipe {
                    outputs: ignore_outputs(64.),
                    inputs: vec![MultiInvSlottedInput::new(label("Empty Cell"), vec![(0, 0, 1)])],
                    fluids: vec![FluidSlottedInput::new(s("hydrochloricacid_gt5u"), vec![(0, 1_000)]).extra_backup(32_000)],
                    max_sets: 16,
                },
                FluidSlottedRecipe {
                    outputs: ignore_outputs(64.),
                    inputs: vec![MultiInvSlottedInput::new(label("Empty Cell"), vec![(0, 0, 1)])],
                    fluids: vec![FluidSlottedInput::new(s("glycerol"), vec![(0, 1_000)]).extra_backup(1)],
                    max_sets: 16,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(label("Hydrogen Cell"), 16),
                    inputs: vec![MultiInvSlottedInput::new(label("Empty Cell"), vec![(0, 0, 1)])],
                    fluids: vec![FluidSlottedInput::new(s("hydrogen"), vec![(0, 1_000)])],
                    max_sets: 16,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(label("Ethanol Cell"), 16),
                    inputs: vec![MultiInvSlottedInput::new(label("Empty Cell"), vec![(0, 0, 1)])],
                    fluids: vec![FluidSlottedInput::new(s("bioethanol"), vec![(0, 1_000)])],
                    max_sets: 16,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(label("Water Cell"), 16),
                    inputs: vec![MultiInvSlottedInput::new(label("Empty Cell"), vec![(0, 0, 1)])],
                    fluids: vec![FluidSlottedInput::new(s("water"), vec![(0, 1_000)])],
                    max_sets: 16,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(label("Bio Diesel Cell"), 16),
                    inputs: vec![MultiInvSlottedInput::new(label("Empty Cell"), vec![(0, 0, 1)])],
                    fluids: vec![FluidSlottedInput::new(s("biodiesel"), vec![(0, 1_000)])],
                    max_sets: 16,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(label("Phosphoric Acid Cell"), 16),
                    inputs: vec![MultiInvSlottedInput::new(label("Empty Cell"), vec![(0, 0, 1)])],
                    fluids: vec![FluidSlottedInput::new(s("phosphoricacid_gt5u"), vec![(0, 1_000)])],
                    max_sets: 16,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(label("Ethylene Cell"), 16),
                    inputs: vec![MultiInvSlottedInput::new(label("Empty Cell"), vec![(0, 0, 1)])],
                    fluids: vec![FluidSlottedInput::new(s("ethylene"), vec![(0, 1_000)])],
                    max_sets: 16,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(label("Sulfuric Acid Cell"), 16),
                    inputs: vec![MultiInvSlottedInput::new(label("Empty Cell"), vec![(0, 0, 1)])],
                    fluids: vec![FluidSlottedInput::new(s("sulfuricacid"), vec![(0, 1_000)])],
                    max_sets: 16,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(label("Epichlorohydrin Cell"), 16),
                    inputs: vec![MultiInvSlottedInput::new(label("Empty Cell"), vec![(0, 0, 1)])],
                    fluids: vec![FluidSlottedInput::new(s("liquid_epichlorhydrin"), vec![(0, 1_000)])],
                    max_sets: 16,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(label("Methane Cell"), 16),
                    inputs: vec![MultiInvSlottedInput::new(label("Empty Cell"), vec![(0, 0, 1)])],
                    fluids: vec![FluidSlottedInput::new(s("methane"), vec![(0, 1_000)])],
                    max_sets: 16,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(label("Nitric Acid Cell"), 16),
                    inputs: vec![MultiInvSlottedInput::new(label("Empty Cell"), vec![(0, 0, 1)])],
                    fluids: vec![FluidSlottedInput::new(s("nitricacid"), vec![(0, 1_000)])],
                    max_sets: 16,
                },
            ],
        });
        factory.add_process(FluidSlottedConfig {
            name: s("cellToFluid"),
            input_slots: vec![vec![0]],
            input_tanks: vec![vec![0]],
            accesses: vec![InvTankAccess {
                client: s("c1"),
                invs: vec![EachInvAccess { addr: s("9f3"), bus_side: SOUTH, inv_side: EAST }],
                tanks: vec![vec![EachBusOfTank { addr: s("9f3"), bus_side: NORTH, tank_side: EAST }]],
            }],
            to_extract: multi_inv_extract_all(),
            fluid_extract: fluid_extract_all(),
            strict_priority: false,
            recipes: vec![
                FluidSlottedRecipe {
                    outputs: ignore_outputs(1.),
                    inputs: vec![MultiInvSlottedInput::new(label("Hydrogen Cell"), vec![(0, 0, 1)]).extra_backup(64)],
                    fluids: vec![],
                    max_sets: 64,
                },
                FluidSlottedRecipe {
                    outputs: ignore_outputs(1.),
                    inputs: vec![MultiInvSlottedInput::new(label("Diluted Hydrochloric Acid Cell"), vec![(0, 0, 1)])],
                    fluids: vec![],
                    max_sets: 64,
                },
                FluidSlottedRecipe {
                    outputs: ignore_outputs(1.),
                    inputs: vec![MultiInvSlottedInput::new(label("Salt Water Cell"), vec![(0, 0, 1)])],
                    fluids: vec![],
                    max_sets: 64,
                },
                FluidSlottedRecipe {
                    outputs: FluidOutput::new(s("oxygen"), 64_000),
                    inputs: vec![MultiInvSlottedInput::new(label("Oxygen Cell"), vec![(0, 0, 1)]).allow_backup()],
                    fluids: vec![],
                    max_sets: 64,
                },
                FluidSlottedRecipe {
                    outputs: FluidOutput::new(s("air"), 16_000),
                    inputs: vec![MultiInvSlottedInput::new(label("Compressed Air Cell"), vec![(0, 0, 1)])],
                    fluids: vec![],
                    max_sets: 16,
                },
                FluidSlottedRecipe {
                    outputs: FluidOutput::new(s("hydrochloricacid_gt5u"), 16_000),
                    inputs: vec![MultiInvSlottedInput::new(label("Hydrochloric Acid Cell"), vec![(0, 0, 1)])],
                    fluids: vec![],
                    max_sets: 8,
                },
            ],
        });
        factory.add_process(FluidSlottedConfig {
            name: s("LCR-1"),
            input_slots: vec![vec![0, 1], vec![0]],
            input_tanks: vec![vec![0]],
            accesses: vec![InvTankAccess {
                client: s("c2"),
                invs: vec![
                    EachInvAccess { addr: s("72e"), bus_side: WEST, inv_side: EAST },
                    EachInvAccess { addr: s("72e"), bus_side: WEST, inv_side: NORTH },
                ],
                tanks: vec![vec![EachBusOfTank { addr: s("f6a"), bus_side: SOUTH, tank_side: EAST }]],
            }],
            to_extract: multi_inv_extract_all(),
            fluid_extract: None,
            strict_priority: false,
            recipes: vec![
                FluidSlottedRecipe {
                    outputs: FluidOutput::new(s("ammonia"), 16_000),
                    inputs: vec![MultiInvSlottedInput::new(label("Hydrogen Cell"), vec![(1, 0, 3)])],
                    fluids: vec![FluidSlottedInput::new(s("nitrogen"), vec![(0, 1_000)])],
                    max_sets: 8,
                },
                FluidSlottedRecipe {
                    outputs: FluidOutput::new(s("ethylene"), 16_000),
                    inputs: vec![MultiInvSlottedInput::new(label("Ethanol Cell"), vec![(1, 0, 1)])],
                    fluids: vec![FluidSlottedInput::new(s("sulfuricacid"), vec![(0, 1_000)])],
                    max_sets: 8,
                },
                FluidSlottedRecipe {
                    outputs: FluidOutput::new(s("molten.plastic"), 32_000),
                    inputs: vec![MultiInvSlottedInput::new(label("Oxygen Cell"), vec![(1, 0, 1)])],
                    fluids: vec![FluidSlottedInput::new(s("ethylene"), vec![(0, 144)])],
                    max_sets: 16,
                },
                FluidSlottedRecipe {
                    outputs: FluidOutput::new(s("molten.polyvinylchloride"), 32_000),
                    inputs: vec![MultiInvSlottedInput::new(label("Oxygen Cell"), vec![(1, 0, 1)])],
                    fluids: vec![FluidSlottedInput::new(s("vinylchloride"), vec![(0, 144)])],
                    max_sets: 16,
                },
                FluidSlottedRecipe {
                    outputs: hcl_output(),
                    inputs: vec![MultiInvSlottedInput::new(label("Hydrogen Cell"), vec![(1, 0, 1)])],
                    fluids: vec![FluidSlottedInput::new(s("chlorine"), vec![(0, 1_000)])],
                    max_sets: 8,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(label("Sodium Hydroxide Dust"), 16),
                    inputs: vec![MultiInvSlottedInput::new(label("Sodium Dust"), vec![(0, 0, 1)])],
                    fluids: vec![FluidSlottedInput::new(s("water"), vec![(0, 1_000)])],
                    max_sets: 8,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(label("Epoxy Circuit Board"), 16),
                    inputs: vec![
                        MultiInvSlottedInput::new(label("Epoxid Sheet"), vec![(0, 0, 1)]),
                        MultiInvSlottedInput::new(label("Gold Foil"), vec![(0, 1, 8)]),
                    ],
                    fluids: vec![FluidSlottedInput::new(s("sulfuricacid"), vec![(0, 500)])],
                    max_sets: 8,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(label("Advanced Circuit Board"), 16),
                    inputs: vec![
                        MultiInvSlottedInput::new(label("Epoxy Circuit Board"), vec![(0, 0, 1)]),
                        MultiInvSlottedInput::new(label("Electrum Foil"), vec![(0, 1, 8)]),
                    ],
                    fluids: vec![FluidSlottedInput::new(s("ironiiichloride"), vec![(0, 500)])],
                    max_sets: 8,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(plastic_cb(), 16),
                    inputs: vec![
                        MultiInvSlottedInput::new(label("Polyvinyl Chloride Sheet"), vec![(0, 0, 1)]),
                        MultiInvSlottedInput::new(label("Copper Foil"), vec![(0, 1, 4)]),
                    ],
                    fluids: vec![FluidSlottedInput::new(s("sulfuricacid"), vec![(0, 500)])],
                    max_sets: 8,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(plastic_pcb(), 16),
                    inputs: vec![
                        MultiInvSlottedInput::new(plastic_cb(), vec![(0, 0, 1)]),
                        MultiInvSlottedInput::new(label("Copper Foil"), vec![(0, 1, 6)]),
                    ],
                    fluids: vec![FluidSlottedInput::new(s("ironiiichloride"), vec![(0, 250)])],
                    max_sets: 8,
                },
                FluidSlottedRecipe {
                    outputs: FluidOutput::new(s("molten.silicone"), 32_000),
                    inputs: vec![
                        MultiInvSlottedInput::new(label("Polydimethylsiloxane Pulp"), vec![(0, 0, 9)]),
                        MultiInvSlottedInput::new(label("Sulfur Dust"), vec![(0, 1, 1)]),
                    ],
                    fluids: vec![],
                    max_sets: 8,
                },
                FluidSlottedRecipe {
                    outputs: FluidOutput::new(s("phosphoricacid_gt5u"), 16_000),
                    inputs: vec![MultiInvSlottedInput::new(label("Phosphorous Pentoxide Dust"), vec![(0, 0, 14)])],
                    fluids: vec![FluidSlottedInput::new(s("water"), vec![(0, 6_000)])],
                    max_sets: 8,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(label("Phosphorous Pentoxide Dust"), 16),
                    inputs: vec![MultiInvSlottedInput::new(label("Phosphorus Dust"), vec![(0, 0, 4)])],
                    fluids: vec![FluidSlottedInput::new(s("oxygen"), vec![(0, 10_000)]).allow_backup()],
                    max_sets: 6,
                },
                FluidSlottedRecipe {
                    outputs: FluidOutput::new(s("methane"), 16_000),
                    inputs: vec![MultiInvSlottedInput::new(label("Carbon Dust"), vec![(0, 0, 1)])],
                    fluids: vec![FluidSlottedInput::new(s("hydrogen"), vec![(0, 4_000)])],
                    max_sets: 8,
                },
                FluidSlottedRecipe {
                    outputs: FluidOutput::new(s("ethenone"), 16_000),
                    inputs: vec![MultiInvSlottedInput::new(label("Sulfuric Acid Cell"), vec![(1, 0, 1)])],
                    fluids: vec![FluidSlottedInput::new(s("aceticacid"), vec![(0, 1_000)])],
                    max_sets: 8,
                },
                FluidSlottedRecipe {
                    outputs: FluidOutput::new(s("tetranitromethane"), 16_000),
                    inputs: vec![MultiInvSlottedInput::new(label("Nitric Acid Cell"), vec![(1, 0, 8)])],
                    fluids: vec![FluidSlottedInput::new(s("ethenone"), vec![(0, 1_000)])],
                    max_sets: 4,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(label("Raw Silicon Dust"), 16),
                    inputs: vec![
                        MultiInvSlottedInput::new(label("Silicon Dioxide Dust"), vec![(0, 0, 3)]),
                        MultiInvSlottedInput::new(label("Magnesium Dust"), vec![(0, 1, 2)]),
                    ],
                    fluids: vec![],
                    max_sets: 8,
                },
            ]
        });
        factory.add_process(FluidSlottedConfig {
            name: s("LCR-24"),
            input_slots: vec![vec![0], vec![0], vec![0], vec![0]],
            input_tanks: vec![vec![0]],
            accesses: vec![InvTankAccess {
                client: s("c2"),
                invs: vec![
                    EachInvAccess { addr: s("5f7"), bus_side: EAST, inv_side: SOUTH },
                    EachInvAccess { addr: s("5f7"), bus_side: EAST, inv_side: NORTH },
                    EachInvAccess { addr: s("5f7"), bus_side: EAST, inv_side: WEST },
                    EachInvAccess { addr: s("5f7"), bus_side: EAST, inv_side: UP },
                ],
                tanks: vec![vec![EachBusOfTank { addr: s("f6a"), bus_side: SOUTH, tank_side: WEST }]],
            }],
            to_extract: multi_inv_extract_all(),
            fluid_extract: None,
            strict_priority: false,
            recipes: vec![
                FluidSlottedRecipe {
                    outputs: FluidOutput::new(s("sulfuricacid"), 16_000),
                    inputs: vec![
                        MultiInvSlottedInput::new(label("Sulfur Dust"), vec![(0, 0, 1)]),
                        MultiInvSlottedInput::new(label("Water Cell"), vec![(1, 0, 1)]),
                    ],
                    fluids: vec![FluidSlottedInput::new(s("oxygen"), vec![(0, 3_000)])],
                    max_sets: 8,
                },
                FluidSlottedRecipe {
                    outputs: FluidOutput::new(s("nitricacid"), 16_000),
                    inputs: vec![MultiInvSlottedInput::new(label("Oxygen Cell"), vec![(1, 0, 4)])],
                    fluids: vec![FluidSlottedInput::new(s("ammonia"), vec![(0, 1_000)])],
                    max_sets: 8,
                },
                FluidSlottedRecipe {
                    outputs: FluidOutput::new(s("vinylchloride"), 16_000),
                    inputs: vec![
                        MultiInvSlottedInput::new(label("Ethylene Cell"), vec![(1, 0, 2)]),
                        MultiInvSlottedInput::new(label("Oxygen Cell"), vec![(2, 0, 1)]),
                    ],
                    fluids: vec![FluidSlottedInput::new(s("chlorine"), vec![(0, 2_000)])],
                    max_sets: 8,
                },
                FluidSlottedRecipe {
                    outputs: FluidOutput::new(s("biodiesel"), 16_000),
                    inputs: vec![
                        MultiInvSlottedInput::new(label("Tiny Pile of Sodium Hydroxide Dust"), vec![(0, 0, 1)]),
                        MultiInvSlottedInput::new(label("Ethanol Cell"), vec![(1, 0, 1)]),
                    ],
                    fluids: vec![FluidSlottedInput::new(s("seedoil"), vec![(0, 6_000)])],
                    max_sets: 8,
                },
                FluidSlottedRecipe {
                    outputs: FluidOutput::new(s("aceticacid"), 16),
                    inputs: vec![
                        MultiInvSlottedInput::new(label("Carbon Dust"), vec![(0, 0, 2)]),
                        MultiInvSlottedInput::new(label("Hydrogen Cell"), vec![(1, 0, 4)]),
                    ],
                    fluids: vec![FluidSlottedInput::new(s("oxygen"), vec![(0, 2_000)])],
                    max_sets: 8,
                },
                FluidSlottedRecipe {
                    outputs: FluidOutput::new(s("ironiiichloride"), 16_000),
                    inputs: vec![MultiInvSlottedInput::new(label("Iron Dust"), vec![(0, 0, 1)])],
                    fluids: vec![FluidSlottedInput::new(s("hydrochloricacid_gt5u"), vec![(0, 3_000)])],
                    max_sets: 8,
                },
                FluidSlottedRecipe {
                    outputs: FluidOutput::new(s("liquid_epichlorhydrin"), 16_000),
                    inputs: vec![MultiInvSlottedInput::new(label("Glycerol Cell"), vec![(1, 0, 1)])],
                    fluids: vec![FluidSlottedInput::new(s("hydrochloricacid_gt5u"), vec![(0, 1_000)])],
                    max_sets: 8,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(label("Polydimethylsiloxane Pulp"), 16),
                    inputs: vec![
                        MultiInvSlottedInput::new(label("Raw Silicon Dust"), vec![(0, 0, 1)]),
                        MultiInvSlottedInput::new(label("Water Cell"), vec![(1, 0, 1)]),
                        MultiInvSlottedInput::new(label("Methane Cell"), vec![(2, 0, 2)]),
                    ],
                    fluids: vec![FluidSlottedInput::new(s("chlorine"), vec![(0, 4_000)])],
                    max_sets: 8,
                },
                FluidSlottedRecipe {
                    outputs: FluidOutput::new(s("molten.epoxid"), 16_000),
                    inputs: vec![
                        MultiInvSlottedInput::new(label("Sodium Hydroxide Dust"), vec![(0, 0, 6)]),
                        MultiInvSlottedInput::new(label("Epichlorohydrin Cell"), vec![(1, 0, 2)]),
                        MultiInvSlottedInput::new(label("Phenol Cell"), vec![(2, 0, 2)]),
                        MultiInvSlottedInput::new(label("Hydrochloric Acid Cell"), vec![(3, 0, 1)]),
                    ],
                    fluids: vec![FluidSlottedInput::new(s("acetone"), vec![(0, 1_000)])],
                    max_sets: 8,
                },
                FluidSlottedRecipe {
                    outputs: FluidOutput::new(s("acetone"), 16_000),
                    inputs: vec![],
                    fluids: vec![FluidSlottedInput::new(s("aceticacid"), vec![(0, 2_000)])],
                    max_sets: 8,
                },
            ]
        });
        factory.add_process(FluidSlottedConfig {
            name: s("mixer-1"),
            input_slots: vec![vec![5, 6]],
            input_tanks: vec![vec![0]],
            accesses: vec![InvTankAccess {
                client: s("c1"),
                invs: vec![EachInvAccess { addr: s("926"), bus_side: SOUTH, inv_side: EAST }],
                tanks: vec![vec![EachBusOfTank { addr: s("926"), bus_side: NORTH, tank_side: EAST }]],
            }],
            to_extract: None,
            fluid_extract: fluid_extract_all(),
            strict_priority: false,
            recipes: vec![
                FluidSlottedRecipe {
                    outputs: FluidOutput::new(s("nitrofuel"), 16_000),
                    inputs: vec![MultiInvSlottedInput::new(label("Bio Diesel Cell"), vec![(0, 5, 1)])],
                    fluids: vec![FluidSlottedInput::new(s("tetranitromethane"), vec![(0, 40)])],
                    max_sets: 8,
                },
            ],
        });
        factory.add_process(FluidSlottedConfig {
            name: s("electrolyzer-1"),
            input_slots: vec![vec![5, 6]],
            input_tanks: vec![vec![0]],
            accesses: vec![InvTankAccess {
                client: s("c2"),
                invs: vec![EachInvAccess { addr: s("655"), bus_side: WEST, inv_side: UP }],
                tanks: vec![vec![EachBusOfTank { addr: s("655"), bus_side: EAST, tank_side: UP }]],
            }],
            to_extract: None,
            fluid_extract: fluid_extract_slots(|_, i| i == 1),
            strict_priority: false,
            recipes: vec![
                FluidSlottedRecipe {
                    outputs: ignore_outputs(64.),
                    inputs: vec![MultiInvSlottedInput::new(label("Empty Cell"), vec![(0, 5, 1)])],
                    fluids: vec![FluidSlottedInput::new(s("saltwater"), vec![(0, 1_000)]).extra_backup(1)],
                    max_sets: 8,
                },
                FluidSlottedRecipe {
                    outputs: ignore_outputs(64.),
                    inputs: vec![MultiInvSlottedInput::new(label("Empty Cell"), vec![(0, 5, 1)])],
                    fluids: vec![FluidSlottedInput::new(s("solution.greenvitriol"), vec![(0, 2_000)]).extra_backup(1)],
                    max_sets: 8,
                },
                FluidSlottedRecipe {
                    outputs: ignore_outputs(64.),
                    inputs: vec![],
                    fluids: vec![FluidSlottedInput::new(s("carbondioxide"), vec![(0, 1_000)]).extra_backup(1)],
                    max_sets: 8,
                },
                FluidSlottedRecipe {
                    outputs: ignore_outputs(64.),
                    inputs: vec![MultiInvSlottedInput::new(label("Magnesia Dust"), vec![(0, 5, 2)])],
                    fluids: vec![],
                    max_sets: 8,
                },
                FluidSlottedRecipe {
                    outputs: FluidOutput::new(s("chlorine"), 16_000).and(Output::new(label("Sodium Dust"),16)),
                    inputs: vec![MultiInvSlottedInput::new(label("Salt"), vec![(0, 5, 2)])],
                    fluids: vec![],
                    max_sets: 8,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(label("Oxygen Cell"), 64).and(hydrogen_output()),
                    inputs: vec![
                        MultiInvSlottedInput::new(label("Phosphoric Acid Cell"), vec![(0, 5, 1)]),
                        MultiInvSlottedInput::new(label("Empty Cell"), vec![(0, 6, 6)]),
                    ],
                    fluids: vec![],
                    max_sets: 8,
                },
            ],
        });
        factory.add_process(FluidSlottedConfig {
            name: s("fluidExtractor"),
            input_slots: vec![vec![5]],
            input_tanks: vec![vec![]],
            accesses: vec![InvTankAccess {
                client: s("c2"),
                invs: vec![EachInvAccess { addr: s("5b8"), bus_side: EAST, inv_side: NORTH }],
                tanks: vec![vec![EachBusOfTank { addr: s("0c0"), bus_side: EAST, tank_side: SOUTH }]],
            }],
            to_extract: None,
            fluid_extract: fluid_extract_all(),
            strict_priority: false,
            recipes: vec![
                FluidSlottedRecipe {
                    outputs: ignore_outputs(1.),
                    inputs: vec![MultiInvSlottedInput::new(label("Glow Flower Seed"), vec![(0, 5, 1)]).extra_backup(64)],
                    fluids: vec![],
                    max_sets: 64,
                },
                FluidSlottedRecipe {
                    outputs: FluidOutput::new(s("seedoil"), 16_000),
                    inputs: vec![MultiInvSlottedInput::new(label("Sesame Seeds"), vec![(0, 5, 1)])],
                    fluids: vec![],
                    max_sets: 64,
                },
            ],
        });
        factory.add_process(FluidSlottedConfig {
            name: s("centrifuge"),
            input_slots: vec![vec![0]],
            input_tanks: vec![vec![0]],
            accesses: vec![InvTankAccess {
                client: s("c2"),
                invs: vec![EachInvAccess { addr: s("5b8"), bus_side: EAST, inv_side: WEST }],
                tanks: vec![vec![EachBusOfTank { addr: s("0c0"), bus_side: EAST, tank_side: WEST }]],
            }],
            to_extract: None,
            fluid_extract: fluid_extract_all(),
            strict_priority: false,
            recipes: vec![
                FluidSlottedRecipe {
                    outputs: Output::new(label("Bio Chaff"), 16),
                    inputs: vec![MultiInvSlottedInput::new(label("Plant Mass"), vec![(0, 0, 1)])],
                    fluids: vec![],
                    max_sets: 64,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(label("Gold Dust"), 16).and(Output::new(label("Redstone"), 16)),
                    inputs: vec![MultiInvSlottedInput::new(label("Glowstone Dust"), vec![(0, 0, 2)])],
                    fluids: vec![],
                    max_sets: 64,
                },
                FluidSlottedRecipe {
                    outputs: FluidOutput::new(s("nitrogen"), 16_000),
                    inputs: vec![],
                    fluids: vec![FluidSlottedInput::new(s("air"), vec![(0, 10_000)])],
                    max_sets: 6,
                },
            ],
        });
        factory.add_process(SlottedConfig {
            name: s("compressor"),
            accesses: vec![InvAccess { client: s("c2"), addr: s("72e"), bus_side: WEST, inv_side: UP }],
            input_slots: vec![5],
            to_extract: None,
            strict_priority: false,
            recipes: vec![
                SlottedRecipe {
                    outputs: Output::new(label("Plantball"), 16),
                    inputs: vec![SlottedInput::new(label("Sesame Seeds"), vec![(5, 8)])],
                    max_sets: 8,
                },
                SlottedRecipe {
                    outputs: Output::new(label("Compressed Air Cell"), 16),
                    inputs: vec![SlottedInput::new(label("Empty Cell"), vec![(5, 1)])],
                    max_sets: 8,
                },
            ],
        });
        factory.add_process(SlottedConfig {
            name: s("extractor"),
            accesses: vec![InvAccess { client: s("c2"), addr: s("72e"), bus_side: WEST, inv_side: SOUTH }],
            input_slots: vec![5],
            to_extract: None,
            strict_priority: false,
            recipes: vec![SlottedRecipe {
                outputs: Output::new(label("Glowstone Dust"), 16),
                inputs: vec![SlottedInput::new(label("Glow Flower"), vec![(5, 2)])],
                max_sets: 32,
            }],
        });
        factory.add_process(SlottedConfig {
            name: s("furnace"),
            accesses: vec![InvAccess { client: s("c1"), addr: s("fe0"), bus_side: NORTH, inv_side: WEST }],
            input_slots: vec![5],
            to_extract: None,
            strict_priority: false,
            recipes: vec![SlottedRecipe {
                outputs: Output::new(label("Gold Ingot"), 16),
                inputs: vec![SlottedInput::new(label("Gold Dust"), vec![(5, 1)])],
                max_sets: 16,
            }],
        });
        factory.add_process(SlottedConfig {
            name: s("alloySmelter"),
            accesses: vec![InvAccess { client: s("c1"), addr: s("fe0"), bus_side: NORTH, inv_side: SOUTH }],
            input_slots: vec![5, 6],
            to_extract: None,
            strict_priority: false,
            recipes: vec![SlottedRecipe {
                outputs: Output::new(label("Electrum Ingot"), 16),
                inputs: vec![
                    SlottedInput::new(label("Gold Dust"), vec![(5, 1)]),
                    SlottedInput::new(label("Silver Dust"), vec![(6, 1)]),
                ],
                max_sets: 8,
            }],
        });
        factory.add_process(SlottedConfig {
            name: s("bender-10"),
            accesses: vec![InvAccess { client: s("c1"), addr: s("fe0"), bus_side: NORTH, inv_side: UP }],
            input_slots: vec![5],
            to_extract: None,
            strict_priority: false,
            recipes: vec![
                SlottedRecipe {
                    outputs: Output::new(label("Gold Foil"), 16),
                    inputs: vec![SlottedInput::new(label("Gold Ingot"), vec![(5, 1)])],
                    max_sets: 8,
                },
                SlottedRecipe {
                    outputs: Output::new(label("Copper Foil"), 16),
                    inputs: vec![SlottedInput::new(label("Copper Ingot"), vec![(5, 1)])],
                    max_sets: 8,
                },
                SlottedRecipe {
                    outputs: Output::new(label("Electrum Foil"), 16),
                    inputs: vec![SlottedInput::new(label("Electrum Ingot"), vec![(5, 1)])],
                    max_sets: 8,
                },
            ],
        });
        factory.add_process(SlottedConfig {
            name: s("macerator"),
            accesses: vec![InvAccess{ client: s("main"), addr: s("c47"), bus_side: NORTH, inv_side: EAST }],
            input_slots: vec![5],
            to_extract: None,
            strict_priority: false,
            recipes: vec![SlottedRecipe {
                outputs: Output::new(label("Plant Mass"), 16),
                inputs: vec![SlottedInput::new(label("Plantball"), vec![(5, 2)])],
                max_sets: 8,
            }],
        });
        factory.add_process(BlockingFluidOutputConfig {
            accesses: vec![TankAccess {
                client: s("main"),
                buses: vec![EachBusOfTank { addr: s("608"), bus_side: SOUTH, tank_side: WEST }],
            }],
            outputs: vec![FluidOutput { fluid: s("water"), n_wanted: 16_000 }],
        })
    })
}
