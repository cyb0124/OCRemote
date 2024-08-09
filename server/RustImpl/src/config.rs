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
        bus_accesses: vec![SidedAccess { client: s("main"), addr: s("765"), side: WEST }],
        fluid_bus_accesses: vec![FluidAccess {
            client: s("main"),
            tanks: vec![EachTank { addr: s("765"), side: EAST }],
        }],
        fluid_bus_capacity: 64_000,
        backups: vec![(label("Glow Flower"), 16)],
        fluid_backups: vec![],
    }
    .build(|factory| {
        factory.add_process(ManualUiConfig { accesses: vec![] });
        factory.add_process(LowAlert::new(label("Salt"), 16));
        factory.add_process(LowAlert::new(label("Empty Cell"), 16));
        factory.add_process(LowAlert::new(label("Sulfur Dust"), 16));
        factory.add_process(LowAlert::new(label("Carbon Dust"), 16));
        factory.add_process(LowAlert::new(label("Raw Silicon Dust"), 16));
        factory.add_process(LowAlert::new(label("Tiny Pile of Sodium Hydroxide Dust"), 16));
        factory.add_storage(ChestConfig {
            accesses: vec![InvAccess { client: s("main"), addr: s("56f"), bus_side: EAST, inv_side: WEST }],
        });
        let hydrogen_output = || FluidOutput::new(s("hydrogen"), 16_000).or(Output::new(label("Hydrogen Cell"), 65));
        for (fluid, bus_of_tank) in [
            ("bioethanol", EachBusOfTank { addr: s("461"), bus_side: WEST, tank_side: SOUTH }),
            ("biomass", EachBusOfTank { addr: s("07d"), bus_side: WEST, tank_side: SOUTH }),
            ("ic2distilledwater", EachBusOfTank { addr: s("07d"), bus_side: WEST, tank_side: UP }),
            ("water", EachBusOfTank { addr: s("07d"), bus_side: WEST, tank_side: EAST }),
            ("seedoil", EachBusOfTank { addr: s("572"), bus_side: WEST, tank_side: EAST }),
            ("biodiesel", EachBusOfTank { addr: s("572"), bus_side: WEST, tank_side: SOUTH }),
            ("sulfurdioxide", EachBusOfTank { addr: s("572"), bus_side: WEST, tank_side: NORTH }),
            ("oxygen", EachBusOfTank { addr: s("4f6"), bus_side: WEST, tank_side: EAST }),
            ("sulfurtrioxide", EachBusOfTank { addr: s("4f6"), bus_side: WEST, tank_side: SOUTH }),
            ("sulfuricacid", EachBusOfTank { addr: s("4f6"), bus_side: WEST, tank_side: NORTH }),
            ("dilutedsulfuricacid", EachBusOfTank { addr: s("4f6"), bus_side: WEST, tank_side: UP }),
            ("molten.plastic", EachBusOfTank { addr: s("02a"), bus_side: WEST, tank_side: EAST }),
            ("chlorine", EachBusOfTank { addr: s("02a"), bus_side: WEST, tank_side: NORTH }),
            ("vinylchloride", EachBusOfTank { addr: s("02a"), bus_side: WEST, tank_side: UP }),
            ("molten.polyvinylchloride", EachBusOfTank { addr: s("02a"), bus_side: WEST, tank_side: SOUTH }),
            ("hydrogen", EachBusOfTank { addr: s("4fc"), bus_side: WEST, tank_side: SOUTH }),
            ("chloromethane", EachBusOfTank { addr: s("4fc"), bus_side: WEST, tank_side: UP }),
            ("dimethyldichlorosilane", EachBusOfTank { addr: s("4fc"), bus_side: WEST, tank_side: NORTH }),
            ("molten.silicone", EachBusOfTank { addr: s("4fc"), bus_side: WEST, tank_side: EAST }),
            ("nitrogen", EachBusOfTank { addr: s("23f"), bus_side: EAST, tank_side: SOUTH }),
        ] {
            factory.add_fluid_storage(FluidStorageConfig {
                accesses: vec![TankAccess { client: s("main"), buses: vec![bus_of_tank] }],
                fluid: s(fluid),
            })
        }
        for access in [InvAccess { client: s("main"), addr: s("56f"), bus_side: EAST, inv_side: UP }] {
            factory.add_process(SlottedConfig {
                name: s("output"),
                accesses: vec![access],
                input_slots: vec![],
                to_extract: extract_all(),
                strict_priority: false,
                recipes: vec![],
            })
        }
        for (access, item, qty) in
            [(InvAccess { client: s("main"), addr: s("56f"), bus_side: EAST, inv_side: NORTH }, "Glow Flower Seed", 64)]
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
            ("biodiesel", 16_000, EachBusOfTank { addr: s("461"), bus_side: WEST, tank_side: EAST }),
            ("biodiesel", 16_000, EachBusOfTank { addr: s("572"), bus_side: WEST, tank_side: UP }),
            ("biodiesel", 16_000, EachBusOfTank { addr: s("23f"), bus_side: EAST, tank_side: NORTH }),
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
            recipes: vec![CraftingGridRecipe {
                outputs: Output::new(label("Glow Flower Seed"), 16).and(Output::new(label("Glow Flower"), 1024).not()),
                inputs: vec![CraftingGridInput::new(label("Glow Flower"), vec![(0)]).allow_backup()],
                max_sets: 64,
                non_consumables: vec![],
            }],
        });
        factory.add_process(FluidSlottedConfig {
            name: s("distillery-1"),
            input_slots: vec![],
            input_tanks: vec![vec![0]],
            accesses: vec![InvTankAccess {
                client: s("main"),
                invs: vec![],
                tanks: vec![vec![EachBusOfTank { addr: s("07d"), bus_side: WEST, tank_side: NORTH }]],
            }],
            to_extract: None,
            fluid_extract: fluid_extract_slots(|_, i| i == 1),
            strict_priority: false,
            recipes: vec![
                FluidSlottedRecipe {
                    outputs: ignore_outputs(1.),
                    inputs: vec![],
                    fluids: vec![FluidSlottedInput::new(s("dilutedsulfuricacid"), vec![(0, 60)]).extra_backup(1)],
                    max_sets: 64,
                },
                FluidSlottedRecipe {
                    outputs: FluidOutput::new(s("bioethanol"), 16_000),
                    inputs: vec![],
                    fluids: vec![FluidSlottedInput::new(s("biomass"), vec![(0, 40)])],
                    max_sets: 64,
                },
            ],
        });
        factory.add_process(FluidSlottedConfig {
            name: s("distillery-5"),
            input_slots: vec![],
            input_tanks: vec![vec![0]],
            accesses: vec![InvTankAccess {
                client: s("main"),
                invs: vec![],
                tanks: vec![vec![EachBusOfTank { addr: s("461"), bus_side: WEST, tank_side: NORTH }]],
            }],
            to_extract: None,
            fluid_extract: fluid_extract_slots(|_, i| i == 1),
            strict_priority: false,
            recipes: vec![FluidSlottedRecipe {
                outputs: FluidOutput::new(s("ic2distilledwater"), 16_000),
                inputs: vec![],
                fluids: vec![FluidSlottedInput::new(s("water"), vec![(0, 5)])],
                max_sets: 64,
            }],
        });
        factory.add_process(FluidSlottedConfig {
            name: s("chemicalBath"),
            input_slots: vec![vec![5]],
            input_tanks: vec![vec![0]],
            accesses: vec![InvTankAccess {
                client: s("main"),
                invs: vec![EachInvAccess { addr: s("765"), bus_side: WEST, inv_side: NORTH }],
                tanks: vec![vec![EachBusOfTank { addr: s("765"), bus_side: EAST, tank_side: NORTH }]],
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
            name: s("brewery"),
            input_slots: vec![vec![5]],
            input_tanks: vec![vec![0]],
            accesses: vec![InvTankAccess {
                client: s("main"),
                invs: vec![EachInvAccess { addr: s("5d9"), bus_side: WEST, inv_side: NORTH }],
                tanks: vec![vec![EachBusOfTank { addr: s("5d9"), bus_side: EAST, tank_side: NORTH }]],
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
                invs: vec![EachInvAccess { addr: s("5d9"), bus_side: WEST, inv_side: SOUTH }],
                tanks: vec![vec![EachBusOfTank { addr: s("5d9"), bus_side: EAST, tank_side: SOUTH }]],
            }],
            to_extract: multi_inv_extract_all(),
            fluid_extract: None,
            strict_priority: false,
            recipes: vec![
                FluidSlottedRecipe {
                    outputs: Output::new(label("Hydrogen Cell"), 16),
                    inputs: vec![MultiInvSlottedInput::new(label("Empty Cell"), vec![(0, 0, 1)])],
                    fluids: vec![FluidSlottedInput::new(s("hydrogen"), vec![(0, 1_000)])],
                    max_sets: 8,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(label("Ethanol Cell"), 16),
                    inputs: vec![MultiInvSlottedInput::new(label("Empty Cell"), vec![(0, 0, 1)])],
                    fluids: vec![FluidSlottedInput::new(s("bioethanol"), vec![(0, 1_000)])],
                    max_sets: 8,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(label("Water Cell"), 16),
                    inputs: vec![MultiInvSlottedInput::new(label("Empty Cell"), vec![(0, 0, 1)])],
                    fluids: vec![FluidSlottedInput::new(s("water"), vec![(0, 1_000)])],
                    max_sets: 8,
                },
            ],
        });
        factory.add_process(FluidSlottedConfig {
            name: s("cellToFluid"),
            input_slots: vec![vec![0]],
            input_tanks: vec![vec![0]],
            accesses: vec![InvTankAccess {
                client: s("main"),
                invs: vec![EachInvAccess { addr: s("b8f"), bus_side: WEST, inv_side: SOUTH }],
                tanks: vec![vec![EachBusOfTank { addr: s("b8f"), bus_side: EAST, tank_side: SOUTH }]],
            }],
            to_extract: multi_inv_extract_all(),
            fluid_extract: fluid_extract_all(),
            strict_priority: false,
            recipes: vec![
                FluidSlottedRecipe {
                    outputs: ignore_outputs(1.),
                    inputs: vec![MultiInvSlottedInput::new(label("Hydrogen Cell"), vec![(0, 0, 1)]).extra_backup(64)],
                    fluids: vec![],
                    max_sets: 8,
                },
                FluidSlottedRecipe {
                    outputs: FluidOutput::new(s("oxygen"), 16_000),
                    inputs: vec![MultiInvSlottedInput::new(label("Oxygen Cell"), vec![(0, 0, 1)])],
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
                invs: vec![EachInvAccess { addr: s("b8f"), bus_side: WEST, inv_side: UP }],
                tanks: vec![vec![EachBusOfTank { addr: s("b8f"), bus_side: EAST, tank_side: UP }]],
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
            ],
        });
        factory.add_process(FluidSlottedConfig {
            name: s("reactor-3"),
            input_slots: vec![vec![5, 6]],
            input_tanks: vec![vec![0]],
            accesses: vec![InvTankAccess {
                client: s("main"),
                invs: vec![EachInvAccess { addr: s("765"), bus_side: WEST, inv_side: UP }],
                tanks: vec![vec![EachBusOfTank { addr: s("765"), bus_side: EAST, tank_side: UP }]],
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
            ],
        });
        factory.add_process(FluidSlottedConfig {
            name: s("reactor-11"),
            input_slots: vec![vec![5, 6]],
            input_tanks: vec![vec![0]],
            accesses: vec![InvTankAccess {
                client: s("main"),
                invs: vec![EachInvAccess { addr: s("23f"), bus_side: WEST, inv_side: UP }],
                tanks: vec![vec![EachBusOfTank { addr: s("23f"), bus_side: EAST, tank_side: UP }]],
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
                    outputs: Output::new(label("Nitrogen Dioxide Cell"), /* TODO: reduce */ 64),
                    inputs: vec![MultiInvSlottedInput::new(label("Nitric Oxide Cell"), vec![(0, 5, 1)])],
                    fluids: vec![FluidSlottedInput::new(s("oxygen"), vec![(0, 1_000)])],
                    max_sets: 8,
                },
            ],
        });
        factory.add_process(FluidSlottedConfig {
            name: s("electrolyzer-1"),
            input_slots: vec![vec![5, 6]],
            input_tanks: vec![vec![0]],
            accesses: vec![InvTankAccess {
                client: s("main"),
                invs: vec![EachInvAccess { addr: s("5d9"), bus_side: WEST, inv_side: UP }],
                tanks: vec![vec![EachBusOfTank { addr: s("5d9"), bus_side: EAST, tank_side: UP }]],
            }],
            to_extract: None,
            fluid_extract: fluid_extract_slots(|_, i| i == 1),
            strict_priority: false,
            recipes: vec![
                FluidSlottedRecipe {
                    outputs: ignore_outputs(1.),
                    inputs: vec![
                        MultiInvSlottedInput::new(label("Hydrochloric Acid Cell"), vec![(0, 5, 1)]).extra_backup(64)
                    ],
                    fluids: vec![],
                    max_sets: 8,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(label("Electrolyzed Water Cell"), 16),
                    inputs: vec![MultiInvSlottedInput::new(label("Water Cell"), vec![(0, 5, 1)])],
                    fluids: vec![],
                    max_sets: 2,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(label("Oxygen Cell"), 16).and(hydrogen_output()),
                    inputs: vec![MultiInvSlottedInput::new(label("Electrolyzed Water Cell"), vec![(0, 5, 1)])],
                    fluids: vec![],
                    max_sets: 8,
                },
                FluidSlottedRecipe {
                    outputs: FluidOutput::new(s("chlorine"), 16_000),
                    inputs: vec![MultiInvSlottedInput::new(label("Salt"), vec![(0, 5, 2)])],
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
                client: s("main"),
                invs: vec![EachInvAccess { addr: s("765"), bus_side: WEST, inv_side: SOUTH }],
                tanks: vec![vec![EachBusOfTank { addr: s("765"), bus_side: EAST, tank_side: SOUTH }]],
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
                client: s("main"),
                invs: vec![EachInvAccess { addr: s("b8f"), bus_side: WEST, inv_side: NORTH }],
                tanks: vec![vec![EachBusOfTank { addr: s("b8f"), bus_side: EAST, tank_side: NORTH }]],
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
            accesses: vec![InvAccess { client: s("main"), addr: s("56f"), bus_side: EAST, inv_side: SOUTH }],
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
            accesses: vec![InvAccess { client: s("main"), addr: s("27e"), bus_side: EAST, inv_side: UP }],
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
            name: s("macerator"),
            accesses: vec![InvAccess { client: s("main"), addr: s("27e"), bus_side: EAST, inv_side: NORTH }],
            input_slots: vec![5],
            to_extract: None,
            strict_priority: false,
            recipes: vec![SlottedRecipe {
                outputs: Output::new(label("Plant Mass"), 16),
                inputs: vec![SlottedInput::new(label("Plantball"), vec![(5, 2)])],
                max_sets: 8,
            }],
        });
        factory.add_process(SlottedConfig {
            name: s("trash"),
            accesses: vec![InvAccess { client: s("main"), addr: s("27e"), bus_side: EAST, inv_side: WEST }],
            input_slots: vec![0],
            to_extract: None,
            strict_priority: false,
            recipes: vec![SlottedRecipe {
                outputs: ignore_outputs(1.),
                inputs: vec![SlottedInput::new(label("Sesame Seeds"), vec![(0, 1)]).extra_backup(1024)],
                max_sets: 64,
            }],
        });
        factory.add_process(BlockingFluidOutputConfig {
            accesses: vec![TankAccess {
                client: s("main"),
                buses: vec![EachBusOfTank { addr: s("461"), bus_side: WEST, tank_side: UP }],
            }],
            outputs: vec![FluidOutput { fluid: s("water"), n_wanted: 8_000 }],
        })
    })
}
