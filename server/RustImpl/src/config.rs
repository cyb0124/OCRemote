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
        ],
        fluid_bus_accesses: vec![
            FluidAccess { client: s("main"), tanks: vec![EachTank { addr: s("e47"), side: SOUTH }]},
            FluidAccess { client: s("c1"), tanks: vec![EachTank { addr: s("cd4"), side: SOUTH }]},
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
        factory.add_process(LowAlert::new(label("Raw Silicon Dust"), 16));
        factory.add_storage(ChestConfig {
            accesses: vec![InvAccess { client: s("main"), addr: s("09c"), bus_side: NORTH, inv_side: SOUTH }],
        });
        factory.add_storage(DrawerConfig { accesses: vec![InvAccess{ client: s("main"), addr: s("c47"), bus_side: NORTH, inv_side: SOUTH }],
         filters: vec![label("Sesame Seeds")] });
        let hydrogen_output = || FluidOutput::new(s("hydrogen"), 16_000).or(Output::new(label("Hydrogen Cell"), 65));
        let hcl_output = || FluidOutput::new(s("hydrochloricacid_gt5u"), 33_000).or(Output::new(label("Hydrochloric Acid Cell"), 16));
        for (fluid, client, bus_of_tank) in [
            ("water", "main", EachBusOfTank { addr: s("608"), bus_side: SOUTH, tank_side: UP }),
            ("hydrogen", "main", EachBusOfTank { addr: s("608"), bus_side: SOUTH, tank_side: NORTH }),
            ("bioethanol", "main", EachBusOfTank { addr: s("e47"), bus_side: SOUTH, tank_side: EAST }),
            ("nitrofuel", "main", EachBusOfTank { addr: s("e47"), bus_side: SOUTH, tank_side: UP }),
            ("molten.polyvinylchloride", "main", EachBusOfTank { addr: s("e47"), bus_side: SOUTH, tank_side: NORTH }),
            ("biomass", "c1", EachBusOfTank { addr: s("cd4"), bus_side: SOUTH, tank_side: WEST }),
            ("ic2distilledwater", "c1", EachBusOfTank { addr: s("cd4"), bus_side: SOUTH, tank_side: UP }),
            ("molten.plastic", "c1", EachBusOfTank { addr: s("cd4"), bus_side: SOUTH, tank_side: NORTH }),
            ("oxygen", "c1", EachBusOfTank { addr: s("cd4"), bus_side: SOUTH, tank_side: EAST }),
            ("seedoil", "c1", EachBusOfTank { addr: s("6e5"), bus_side: SOUTH, tank_side: WEST }),
            ("sulfurdioxide", "c1", EachBusOfTank { addr: s("6e5"), bus_side: SOUTH, tank_side: UP }),
            ("sulfuricacid", "c1", EachBusOfTank { addr: s("6e5"), bus_side: SOUTH, tank_side: NORTH }),
            ("sulfurtrioxide", "c1", EachBusOfTank { addr: s("6e5"), bus_side: SOUTH, tank_side: EAST }),
            ("vinylchloride", "c1", EachBusOfTank { addr: s("328"), bus_side: SOUTH, tank_side: WEST }),
            ("dilutedsulfuricacid", "c1", EachBusOfTank { addr: s("328"), bus_side: SOUTH, tank_side: UP }),
            ("nitrogen", "c1", EachBusOfTank { addr: s("328"), bus_side: SOUTH, tank_side: EAST }),
            ("nitricacid", "c1", EachBusOfTank { addr: s("328"), bus_side: SOUTH, tank_side: NORTH }),
            ("chloromethane", "c1", EachBusOfTank { addr: s("500"), bus_side: NORTH, tank_side: UP }),
            ("molten.silicone", "c1", EachBusOfTank { addr: s("500"), bus_side: NORTH, tank_side: SOUTH }),
            ("dimethyldichlorosilane", "c1", EachBusOfTank { addr: s("500"), bus_side: NORTH, tank_side: WEST }),
            ("chlorine", "c1", EachBusOfTank { addr: s("500"), bus_side: NORTH, tank_side: EAST }),
            ("tetranitromethane", "c1", EachBusOfTank { addr: s("e81"), bus_side: SOUTH, tank_side: UP }),
            ("biodiesel", "c1", EachBusOfTank { addr: s("e81"), bus_side: SOUTH, tank_side: WEST }),
            ("phosphoricacid_gt5u", "c1", EachBusOfTank { addr: s("e81"), bus_side: SOUTH, tank_side: NORTH }),
            ("ironiiichloride", "c1", EachBusOfTank { addr: s("f9d"), bus_side: EAST, tank_side: SOUTH }),
            ("aceticacid", "c1", EachBusOfTank { addr: s("f9d"), bus_side: EAST, tank_side: UP }),
            ("molten.bisphenol_a", "c1", EachBusOfTank { addr: s("f9d"), bus_side: EAST, tank_side: WEST }),
            ("calciumacetatesolution", "c1", EachBusOfTank { addr: s("449"), bus_side: UP, tank_side: EAST }),
            ("dilutedhydrochloricacid_gt5u", "c1", EachBusOfTank { addr: s("449"), bus_side: UP, tank_side: DOWN }),
            ("acetone", "c1", EachBusOfTank { addr: s("449"), bus_side: UP, tank_side: SOUTH }),
            ("molten.epoxid", "main", EachBusOfTank { addr: s("269"), bus_side: EAST, tank_side: SOUTH }),
            ("saltwater", "main", EachBusOfTank { addr: s("269"), bus_side: EAST, tank_side: WEST }),
            ("hydrochloricacid_gt5u", "main", EachBusOfTank { addr: s("269"), bus_side: EAST, tank_side: NORTH }),
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
            ("c1", EachBusOfTank { addr: s("449"), bus_side: UP, tank_side: NORTH }),
            ("c1", EachBusOfTank { addr: s("f9d"), bus_side: EAST, tank_side: NORTH }),
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
        for (fluid, qty, bus_of_tank) in [
        ] {
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
                }
            ],
        });
        factory.add_process(FluidSlottedConfig {
            name: s("chemicalBath"),
            input_slots: vec![vec![5]],
            input_tanks: vec![vec![0]],
            accesses: vec![InvTankAccess {
                client: s("main"),
                invs: vec![EachInvAccess { addr: s("7c4"), bus_side: SOUTH, inv_side: WEST }],
                tanks: vec![vec![EachBusOfTank { addr: s("7c4"), bus_side: NORTH, tank_side: WEST }]],
            }],
            to_extract: None,
            fluid_extract: None,
            strict_priority: false,
            recipes: vec![FluidSlottedRecipe {
                outputs: Output::new(label("Mulch"), 16),
                inputs: vec![MultiInvSlottedInput::new(label("Bio Chaff"), vec![(0, 5, 1)])],
                fluids: vec![FluidSlottedInput::new(s("water"), vec![(0, 750)])],
                max_sets: 8,
            }],
        });
        factory.add_process(FluidSlottedConfig {
            name: s("distillery-1"),
            input_slots: vec![],
            input_tanks: vec![vec![0]],
            accesses: vec![InvTankAccess {
                client: s("main"),
                invs: vec![],
                tanks: vec![vec![EachBusOfTank { addr: s("608"), bus_side: SOUTH, tank_side: EAST }]],
            }],
            to_extract: None,
            fluid_extract: fluid_extract_slots(|_, i| i == 1),
            strict_priority: false,
            recipes: vec![
                FluidSlottedRecipe {
                    outputs: FluidOutput::new(s("acetone"), 16_000),
                    inputs: vec![],
                    fluids: vec![FluidSlottedInput::new(s("calciumacetatesolution"), vec![(0, 1_000)])],
                    max_sets: 16,
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
                    outputs: FluidOutput::new(s("tetranitromethane"), 16_000),
                    inputs: vec![MultiInvSlottedInput::new(label("Tetranitromethane Cell"), vec![(0, 0, 1)])],
                    fluids: vec![],
                    max_sets: 16,
                },
                FluidSlottedRecipe {
                    outputs: FluidOutput::new(s("hydrochloricacid_gt5u"), 16_000),
                    inputs: vec![MultiInvSlottedInput::new(label("Hydrochloric Acid Cell"), vec![(0, 0, 1)])],
                    fluids: vec![],
                    max_sets: 8,
                },
                FluidSlottedRecipe {
                    outputs: FluidOutput::new(s("aceticacid"), 16_000),
                    inputs: vec![MultiInvSlottedInput::new(label("Acetic Acid Cell"), vec![(0, 0, 1)])],
                    fluids: vec![],
                    max_sets: 8,
                },
            ],
        });
        factory.add_process(FluidSlottedConfig {
            name: s("reactor-1"),
            input_slots: vec![vec![5, 6]],
            input_tanks: vec![vec![0]],
            accesses: vec![InvTankAccess {
                client: s("main"),
                invs: vec![EachInvAccess { addr: s("039"), bus_side: SOUTH, inv_side: UP }],
                tanks: vec![vec![EachBusOfTank { addr: s("039"), bus_side: NORTH, tank_side: UP }]],
            }],
            to_extract: None,
            fluid_extract: fluid_extract_slots(|_, i| i == 1),
            strict_priority: false,
            recipes: vec![
                FluidSlottedRecipe {
                    outputs: FluidOutput::new(s("sulfurtrioxide"), 16_000),
                    inputs: vec![MultiInvSlottedInput::new(label("Oxygen Cell"), vec![(0, 5, 1)])],
                    fluids: vec![FluidSlottedInput::new(s("sulfurdioxide"), vec![(0, 1_000)])],
                    max_sets: 8,
                },
                FluidSlottedRecipe {
                    outputs: FluidOutput::new(s("sulfuricacid"), 16_000),
                    inputs: vec![MultiInvSlottedInput::new(label("Water Cell"), vec![(0, 5, 1)])],
                    fluids: vec![FluidSlottedInput::new(s("sulfurtrioxide"), vec![(0, 1_000)])],
                    max_sets: 8,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(label("Ethylene Cell"), 16),
                    inputs: vec![MultiInvSlottedInput::new(label("Ethanol Cell"), vec![(0, 5, 1)])],
                    fluids: vec![FluidSlottedInput::new(s("sulfuricacid"), vec![(0, 1_000)])],
                    max_sets: 2,
                },
                FluidSlottedRecipe {
                    outputs: FluidOutput::new(s("molten.plastic"), 32_000),
                    inputs: vec![MultiInvSlottedInput::new(label("Ethylene Cell"), vec![(0, 5, 1)])],
                    fluids: vec![FluidSlottedInput::new(s("oxygen"), vec![(0, 7_000)])],
                    max_sets: 2,
                },
                FluidSlottedRecipe {
                    outputs: FluidOutput::new(s("vinylchloride"), 16_000),
                    inputs: vec![MultiInvSlottedInput::new(label("Ethylene Cell"), vec![(0, 5, 1)])],
                    fluids: vec![FluidSlottedInput::new(s("chlorine"), vec![(0, 2_000)])],
                    max_sets: 8,
                },
                FluidSlottedRecipe {
                    outputs: FluidOutput::new(s("molten.polyvinylchloride"), 32_000),
                    inputs: vec![MultiInvSlottedInput::new(label("Oxygen Cell"), vec![(0, 5, 1)])],
                    fluids: vec![FluidSlottedInput::new(s("vinylchloride"), vec![(0, 144)])],
                    max_sets: 8,
                },
                FluidSlottedRecipe {
                    outputs: FluidOutput::new(s("chloromethane"), 16_000),
                    inputs: vec![MultiInvSlottedInput::new(label("Methane Cell"), vec![(0, 5, 1)])],
                    fluids: vec![FluidSlottedInput::new(s("chlorine"), vec![(0, 2_000)])],
                    max_sets: 8,
                },
                FluidSlottedRecipe {
                    outputs: FluidOutput::new(s("dimethyldichlorosilane"), 16_000),
                    inputs: vec![MultiInvSlottedInput::new(label("Raw Silicon Dust"), vec![(0, 5, 1)])],
                    fluids: vec![FluidSlottedInput::new(s("chloromethane"), vec![(0, 2_000)])],
                    max_sets: 8,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(label("Nitric Oxide Cell"), 16),
                    inputs: vec![MultiInvSlottedInput::new(label("Ammonia Cell"), vec![(0, 5, 4)])],
                    fluids: vec![FluidSlottedInput::new(s("oxygen"), vec![(0, 10_000)])],
                    max_sets: 3,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(label("Ethenone Cell"), 16),
                    inputs: vec![MultiInvSlottedInput::new(label("Acetic Acid Cell"), vec![(0, 5, 1)])],
                    fluids: vec![FluidSlottedInput::new(s("sulfuricacid"), vec![(0, 1_000)])],
                    max_sets: 8,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(label("Sodium Hydroxide Dust"), 16),
                    inputs: vec![MultiInvSlottedInput::new(label("Sodium Dust"), vec![(0, 5, 1)])],
                    fluids: vec![FluidSlottedInput::new(s("water"), vec![(0, 1_000)])],
                    max_sets: 8,
                },
            ],
        });
        factory.add_process(FluidSlottedConfig {
            name: s("reactor-3"),
            input_slots: vec![vec![5, 6]],
            input_tanks: vec![vec![0]],
            accesses: vec![InvTankAccess {
                client: s("c1"),
                invs: vec![EachInvAccess { addr: s("9f3"), bus_side: SOUTH, inv_side: WEST }],
                tanks: vec![vec![EachBusOfTank { addr: s("9f3"), bus_side: NORTH, tank_side: WEST }]],
            }],
            to_extract: None,
            fluid_extract: fluid_extract_slots(|_, i| i == 1),
            strict_priority: false,
            recipes: vec![
                FluidSlottedRecipe {
                    outputs: FluidOutput::new(s("biodiesel"), 16_000),
                    inputs: vec![
                        MultiInvSlottedInput::new(label("Tiny Pile of Sodium Hydroxide Dust"), vec![(0, 5, 1)]),
                        MultiInvSlottedInput::new(label("Ethanol Cell"), vec![(0, 6, 1)]),
                    ],
                    fluids: vec![FluidSlottedInput::new(s("seedoil"), vec![(0, 6_000)])],
                    max_sets: 2,
                },
                FluidSlottedRecipe {
                    outputs: FluidOutput::new(s("sulfurdioxide"), 16_000),
                    inputs: vec![MultiInvSlottedInput::new(label("Sulfur Dust"), vec![(0, 5, 1)])],
                    fluids: vec![FluidSlottedInput::new(s("oxygen"), vec![(0, 2_000)])],
                    max_sets: 8,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(label("Methane Cell"), 16),
                    inputs: vec![
                        MultiInvSlottedInput::new(label("Carbon Dust"), vec![(0, 5, 1)]),
                        MultiInvSlottedInput::new(label("Empty Cell"), vec![(0, 6, 1)]),
                    ],
                    fluids: vec![FluidSlottedInput::new(s("hydrogen"), vec![(0, 4_000)])],
                    max_sets: 8,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(label("Phosphorous Pentoxide Dust"), 16),
                    inputs: vec![MultiInvSlottedInput::new(label("Phosphorus Dust"), vec![(0, 5, 4)])],
                    fluids: vec![FluidSlottedInput::new(s("oxygen"), vec![(0, 10_000)]).allow_backup()],
                    max_sets: 3,
                },
                FluidSlottedRecipe {
                    outputs: FluidOutput::new(s("phosphoricacid_gt5u"), 16_000),
                    inputs: vec![MultiInvSlottedInput::new(label("Phosphorous Pentoxide Dust"), vec![(0, 5, 14)])],
                    fluids: vec![FluidSlottedInput::new(s("water"), vec![(0, 6_000)])],
                    max_sets: 5,
                },
                FluidSlottedRecipe {
                    outputs: FluidOutput::new(s("ironiiichloride"), 16_000),
                    inputs: vec![
                        MultiInvSlottedInput::new(label("Iron Dust"), vec![(0, 5, 1)]),
                        MultiInvSlottedInput::new(label("Empty Cell"), vec![(0, 6, 3)]),
                    ],
                    fluids: vec![FluidSlottedInput::new(s("hydrochloricacid_gt5u"), vec![(0, 3_000)])],
                    max_sets: 8,
                },
                FluidSlottedRecipe {
                    outputs: FluidOutput::new(s("molten.bisphenol_a"), 16_000),
                    inputs: vec![
                        MultiInvSlottedInput::new(label("Phenol Cell"), vec![(0, 5, 2)]),
                        MultiInvSlottedInput::new(label("Hydrochloric Acid Cell"), vec![(0, 6, 1)]),
                    ],
                    fluids: vec![FluidSlottedInput::new(s("acetone"), vec![(0, 1_000)])],
                    max_sets: 8,
                },
            ],
        });
        factory.add_process(FluidSlottedConfig {
            name: s("reactor-11"),
            input_slots: vec![vec![5, 6]],
            input_tanks: vec![vec![0]],
            accesses: vec![InvTankAccess {
                client: s("main"),
                invs: vec![EachInvAccess { addr: s("039"), bus_side: SOUTH, inv_side: EAST }],
                tanks: vec![vec![EachBusOfTank { addr: s("039"), bus_side: NORTH, tank_side: EAST }]],
            }],
            to_extract: None,
            fluid_extract: fluid_extract_slots(|_, i| i == 1),
            strict_priority: false,
            recipes: vec![
                FluidSlottedRecipe {
                    outputs: Output::new(label("Polydimethylsiloxane Pulp"), 16),
                    inputs: vec![MultiInvSlottedInput::new(label("Water Cell"), vec![(0, 5, 1)])],
                    fluids: vec![FluidSlottedInput::new(s("dimethyldichlorosilane"), vec![(0, 1_000)])],
                    max_sets: 8,
                },
                FluidSlottedRecipe {
                    outputs: FluidOutput::new(s("molten.silicone"), 32_000),
                    inputs: vec![
                        MultiInvSlottedInput::new(label("Polydimethylsiloxane Pulp"), vec![(0, 5, 9)]),
                        MultiInvSlottedInput::new(label("Sulfur Dust"), vec![(0, 6, 1)]),
                    ],
                    fluids: vec![],
                    max_sets: 2,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(label("Ammonia Cell"), 16),
                    inputs: vec![MultiInvSlottedInput::new(label("Hydrogen Cell"), vec![(0, 5, 3)])],
                    fluids: vec![FluidSlottedInput::new(s("nitrogen"), vec![(0, 1_000)])],
                    max_sets: 8,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(label("Nitrogen Dioxide Cell"), 16),
                    inputs: vec![MultiInvSlottedInput::new(label("Nitric Oxide Cell"), vec![(0, 5, 1)])],
                    fluids: vec![FluidSlottedInput::new(s("oxygen"), vec![(0, 1_000)])],
                    max_sets: 8,
                },
                FluidSlottedRecipe {
                    outputs: hcl_output(),
                    inputs: vec![MultiInvSlottedInput::new(label("Hydrogen Cell"), vec![(0, 5, 1)])],
                    fluids: vec![FluidSlottedInput::new(s("chlorine"), vec![(0, 1_000)])],
                    max_sets: 8,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(label("Epichlorohydrin Cell"), 16),
                    inputs: vec![MultiInvSlottedInput::new(label("Glycerol Cell"), vec![(0, 5, 1)])],
                    fluids: vec![FluidSlottedInput::new(s("hydrochloricacid_gt5u"), vec![(0, 1_000)])],
                    max_sets: 8,
                },
                FluidSlottedRecipe {
                    outputs: FluidOutput::new(s("molten.epoxid"), 16_000),
                    inputs: vec![
                        MultiInvSlottedInput::new(label("Sodium Hydroxide Dust"), vec![(0, 5, 6)]),
                        MultiInvSlottedInput::new(label("Epichlorohydrin Cell"), vec![(0, 6, 2)]),
                    ],
                    fluids: vec![FluidSlottedInput::new(s("molten.bisphenol_a"), vec![(0, 1_000)])],
                    max_sets: 8,
                },
            ],
        });
        factory.add_process(FluidSlottedConfig {
            name: s("reactor-19"),
            input_slots: vec![vec![5, 6]],
            input_tanks: vec![vec![0]],
            accesses: vec![InvTankAccess {
                client: s("c1"),
                invs: vec![EachInvAccess { addr: s("926"), bus_side: SOUTH, inv_side: WEST }],
                tanks: vec![vec![EachBusOfTank { addr: s("926"), bus_side: NORTH, tank_side: WEST }]],
            }],
            to_extract: None,
            fluid_extract: fluid_extract_slots(|_, i| i == 1),
            strict_priority: false,
            recipes: vec![
                FluidSlottedRecipe {
                    outputs: FluidOutput::new(s("nitricacid"), 16_000),
                    inputs: vec![
                        MultiInvSlottedInput::new(label("Water Cell"), vec![(0, 5, 1)]),
                        MultiInvSlottedInput::new(label("Nitrogen Dioxide Cell"), vec![(0, 6, 2)]),
                    ],
                    fluids: vec![FluidSlottedInput::new(s("oxygen"), vec![(0, 1_000)])],
                    max_sets: 8,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(label("Acetic Acid Cell"), 16),
                    inputs: vec![MultiInvSlottedInput::new(label("Ethylene Cell"), vec![(0, 5, 1)])],
                    fluids: vec![FluidSlottedInput::new(s("oxygen"), vec![(0, 2_000)])],
                    max_sets: 8,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(label("Tetranitromethane Cell"), 16),
                    inputs: vec![MultiInvSlottedInput::new(label("Ethenone Cell"), vec![(0, 5, 1)]),
                                MultiInvSlottedInput::new(label("Empty Cell"), vec![(0, 6, 1)])],
                    fluids: vec![FluidSlottedInput::new(s("nitricacid"), vec![(0, 8_000)])],
                    max_sets: 4,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(label("Epoxy Circuit Board"), 16),
                    inputs: vec![
                        MultiInvSlottedInput::new(label("Epoxid Sheet"), vec![(0, 5, 1)]),
                        MultiInvSlottedInput::new(label("Gold Foil"), vec![(0, 6, 8)]),
                    ],
                    fluids: vec![FluidSlottedInput::new(s("sulfuricacid"), vec![(0, 500)])],
                    max_sets: 8,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(label("Advanced Circuit Board"), 16),
                    inputs: vec![
                        MultiInvSlottedInput::new(label("Epoxy Circuit Board"), vec![(0, 5, 1)]),
                        MultiInvSlottedInput::new(label("Electrum Foil"), vec![(0, 6, 8)]),
                    ],
                    fluids: vec![FluidSlottedInput::new(s("ironiiichloride"), vec![(0, 500)])],
                    max_sets: 8,
                },
            ],
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
                FluidSlottedRecipe {
                    outputs: FluidOutput::new(s("calciumacetatesolution"), 16_000),
                    inputs: vec![
                        MultiInvSlottedInput::new(label("Empty Cell"), vec![(0, 5, 2)]),
                        MultiInvSlottedInput::new(label("Calcium Dust"), vec![(0, 6, 1)]),
                    ],
                    fluids: vec![FluidSlottedInput::new(s("aceticacid"), vec![(0, 2_000)])],
                    max_sets: 8,
                },
            ],
        });
        factory.add_process(FluidSlottedConfig {
            name: s("electrolyzer-1"),
            input_slots: vec![vec![5, 6]],
            input_tanks: vec![vec![0]],
            accesses: vec![InvTankAccess {
                client: s("c1"),
                invs: vec![EachInvAccess { addr: s("9f3"), bus_side: SOUTH, inv_side: UP }],
                tanks: vec![vec![EachBusOfTank { addr: s("9f3"), bus_side: NORTH, tank_side: UP }]],
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
                    inputs: vec![MultiInvSlottedInput::new(label("Quicklime Dust"), vec![(0, 5, 2)])],
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
                    max_sets: 1,
                },
            ],
        });
        factory.add_process(FluidSlottedConfig {
            name: s("fluidExtractor"),
            input_slots: vec![vec![5]],
            input_tanks: vec![vec![]],
            accesses: vec![InvTankAccess {
                client: s("main"),
                invs: vec![EachInvAccess { addr: s("7c4"), bus_side: SOUTH, inv_side: UP }],
                tanks: vec![vec![EachBusOfTank { addr: s("7c4"), bus_side: NORTH, tank_side: UP }]],
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
            input_slots: vec![vec![5]],
            input_tanks: vec![vec![0]],
            accesses: vec![InvTankAccess {
                client: s("c1"),
                invs: vec![EachInvAccess { addr: s("926"), bus_side: SOUTH, inv_side: UP }],
                tanks: vec![vec![EachBusOfTank { addr: s("926"), bus_side: NORTH, tank_side: UP }]],
            }],
            to_extract: None,
            fluid_extract: fluid_extract_all(),
            strict_priority: false,
            recipes: vec![
                FluidSlottedRecipe {
                    outputs: Output::new(label("Bio Chaff"), 16),
                    inputs: vec![MultiInvSlottedInput::new(label("Plant Mass"), vec![(0, 5, 1)])],
                    fluids: vec![],
                    max_sets: 8,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(label("Gold Dust"), 64),
                    inputs: vec![MultiInvSlottedInput::new(label("Glowstone Dust"), vec![(0, 5, 2)])],
                    fluids: vec![],
                    max_sets: 1,
                },
                FluidSlottedRecipe {
                    outputs: FluidOutput::new(s("nitrogen"), 16_000),
                    inputs: vec![MultiInvSlottedInput::new(label("Compressed Air Cell"), vec![(0, 5, 5)])],
                    fluids: vec![],
                    max_sets: 1,
                },
            ],
        });
        factory.add_process(SlottedConfig {
            name: s("compressor"),
            accesses: vec![InvAccess { client: s("main"), addr: s("09c"), bus_side: NORTH, inv_side: WEST }],
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
            accesses: vec![InvAccess { client: s("main"), addr: s("09c"), bus_side: NORTH, inv_side: UP }],
            input_slots: vec![5],
            to_extract: None,
            strict_priority: false,
            recipes: vec![SlottedRecipe {
                outputs: Output::new(label("Glowstone Dust"), 16),
                inputs: vec![SlottedInput::new(label("Glow Flower"), vec![(5, 2)])],
                max_sets: 8,
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
                },SlottedRecipe {
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
