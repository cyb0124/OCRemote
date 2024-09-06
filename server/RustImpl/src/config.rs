use crate::factory::{Factory, FactoryConfig, FluidStorageConfig};
use crate::{access::*, config_util::*, item::*, process::*, recipe::*, side::*, storage::*};
use crate::{server::Server, Tui};
use flexstr::local_fmt;
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
            SidedAccess { client: s("c3"), addr: s("189"), side: EAST },
            SidedAccess { client: s("c5"), addr: s("cdc"), side: NORTH },
            SidedAccess { client: s("cr"), addr: s("b20"), side: EAST },
        ],
        fluid_bus_accesses: vec![
            FluidAccess { client: s("main"), tanks: vec![EachTank { addr: s("e47"), side: SOUTH }] },
            FluidAccess { client: s("c1"), tanks: vec![EachTank { addr: s("cd4"), side: SOUTH }] },
            FluidAccess { client: s("c2"), tanks: vec![EachTank { addr: s("92b"), side: NORTH }] },
            FluidAccess { client: s("c3"), tanks: vec![EachTank { addr: s("324"), side: EAST }] },
            FluidAccess { client: s("c4"), tanks: vec![EachTank { addr: s("dd2"), side: SOUTH }] },
            FluidAccess { client: s("c5"), tanks: vec![EachTank { addr: s("01e"), side: NORTH }] },
            FluidAccess { client: s("cr"), tanks: vec![EachTank { addr: s("36a"), side: EAST }] },
        ],
        fluid_bus_capacity: 256_000,
        backups: vec![(label("Glow Flower"), 16)],
        fluid_backups: vec![],
    }
    .build(|factory| {
        factory.add_process(ManualUiConfig { accesses: vec![] });
        // Optional: Bauxite Dust
        // Optional: Block of Redstone
        // Optional: Impure Pile of ? Dust
        factory.add_process(FluidLowAlert(s("helium"), 16_000));
        factory.add_process(LowAlert::new(label("Chest"), 16));
        factory.add_process(LowAlert::new(label("Red Dye"), 16));
        factory.add_process(LowAlert::new(label("Interweb"), 4));
        factory.add_process(LowAlert::new(label("Salt Ore"), 16));
        factory.add_process(LowAlert::new(label("Charcoal"), 16));
        factory.add_process(LowAlert::new(label("Empty Cell"), 16));
        factory.add_process(LowAlert::new(label("Silver Ore"), 16));
        factory.add_process(LowAlert::new(label("Galena Ore"), 16));
        factory.add_process(LowAlert::new(label("Phenol Cell"), 16));
        factory.add_process(LowAlert::new(label("Cobalt Dust"), 16));
        factory.add_process(LowAlert::new(label("Benzene Cell"), 16));
        factory.add_process(LowAlert::new(label("Eye of Ender"), 16));
        factory.add_process(LowAlert::new(label("Rock Salt Ore"), 16));
        factory.add_process(LowAlert::new(label("Cryolite Dust"), 16));
        factory.add_process(LowAlert::new(label("Platinum Ingot"), 16));
        factory.add_process(LowAlert::new(label("Sphalerite Ore"), 16));
        factory.add_process(LowAlert::new(label("Neodymium Ring"), 16));
        factory.add_process(LowAlert::new(label("Manganese Dust"), 16));
        factory.add_process(LowAlert::new(label("Phosphorus Dust"), 16));
        factory.add_process(LowAlert::new(label("Molybdenum Dust"), 16));
        factory.add_process(LowAlert::new(label("Uranium 238 Dust"), 16));
        factory.add_process(LowAlert::new(label("Potassium Dichromate Dust"), 16));
        factory.add_process(LowAlert::new(label("1,2-Dimethylbenzene Cell"), 16));
        factory.add_process(LowAlert::new(label("Tungstate Dust"), 16));
        factory.add_process(LowAlert::new(label("Vanadium Dust"), 16));
        factory.add_process(LowAlert::new(label("Tantalum Dust"), 16));
        factory.add_process(LowAlert::new(label("Antimony Dust"), 16));
        factory.add_process(LowAlert::new(label("Ilmenite Ore"), 16));
        factory.add_process(LowAlert::new(label("Gallium Dust"), 16));
        factory.add_process(LowAlert::new(label("Arsenic Dust"), 16));
        factory.add_process(LowAlert::new(label("Copper Ingot"), 16));
        factory.add_process(LowAlert::new(label("Nickel Dust"), 16));
        factory.add_process(LowAlert::new(label("Lapis Dust"), 16));
        factory.add_process(LowAlert::new(label("Coal Dust"), 16));
        factory.add_process(LowAlert::new(label("Zinc Dust"), 16));
        factory.add_process(LowAlert::new(label("Tin Dust"), 16));
        factory.add_storage(ChestConfig { accesses: vec![InvAccess { client: s("main"), addr: s("09c"), bus_side: NORTH, inv_side: SOUTH }] });
        factory.add_storage(ChestConfig { accesses: vec![InvAccess { client: s("main"), addr: s("09c"), bus_side: NORTH, inv_side: EAST }] });
        factory.add_storage(ChestConfig { accesses: vec![InvAccess { client: s("main"), addr: s("09c"), bus_side: NORTH, inv_side: WEST }] });
        factory.add_storage(DrawerConfig {
            accesses: vec![InvAccess { client: s("main"), addr: s("c47"), bus_side: NORTH, inv_side: SOUTH }],
            filters: vec![label("Sesame Seeds"), label("Pyrite Dust"), label("Glow Flower"), label("Wood Pulp")],
        });
        factory.add_storage(DrawerConfig {
            accesses: vec![InvAccess { client: s("main"), addr: s("c47"), bus_side: NORTH, inv_side: EAST }],
            filters: vec![label("Gold Dust"), label("Roasted Iron Dust"), label("Sodium Dust"), label("Potassium Dust")],
        });
        let hydrogen_output = || FluidOutput::new(s("hydrogen"), 1024_000).or(Output::new(label("Hydrogen Cell"), 65));
        let oxygen_output = || FluidOutput::new(s("oxygen"), 1024_000).or(Output::new(label("Oxygen Cell"), 65));
        let hcl_output = || FluidOutput::new(s("hydrochloricacid_gt5u"), 128_000).or(Output::new(label("Hydrochloric Acid Cell"), 65));
        let plastic_cb = || custom("plasticCB", |x| x.label == "Plastic Circuit Board" && x.damage == 32007);
        let plastic_pcb = || custom("plasticPCB", |x| x.label == "Plastic Circuit Board" && x.damage == 32106);
        for (fluid, client, bus_of_tank) in [
            ("water", "main", EachBusOfTank { addr: s("608"), bus_side: SOUTH, tank_side: UP }),
            ("hydrogen", "main", EachBusOfTank { addr: s("608"), bus_side: SOUTH, tank_side: NORTH }),
            ("solution.greenvitriol", "main", EachBusOfTank { addr: s("608"), bus_side: SOUTH, tank_side: EAST }),
            ("bioethanol", "main", EachBusOfTank { addr: s("e47"), bus_side: SOUTH, tank_side: EAST }),
            ("molten.polyvinylchloride", "main", EachBusOfTank { addr: s("e47"), bus_side: SOUTH, tank_side: NORTH }),
            ("fluorine", "main", EachBusOfTank { addr: s("e47"), bus_side: SOUTH, tank_side: UP }),
            ("sulfurdioxide", "c5", EachBusOfTank { addr: s("98c"), bus_side: EAST, tank_side: DOWN }),
            ("carbonmonoxide", "c5", EachBusOfTank { addr: s("e7e"), bus_side: WEST, tank_side: UP }),
            ("molten.epoxid", "c1", EachBusOfTank { addr: s("6e2"), bus_side: EAST, tank_side: SOUTH }),
            ("saltwater", "c1", EachBusOfTank { addr: s("6e2"), bus_side: EAST, tank_side: WEST }),
            ("ammonia", "c1", EachBusOfTank { addr: s("6e2"), bus_side: EAST, tank_side: UP }),
            ("mercury", "c1", EachBusOfTank { addr: s("6e2"), bus_side: EAST, tank_side: NORTH }),
            ("molten.glowstone", "c1", EachBusOfTank { addr: s("e1c"), bus_side: WEST, tank_side: NORTH }),
            ("glycerol", "c1", EachBusOfTank { addr: s("e1c"), bus_side: WEST, tank_side: SOUTH }),
            ("lubricant", "c1", EachBusOfTank { addr: s("e1c"), bus_side: WEST, tank_side: EAST }),
            ("molten.solderingalloy", "c1", EachBusOfTank { addr: s("e1c"), bus_side: WEST, tank_side: UP }),
            ("hydrofluoricacid_gt5u", "c1", EachBusOfTank { addr: s("ffd"), bus_side: EAST, tank_side: WEST }),
            ("tetrafluoroethylene", "c1", EachBusOfTank { addr: s("ffd"), bus_side: EAST, tank_side: NORTH }),
            ("molten.polytetrafluoroethylene", "c1", EachBusOfTank { addr: s("ffd"), bus_side: EAST, tank_side: SOUTH }),
            ("chlorobenzene", "c1", EachBusOfTank { addr: s("ffd"), bus_side: EAST, tank_side: UP }),
            ("2nitrochlorobenzene", "c1", EachBusOfTank { addr: s("8db"), bus_side: WEST, tank_side: NORTH }),
            ("3,3dichlorobenzidine", "c1", EachBusOfTank { addr: s("8db"), bus_side: WEST, tank_side: UP }),
            ("phtalicacid", "c1", EachBusOfTank { addr: s("8db"), bus_side: WEST, tank_side: SOUTH }),
            ("diphenylisophtalate", "c1", EachBusOfTank { addr: s("8db"), bus_side: WEST, tank_side: EAST }),
            ("biomass", "c1", EachBusOfTank { addr: s("cd4"), bus_side: SOUTH, tank_side: WEST }),
            ("ic2distilledwater", "c1", EachBusOfTank { addr: s("cd4"), bus_side: SOUTH, tank_side: UP }),
            ("molten.plastic", "c1", EachBusOfTank { addr: s("cd4"), bus_side: SOUTH, tank_side: NORTH }),
            ("oxygen", "c1", EachBusOfTank { addr: s("cd4"), bus_side: SOUTH, tank_side: EAST }),
            ("seedoil", "c4", EachBusOfTank { addr: s("6e5"), bus_side: SOUTH, tank_side: WEST }),
            ("sulfuricacid", "c4", EachBusOfTank { addr: s("6e5"), bus_side: SOUTH, tank_side: NORTH }),
            ("methane", "c4", EachBusOfTank { addr: s("6e5"), bus_side: SOUTH, tank_side: UP }),
            ("titaniumtetrachloride", "c4", EachBusOfTank { addr: s("6e5"), bus_side: SOUTH, tank_side: EAST }),
            ("vinylchloride", "c4", EachBusOfTank { addr: s("328"), bus_side: SOUTH, tank_side: WEST }),
            ("dilutedsulfuricacid", "c4", EachBusOfTank { addr: s("328"), bus_side: SOUTH, tank_side: UP }),
            ("nitrogen", "c4", EachBusOfTank { addr: s("328"), bus_side: SOUTH, tank_side: EAST }),
            ("nitricacid", "c4", EachBusOfTank { addr: s("328"), bus_side: SOUTH, tank_side: NORTH }),
            ("aceticacid", "c4", EachBusOfTank { addr: s("926"), bus_side: NORTH, tank_side: EAST }),
            ("dilutedhydrochloricacid_gt5u", "c4", EachBusOfTank { addr: s("926"), bus_side: NORTH, tank_side: WEST }),
            ("molten.silicone", "c4", EachBusOfTank { addr: s("926"), bus_side: NORTH, tank_side: SOUTH }),
            ("tetranitromethane", "c4", EachBusOfTank { addr: s("926"), bus_side: NORTH, tank_side: UP }),
            ("3,3diaminobenzidine", "c4", EachBusOfTank { addr: s("dd2"), bus_side: SOUTH, tank_side: NORTH }),
            ("phenol", "c4", EachBusOfTank { addr: s("dd2"), bus_side: SOUTH, tank_side: WEST }),
            ("molten.polybenzimidazole", "c4", EachBusOfTank { addr: s("dd2"), bus_side: SOUTH, tank_side: EAST }),
            ("sodium tungstate", "c4", EachBusOfTank { addr: s("dd2"), bus_side: SOUTH, tank_side: UP }),
            ("acetone", "c4", EachBusOfTank { addr: s("83e"), bus_side: NORTH, tank_side: SOUTH }),
            ("chlorine", "c4", EachBusOfTank { addr: s("83e"), bus_side: NORTH, tank_side: EAST }),
            ("ironiiichloride", "c4", EachBusOfTank { addr: s("83e"), bus_side: NORTH, tank_side: UP }),
            ("hydrochloricacid_gt5u", "c4", EachBusOfTank { addr: s("83e"), bus_side: NORTH, tank_side: WEST }),
            ("vinylacetate", "c4", EachBusOfTank { addr: s("812"), bus_side: SOUTH, tank_side: NORTH }),
            ("polyvinylacetate", "c4", EachBusOfTank { addr: s("812"), bus_side: SOUTH, tank_side: EAST }),
            ("advancedglue", "c4", EachBusOfTank { addr: s("812"), bus_side: SOUTH, tank_side: WEST }),
            ("helium", "c4", EachBusOfTank { addr: s("812"), bus_side: SOUTH, tank_side: UP }),
            ("sodiumpersulfate", "c4", EachBusOfTank { addr: s("9a7"), bus_side: NORTH, tank_side: EAST }),
            ("indiumconcentrate", "c4", EachBusOfTank { addr: s("9a7"), bus_side: NORTH, tank_side: SOUTH }),
            ("leadzincsolution", "c4", EachBusOfTank { addr: s("9a7"), bus_side: NORTH, tank_side: UP }),
            ("nitrofuel", "c2", EachBusOfTank { addr: s("9a6"), bus_side: SOUTH, tank_side: NORTH }),
            ("ethylene", "c2", EachBusOfTank { addr: s("92b"), bus_side: NORTH, tank_side: SOUTH }),
            ("liquid_epichlorhydrin", "c2", EachBusOfTank { addr: s("92b"), bus_side: NORTH, tank_side: UP }),
            ("carbondioxide", "c2", EachBusOfTank { addr: s("f6a"), bus_side: SOUTH, tank_side: UP }),
            ("air", "c3", EachBusOfTank { addr: s("0c0"), bus_side: EAST, tank_side: NORTH }),
            ("phosphoricacid_gt5u", "c3", EachBusOfTank { addr: s("0c0"), bus_side: EAST, tank_side: UP }),
            ("biodiesel", "c2", EachBusOfTank { addr: s("d51"), bus_side: WEST, tank_side: SOUTH }),
            ("ethenone", "c2", EachBusOfTank { addr: s("d51"), bus_side: WEST, tank_side: NORTH }),
            ("nitrobenzene", "c3", EachBusOfTank { addr: s("324"), bus_side: EAST, tank_side: WEST }),
            ("steam", "c3", EachBusOfTank { addr: s("324"), bus_side: EAST, tank_side: SOUTH }),
            ("molten.energeticalloy", "c3", EachBusOfTank { addr: s("324"), bus_side: EAST, tank_side: UP }),
            ("silicontetrachloride", "c3", EachBusOfTank { addr: s("324"), bus_side: EAST, tank_side: NORTH }),
            ("radon", "c5", EachBusOfTank { addr: s("01e"), bus_side: NORTH, tank_side: WEST }),
            ("molten.glass", "c4", EachBusOfTank { addr: s("4c5"), bus_side: NORTH, tank_side: UP }),
        ] {
            factory
                .add_fluid_storage(FluidStorageConfig { accesses: vec![TankAccess { client: s(client), buses: vec![bus_of_tank] }], fluid: s(fluid) })
        }
        for access in [
            InvAccess { client: s("c5"), addr: s("fb3"), bus_side: DOWN, inv_side: WEST }, // EV-EBF-11-12-2
            InvAccess { client: s("main"), addr: s("c47"), bus_side: NORTH, inv_side: UP },
            InvAccess { client: s("cr"), addr: s("b20"), bus_side: EAST, inv_side: UP },
        ] {
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
            ("c2", EachBusOfTank { addr: s("d51"), bus_side: WEST, tank_side: UP }), // DT
            ("c2", EachBusOfTank { addr: s("9a6"), bus_side: SOUTH, tank_side: WEST }), // IV-LCR-1
            ("c2", EachBusOfTank { addr: s("92b"), bus_side: NORTH, tank_side: EAST }), // IV-LCR-1
            ("c2", EachBusOfTank { addr: s("f6a"), bus_side: SOUTH, tank_side: NORTH }), // EV-LCR-24
            ("c2", EachBusOfTank { addr: s("92b"), bus_side: NORTH, tank_side: WEST }), // EV-LCR-24
            ("c5", EachBusOfTank { addr: s("01e"), bus_side: NORTH, tank_side: UP }), // EV-LCR-2
            ("c5", EachBusOfTank { addr: s("e7e"), bus_side: WEST, tank_side: SOUTH }), // EV-EBF-11-12-2
            ("c3", EachBusOfTank { addr: s("f1b"), bus_side: UP, tank_side: NORTH }), // EV-EBF-1-11
            ("c5", EachBusOfTank { addr: s("ec0"), bus_side: WEST, tank_side: NORTH }), // electrolyzer
            ("c5", EachBusOfTank { addr: s("ec0"), bus_side: WEST, tank_side: UP }), // electrolyzer
        ] {
            factory.add_process(FluidSlottedConfig {
                name: s("fluidOutput"),
                input_slots: vec![],
                input_tanks: vec![vec![0]],
                accesses: vec![InvTankAccess { client: s(client), invs: vec![], tanks: vec![vec![bus_of_tank]] }],
                to_extract: None,
                fluid_extract: fluid_extract_all(),
                strict_priority: false,
                recipes: vec![],
            })
        }
        for (access, item, qty) in [(InvAccess { client: s("main"), addr: s("c47"), bus_side: NORTH, inv_side: WEST }, "Glow Flower Seed", 64)] {
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
        for (fluid, buckets, backup, client, bus_of_tank) in [
            ("lubricant", 16, 0, "c2", EachBusOfTank { addr: s("9a6"), bus_side: SOUTH, tank_side: UP }), // LCE-main
            ("oxygen", 16, 0, "c2", EachBusOfTank { addr: s("9a6"), bus_side: SOUTH, tank_side: EAST }),  // LCE-main
            ("lubricant", 16, 0, "c5", EachBusOfTank { addr: s("98c"), bus_side: EAST, tank_side: WEST }), // LCE-cr
            ("oxygen", 16, 0, "c5", EachBusOfTank { addr: s("98c"), bus_side: EAST, tank_side: SOUTH }),  // LCE-cr
            ("nitrofuel", 16, 0, "c5", EachBusOfTank { addr: s("98c"), bus_side: EAST, tank_side: UP }),  // LCE-cr
            ("ammonia", 256, 64_000, "main", EachBusOfTank { addr: s("e47"), bus_side: SOUTH, tank_side: WEST }), // supply
            ("sulfuricacid", 256, 64_000, "c4", EachBusOfTank { addr: s("9a7"), bus_side: NORTH, tank_side: WEST }), // supply
            ("ironiiichloride", 16, 64_000, "c4", EachBusOfTank { addr: s("4c5"), bus_side: NORTH, tank_side: EAST }), // supply
            ("glycerol", 256, 64_000, "c4", EachBusOfTank { addr: s("4c5"), bus_side: NORTH, tank_side: WEST }), // supply
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
                    fluids: vec![FluidSlottedInput::new(s(fluid), vec![(0, 1_000)]).extra_backup(backup)],
                    max_sets: buckets,
                }],
            })
        }
        factory.add_process(CraftingRobotConfig {
            name: s("craftingGrid"),
            accesses: vec![CraftingRobotAccess { client: s("crafter"), bus_side: FRONT }],
            recipes: (["Raw Silicon Dust", "Wood Pulp", "Salt", "Rock Salt"].into_iter())
                .map(|x| CraftingGridRecipe {
                    outputs: ignore_outputs(1.),
                    inputs: vec![CraftingGridInput::new(label!("Small Pile of {x}"), vec![0, 1, 3, 4])],
                    max_sets: 64,
                    non_consumables: vec![],
                })
                .chain(["Sodium Hydroxide", "Uranium 238", "Copper"].into_iter().map(|x| CraftingGridRecipe {
                    outputs: Output::new(label!("Tiny Pile of {x} Dust"), 16),
                    inputs: vec![CraftingGridInput::new(label!("{x} Dust"), vec![0])],
                    max_sets: 7,
                    non_consumables: vec![],
                }))
                .chain([
                    CraftingGridRecipe {
                        outputs: ignore_outputs(1.),
                        inputs: vec![CraftingGridInput::new(label("Tiny Pile of Indium Dust"), (0..9).collect())],
                        max_sets: 64,
                        non_consumables: vec![],
                    },
                    CraftingGridRecipe {
                        outputs: ignore_outputs(1.),
                        inputs: vec![CraftingGridInput::new(label("Tiny Pile of Ashes"), (0..9).collect())],
                        max_sets: 64,
                        non_consumables: vec![],
                    },
                    CraftingGridRecipe {
                        outputs: Output::new(label("Glow Flower Seed"), 16),
                        inputs: vec![CraftingGridInput::new(label("Glow Flower"), vec![0]).allow_backup()],
                        max_sets: 64,
                        non_consumables: vec![],
                    },
                    CraftingGridRecipe {
                        outputs: Output::new(label("Thickened Glass"), 16),
                        inputs: vec![CraftingGridInput::new(label("Clear Glass"), vec![0])],
                        max_sets: 64,
                        non_consumables: vec![],
                    },
                    CraftingGridRecipe {
                        outputs: Output::new(label("Electric Motor (LV)"), 4),
                        inputs: vec![
                            CraftingGridInput::new(label("Magnetic Iron Rod"), vec![4]),
                            CraftingGridInput::new(label("Iron Rod"), vec![2, 6]),
                            CraftingGridInput::new(label("1x Tin Cable"), vec![0, 8]),
                            CraftingGridInput::new(label("1x Copper Wire"), vec![1, 3, 5, 7]),
                        ],
                        max_sets: 64,
                        non_consumables: vec![],
                    },
                    CraftingGridRecipe {
                        outputs: Output::new(label("Conveyor Module (LV)"), 4),
                        inputs: vec![
                            CraftingGridInput::new(label("1x Tin Cable"), vec![4]),
                            CraftingGridInput::new(label("Electric Motor (LV)"), vec![3, 5]),
                            CraftingGridInput::new(label("Silicone Rubber Sheet"), vec![0, 1, 2, 6, 7, 8]),
                        ],
                        max_sets: 64,
                        non_consumables: vec![],
                    },
                    CraftingGridRecipe {
                        outputs: Output::new(label("Low Voltage Chest Buffer"), 4),
                        inputs: vec![
                            CraftingGridInput::new(label("Chest"), vec![0]),
                            CraftingGridInput::new(label("LV Machine Hull"), vec![1]),
                            CraftingGridInput::new(label("Conveyor Module (LV)"), vec![2]),
                            CraftingGridInput::new(label("Microprocessor"), vec![4]),
                        ],
                        max_sets: 64,
                        non_consumables: vec![],
                    },
                    CraftingGridRecipe {
                        outputs: Output::new(label("Sensor (HV)"), 4),
                        inputs: vec![
                            CraftingGridInput::new(label("Nanoprocessor"), vec![6]),
                            CraftingGridInput::new(label("Chrome Rod"), vec![4]),
                            CraftingGridInput::new(label("Eye of Ender"), vec![2]),
                            CraftingGridInput::new(label("Stainless Steel Plate"), vec![0, 3, 7, 8]),
                        ],
                        max_sets: 64,
                        non_consumables: vec![],
                    },
                    CraftingGridRecipe {
                        outputs: Output::new(label("Emitter (HV)"), 4),
                        inputs: vec![
                            CraftingGridInput::new(label("Nanoprocessor"), vec![2, 6]),
                            CraftingGridInput::new(label("Chrome Rod"), vec![0, 1, 5, 8]),
                            CraftingGridInput::new(label("Eye of Ender"), vec![4]),
                            CraftingGridInput::new(label("1x Gold Cable"), vec![3, 7]),
                        ],
                        max_sets: 64,
                        non_consumables: vec![],
                    },
                    CraftingGridRecipe {
                        outputs: Output::new(label("Red Illumar Button"), 4),
                        inputs: vec![
                            CraftingGridInput::new(name("minecraft:stone_button"), vec![0]),
                            CraftingGridInput::new(label("Red Illumar"), vec![1, 3]),
                        ],
                        max_sets: 64,
                        non_consumables: vec![],
                    },
                ])
                .collect(),
        });
        factory.add_process(FluidSlottedConfig {
            name: s("DT"),
            input_slots: vec![],
            input_tanks: vec![vec![0]],
            accesses: vec![InvTankAccess {
                client: s("c2"),
                invs: vec![],
                tanks: vec![vec![EachBusOfTank { addr: s("d51"), bus_side: WEST, tank_side: EAST }]],
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
                    outputs: FluidOutput::new(s("bioethanol"), 256_000),
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
                client: s("c5"),
                invs: vec![],
                tanks: vec![vec![EachBusOfTank { addr: s("e7e"), bus_side: WEST, tank_side: DOWN }]],
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
                    outputs: Output::new(label("Polyethylene Sheet"), 16),
                    inputs: vec![],
                    fluids: vec![FluidSlottedInput::new(s("molten.plastic"), vec![(0, 144)])],
                    max_sets: 16,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(label("Silicone Rubber Sheet"), 16),
                    inputs: vec![],
                    fluids: vec![FluidSlottedInput::new(s("molten.silicone"), vec![(0, 144)])],
                    max_sets: 16,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(label("Polyvinyl Chloride Sheet"), 16),
                    inputs: vec![],
                    fluids: vec![FluidSlottedInput::new(s("molten.polyvinylchloride"), vec![(0, 144)])],
                    max_sets: 16,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(label("Polybenzimidazole Plate"), 16),
                    inputs: vec![],
                    fluids: vec![FluidSlottedInput::new(s("molten.polybenzimidazole"), vec![(0, 144)])],
                    max_sets: 16,
                },
            ],
        });
        factory.add_process(BufferedConfig {
            name: s("vacuumFreezer"),
            accesses: vec![InvAccess { client: s("c5"), addr: s("cdc"), bus_side: NORTH, inv_side: EAST }],
            slot_filter: None,
            to_extract: None,
            max_recipe_inputs: i32::MAX,
            stocks: vec![],
            recipes: ([
                (1024, "Titanium"),
                (16, "Vanadium-Gallium"),
                (16, "Nichrome"),
                (16, "HSS-G"),
                (16, "HSS-E"),
                (16, "Tantalum"),
                (16, "Tungstensteel"),
            ]
            .into_iter())
            .map(|(q, x)| BufferedRecipe {
                outputs: Output::new(label!("{x} Ingot"), q),
                inputs: vec![BufferedInput::new(label!("Hot {x} Ingot"), 1)],
                max_inputs: 8,
            })
            .collect(),
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
                FluidSlottedRecipe {
                    outputs: Output::new(label("Fiber-Reinforced Epoxy Resin Sheet"), 16),
                    inputs: vec![MultiInvSlottedInput::new(label("Raw Carbon Fibre"), vec![(0, 5, 1)])],
                    fluids: vec![FluidSlottedInput::new(s("molten.epoxid"), vec![(0, 144)])],
                    max_sets: 8,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(label("Purified Silver Ore"), 16),
                    inputs: vec![MultiInvSlottedInput::new(label("Crushed Silver Ore"), vec![(0, 5, 1)])],
                    fluids: vec![FluidSlottedInput::new(s("mercury"), vec![(0, 1_000)])],
                    max_sets: 8,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(label("Purified Galena Ore"), 16),
                    inputs: vec![MultiInvSlottedInput::new(label("Crushed Galena Ore"), vec![(0, 5, 1)])],
                    fluids: vec![FluidSlottedInput::new(s("mercury"), vec![(0, 1_000)])],
                    max_sets: 8,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(label("Purified Sphalerite Ore"), 16),
                    inputs: vec![MultiInvSlottedInput::new(label("Crushed Sphalerite Ore"), vec![(0, 5, 1)])],
                    fluids: vec![FluidSlottedInput::new(s("sodiumpersulfate"), vec![(0, 100)])],
                    max_sets: 8,
                },
            ],
        });
        factory.add_process(FluidSlottedConfig {
            name: s("oreWasher"),
            input_slots: vec![vec![5]],
            input_tanks: vec![vec![0]],
            accesses: vec![InvTankAccess {
                client: s("c2"),
                invs: vec![EachInvAccess { addr: s("655"), bus_side: WEST, inv_side: UP }],
                tanks: vec![vec![EachBusOfTank { addr: s("655"), bus_side: EAST, tank_side: UP }]],
            }],
            to_extract: None,
            fluid_extract: None,
            strict_priority: false,
            recipes: (["Rock Salt", "Salt"].into_iter())
                .map(|x| FluidSlottedRecipe {
                    outputs: Output::new(label!("Purified {x} Ore"), 16),
                    inputs: vec![MultiInvSlottedInput::new(label!("Crushed {x} Ore"), vec![(0, 5, 1)])],
                    fluids: vec![FluidSlottedInput::new(s("ic2distilledwater"), vec![(0, 200)])],
                    max_sets: 16,
                })
                .collect(),
        });
        factory.add_process(FluidSlottedConfig {
            name: s("brewery"),
            input_slots: vec![vec![5]],
            input_tanks: vec![vec![0]],
            accesses: vec![InvTankAccess {
                client: s("c2"),
                invs: vec![EachInvAccess { addr: s("7b0"), bus_side: WEST, inv_side: UP }],
                tanks: vec![vec![EachBusOfTank { addr: s("7b0"), bus_side: EAST, tank_side: UP }]],
            }],
            to_extract: None,
            fluid_extract: fluid_extract_slots(|_, i| i == 1),
            strict_priority: false,
            recipes: vec![
                FluidSlottedRecipe {
                    outputs: FluidOutput::new(s("biomass"), 16_000),
                    inputs: vec![MultiInvSlottedInput::new(label("Mulch"), vec![(0, 5, 8)])],
                    fluids: vec![FluidSlottedInput::new(s("ic2distilledwater"), vec![(0, 375)])],
                    max_sets: 8,
                },
                FluidSlottedRecipe {
                    outputs: FluidOutput::new(s("lubricant"), 32_000),
                    inputs: vec![MultiInvSlottedInput::new(label("Redstone"), vec![(0, 5, 1)])],
                    fluids: vec![FluidSlottedInput::new(s("seedoil"), vec![(0, 750)])],
                    max_sets: 8,
                },
            ],
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
                    fluids: vec![FluidSlottedInput::new(s("phenol"), vec![(0, 1_000)]).extra_backup(1)],
                    max_sets: 32,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(label("Oxygen Cell"), 64),
                    inputs: vec![MultiInvSlottedInput::new(label("Empty Cell"), vec![(0, 0, 1)])],
                    fluids: vec![FluidSlottedInput::new(s("oxygen"), vec![(0, 1_000)])],
                    max_sets: 64,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(label("Hydrogen Cell"), 64),
                    inputs: vec![MultiInvSlottedInput::new(label("Empty Cell"), vec![(0, 0, 1)])],
                    fluids: vec![FluidSlottedInput::new(s("hydrogen"), vec![(0, 1_000)])],
                    max_sets: 64,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(label("Nitrogen Cell"), 64),
                    inputs: vec![MultiInvSlottedInput::new(label("Empty Cell"), vec![(0, 0, 1)])],
                    fluids: vec![FluidSlottedInput::new(s("nitrogen"), vec![(0, 1_000)])],
                    max_sets: 64,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(label("Bio Diesel Cell"), 64),
                    inputs: vec![MultiInvSlottedInput::new(label("Empty Cell"), vec![(0, 0, 1)])],
                    fluids: vec![FluidSlottedInput::new(s("biodiesel"), vec![(0, 1_000)])],
                    max_sets: 64,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(label("Nitric Acid Cell"), 64),
                    inputs: vec![MultiInvSlottedInput::new(label("Empty Cell"), vec![(0, 0, 1)])],
                    fluids: vec![FluidSlottedInput::new(s("nitricacid"), vec![(0, 1_000)])],
                    max_sets: 64,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(label("Sulfuric Acid Cell"), 64),
                    inputs: vec![MultiInvSlottedInput::new(label("Empty Cell"), vec![(0, 0, 1)])],
                    fluids: vec![FluidSlottedInput::new(s("sulfuricacid"), vec![(0, 1_000)])],
                    max_sets: 64,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(label("Ethanol Cell"), 64),
                    inputs: vec![MultiInvSlottedInput::new(label("Empty Cell"), vec![(0, 0, 1)])],
                    fluids: vec![FluidSlottedInput::new(s("bioethanol"), vec![(0, 1_000)])],
                    max_sets: 64,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(label("Water Cell"), 64),
                    inputs: vec![MultiInvSlottedInput::new(label("Empty Cell"), vec![(0, 0, 1)])],
                    fluids: vec![FluidSlottedInput::new(s("water"), vec![(0, 1_000)])],
                    max_sets: 64,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(label("Ethylene Cell"), 64),
                    inputs: vec![MultiInvSlottedInput::new(label("Empty Cell"), vec![(0, 0, 1)])],
                    fluids: vec![FluidSlottedInput::new(s("ethylene"), vec![(0, 1_000)])],
                    max_sets: 64,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(label("Chlorine Cell"), 64),
                    inputs: vec![MultiInvSlottedInput::new(label("Empty Cell"), vec![(0, 0, 1)])],
                    fluids: vec![FluidSlottedInput::new(s("chlorine"), vec![(0, 1_000)])],
                    max_sets: 64,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(label("Ammonia Cell"), 64),
                    inputs: vec![MultiInvSlottedInput::new(label("Empty Cell"), vec![(0, 0, 1)])],
                    fluids: vec![FluidSlottedInput::new(s("ammonia"), vec![(0, 1_000)])],
                    max_sets: 64,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(label("Hydrochloric Acid Cell"), 16),
                    inputs: vec![MultiInvSlottedInput::new(label("Empty Cell"), vec![(0, 0, 1)])],
                    fluids: vec![FluidSlottedInput::new(s("hydrochloricacid_gt5u"), vec![(0, 1_000)])],
                    max_sets: 32,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(label("Glycerol Cell"), 16),
                    inputs: vec![MultiInvSlottedInput::new(label("Empty Cell"), vec![(0, 0, 1)])],
                    fluids: vec![FluidSlottedInput::new(s("glycerol"), vec![(0, 1_000)])],
                    max_sets: 32,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(label("Epichlorohydrin Cell"), 16),
                    inputs: vec![MultiInvSlottedInput::new(label("Empty Cell"), vec![(0, 0, 1)])],
                    fluids: vec![FluidSlottedInput::new(s("liquid_epichlorhydrin"), vec![(0, 1_000)])],
                    max_sets: 32,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(label("Methane Cell"), 16),
                    inputs: vec![MultiInvSlottedInput::new(label("Empty Cell"), vec![(0, 0, 1)])],
                    fluids: vec![FluidSlottedInput::new(s("methane"), vec![(0, 1_000)])],
                    max_sets: 32,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(label("Diphenyl Isophtalate Cell"), 16),
                    inputs: vec![MultiInvSlottedInput::new(label("Empty Cell"), vec![(0, 0, 1)])],
                    fluids: vec![FluidSlottedInput::new(s("diphenylisophtalate"), vec![(0, 1_000)])],
                    max_sets: 32,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(label("Polyvinyl Acetate Cell"), 16),
                    inputs: vec![MultiInvSlottedInput::new(label("Empty Cell"), vec![(0, 0, 1)])],
                    fluids: vec![FluidSlottedInput::new(s("polyvinylacetate"), vec![(0, 1_000)])],
                    max_sets: 32,
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
                    inputs: vec![MultiInvSlottedInput::new(label("Oxygen Cell"), vec![(0, 0, 1)]).extra_backup(64)],
                    fluids: vec![],
                    max_sets: 64,
                },
                FluidSlottedRecipe {
                    outputs: ignore_outputs(1.),
                    inputs: vec![MultiInvSlottedInput::new(label("Hydrogen Cell"), vec![(0, 0, 1)]).extra_backup(64)],
                    fluids: vec![],
                    max_sets: 64,
                },
                FluidSlottedRecipe {
                    outputs: ignore_outputs(1.),
                    inputs: vec![MultiInvSlottedInput::new(label("Hydrochloric Acid Cell"), vec![(0, 0, 1)]).extra_backup(64)],
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
                    outputs: FluidOutput::new(s("air"), 64_000),
                    inputs: vec![MultiInvSlottedInput::new(label("Compressed Air Cell"), vec![(0, 0, 1)])],
                    fluids: vec![],
                    max_sets: 64,
                },
            ],
        });
        factory.add_process(FluidSlottedConfig {
            name: s("IV-LCR-1"),
            input_slots: vec![vec![0, 1, 2, 3, 4], vec![0], vec![0], vec![0]],
            input_tanks: vec![vec![0]],
            accesses: vec![InvTankAccess {
                client: s("c2"),
                invs: vec![
                    EachInvAccess { addr: s("72e"), bus_side: WEST, inv_side: SOUTH },
                    EachInvAccess { addr: s("72e"), bus_side: WEST, inv_side: NORTH },
                    EachInvAccess { addr: s("72e"), bus_side: WEST, inv_side: UP },
                    EachInvAccess { addr: s("72e"), bus_side: WEST, inv_side: EAST },
                ],
                tanks: vec![vec![EachBusOfTank { addr: s("f6a"), bus_side: SOUTH, tank_side: EAST }]],
            }],
            to_extract: multi_inv_extract_all(),
            fluid_extract: None,
            strict_priority: true,
            recipes: vec![
                FluidSlottedRecipe {
                    outputs: FluidOutput::new(s("nitrobenzene"), 64_000).map_priority(|_, x| x * 1024.),
                    inputs: vec![
                        MultiInvSlottedInput::new(label("Benzene Cell"), vec![(1, 0, 5)]),
                        MultiInvSlottedInput::new(label("Sulfuric Acid Cell"), vec![(2, 0, 3)]),
                        MultiInvSlottedInput::new(label("Nitric Acid Cell"), vec![(3, 0, 5)]),
                    ],
                    fluids: vec![FluidSlottedInput::new(s("ic2distilledwater"), vec![(0, 10_000)])],
                    max_sets: 12,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(label("Raw Silicon Dust"), 16).and(oxygen_output()).or(Output::new(label("Magnesia Dust"), 256)),
                    inputs: vec![
                        MultiInvSlottedInput::new(label("Silicon Dioxide Dust"), vec![(0, 0, 1)]),
                        MultiInvSlottedInput::new(label("Silicon Dioxide Dust"), vec![(0, 1, 1)]),
                        MultiInvSlottedInput::new(label("Silicon Dioxide Dust"), vec![(0, 2, 1)]),
                        MultiInvSlottedInput::new(label("Magnesium Dust"), vec![(0, 3, 1)]),
                        MultiInvSlottedInput::new(label("Magnesium Dust"), vec![(0, 4, 1)]),
                    ],
                    fluids: vec![],
                    max_sets: 64,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(label("Silicon Solar Grade (Poly SI) Dust"), 64),
                    inputs: vec![MultiInvSlottedInput::new(label("Sodium Dust"), vec![(0, 0, 4)])],
                    fluids: vec![FluidSlottedInput::new(s("silicontetrachloride"), vec![(0, 1_000)])],
                    max_sets: 64,
                },
                FluidSlottedRecipe {
                    outputs: hcl_output(),
                    inputs: vec![MultiInvSlottedInput::new(label("Hydrogen Cell"), vec![(1, 0, 1)])],
                    fluids: vec![FluidSlottedInput::new(s("chlorine"), vec![(0, 1_000)])],
                    max_sets: 64,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(label("Sodium Hydroxide Dust"), 16),
                    inputs: vec![MultiInvSlottedInput::new(label("Sodium Dust"), vec![(0, 0, 1)])],
                    fluids: vec![FluidSlottedInput::new(s("water"), vec![(0, 1_000)])],
                    max_sets: 64,
                },
                FluidSlottedRecipe {
                    outputs: FluidOutput::new(s("ethylene"), 256_000),
                    inputs: vec![MultiInvSlottedInput::new(label("Ethanol Cell"), vec![(1, 0, 1)])],
                    fluids: vec![FluidSlottedInput::new(s("sulfuricacid"), vec![(0, 1_000)])],
                    max_sets: 64,
                },
                FluidSlottedRecipe {
                    outputs: FluidOutput::new(s("ammonia"), 256_000),
                    inputs: vec![MultiInvSlottedInput::new(label("Nitrogen Cell"), vec![(1, 0, 1)])],
                    fluids: vec![FluidSlottedInput::new(s("hydrogen"), vec![(0, 3_000)])],
                    max_sets: 42,
                },
                FluidSlottedRecipe {
                    outputs: FluidOutput::new(s("phosphoricacid_gt5u"), 256_000),
                    inputs: vec![
                        MultiInvSlottedInput::new(label("Phosphorous Pentoxide Dust"), vec![(0, 0, 3)]),
                        MultiInvSlottedInput::new(label("Phosphorous Pentoxide Dust"), vec![(0, 1, 3)]),
                        MultiInvSlottedInput::new(label("Phosphorous Pentoxide Dust"), vec![(0, 2, 3)]),
                        MultiInvSlottedInput::new(label("Phosphorous Pentoxide Dust"), vec![(0, 3, 3)]),
                        MultiInvSlottedInput::new(label("Phosphorous Pentoxide Dust"), vec![(0, 4, 2)]),
                    ],
                    fluids: vec![FluidSlottedInput::new(s("water"), vec![(0, 6_000)])],
                    max_sets: 21,
                },
                FluidSlottedRecipe {
                    outputs: FluidOutput::new(s("hydrofluoricacid_gt5u"), 16_000),
                    inputs: vec![MultiInvSlottedInput::new(label("Hydrogen Cell"), vec![(1, 0, 1)])],
                    fluids: vec![FluidSlottedInput::new(s("fluorine"), vec![(0, 1_000)])],
                    max_sets: 16,
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
                    outputs: FluidOutput::new(s("molten.polytetrafluoroethylene"), 32_000),
                    inputs: vec![MultiInvSlottedInput::new(label("Oxygen Cell"), vec![(1, 0, 1)])],
                    fluids: vec![FluidSlottedInput::new(s("tetrafluoroethylene"), vec![(0, 144)])],
                    max_sets: 16,
                },
                FluidSlottedRecipe {
                    outputs: FluidOutput::new(s("polyvinylacetate"), 16_000),
                    inputs: vec![MultiInvSlottedInput::new(label("Oxygen Cell"), vec![(1, 0, 1)])],
                    fluids: vec![FluidSlottedInput::new(s("vinylacetate"), vec![(0, 144)])],
                    max_sets: 16,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(label("Epoxy Circuit Board"), 16),
                    inputs: vec![
                        MultiInvSlottedInput::new(label("Epoxid Sheet"), vec![(0, 0, 1)]),
                        MultiInvSlottedInput::new(label("Gold Foil"), vec![(0, 1, 8)]),
                    ],
                    fluids: vec![FluidSlottedInput::new(s("sulfuricacid"), vec![(0, 500)])],
                    max_sets: 16,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(label("Advanced Circuit Board"), 16),
                    inputs: vec![
                        MultiInvSlottedInput::new(label("Epoxy Circuit Board"), vec![(0, 0, 1)]),
                        MultiInvSlottedInput::new(label("Electrum Foil"), vec![(0, 1, 8)]),
                    ],
                    fluids: vec![FluidSlottedInput::new(s("ironiiichloride"), vec![(0, 500)])],
                    max_sets: 16,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(label("Fiber-Reinforced Circuit Board"), 16),
                    inputs: vec![
                        MultiInvSlottedInput::new(label("Fiber-Reinforced Epoxy Resin Sheet"), vec![(0, 0, 1)]),
                        MultiInvSlottedInput::new(label("Aluminium Foil"), vec![(0, 1, 12)]),
                    ],
                    fluids: vec![FluidSlottedInput::new(s("sulfuricacid"), vec![(0, 500)])],
                    max_sets: 16,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(label("More Advanced Circuit Board"), 16),
                    inputs: vec![
                        MultiInvSlottedInput::new(label("Fiber-Reinforced Circuit Board"), vec![(0, 0, 1)]),
                        MultiInvSlottedInput::new(label("Energetic Alloy Foil"), vec![(0, 1, 12)]),
                    ],
                    fluids: vec![FluidSlottedInput::new(s("ironiiichloride"), vec![(0, 1_000)])],
                    max_sets: 16,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(plastic_cb(), 16),
                    inputs: vec![
                        MultiInvSlottedInput::new(label("Polyvinyl Chloride Sheet"), vec![(0, 0, 1)]),
                        MultiInvSlottedInput::new(label("Copper Foil"), vec![(0, 1, 4)]),
                    ],
                    fluids: vec![FluidSlottedInput::new(s("sulfuricacid"), vec![(0, 500)])],
                    max_sets: 16,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(plastic_pcb(), 16),
                    inputs: vec![
                        MultiInvSlottedInput::new(plastic_cb(), vec![(0, 0, 1)]),
                        MultiInvSlottedInput::new(label("Copper Foil"), vec![(0, 1, 6)]),
                    ],
                    fluids: vec![FluidSlottedInput::new(s("ironiiichloride"), vec![(0, 250)])],
                    max_sets: 16,
                },
                FluidSlottedRecipe {
                    outputs: FluidOutput::new(s("indiumconcentrate"), 16_000),
                    inputs: vec![
                        MultiInvSlottedInput::new(label("Purified Galena Ore"), vec![(0, 0, 3)]),
                        MultiInvSlottedInput::new(label("Purified Sphalerite Ore"), vec![(0, 1, 1)]),
                    ],
                    fluids: vec![FluidSlottedInput::new(s("sulfuricacid"), vec![(0, 4_000)])],
                    max_sets: 16,
                },
                FluidSlottedRecipe {
                    outputs: FluidOutput::new(s("molten.silicone"), 32_000),
                    inputs: vec![
                        MultiInvSlottedInput::new(label("Polydimethylsiloxane Pulp"), vec![(0, 0, 9)]),
                        MultiInvSlottedInput::new(label("Sulfur Dust"), vec![(0, 1, 1)]),
                    ],
                    fluids: vec![],
                    max_sets: 16,
                },
                FluidSlottedRecipe {
                    outputs: FluidOutput::new(s("methane"), 16_000),
                    inputs: vec![MultiInvSlottedInput::new(label("Carbon Dust"), vec![(0, 0, 1)])],
                    fluids: vec![FluidSlottedInput::new(s("hydrogen"), vec![(0, 4_000)])],
                    max_sets: 16,
                },
                FluidSlottedRecipe {
                    outputs: FluidOutput::new(s("chlorobenzene"), 16_000),
                    inputs: vec![MultiInvSlottedInput::new(label("Benzene Cell"), vec![(1, 0, 1)])],
                    fluids: vec![FluidSlottedInput::new(s("chlorine"), vec![(0, 2_000)])],
                    max_sets: 16,
                },
                FluidSlottedRecipe {
                    outputs: FluidOutput::new(s("2nitrochlorobenzene"), 16_000),
                    inputs: vec![MultiInvSlottedInput::new(label("Nitration Mixture Cell"), vec![(1, 0, 2)])],
                    fluids: vec![FluidSlottedInput::new(s("chlorobenzene"), vec![(0, 1_000)])],
                    max_sets: 16,
                },
                FluidSlottedRecipe {
                    outputs: FluidOutput::new(s("3,3dichlorobenzidine"), 16_000),
                    inputs: vec![MultiInvSlottedInput::new(label("Tiny Pile of Copper Dust"), vec![(0, 0, 1)])],
                    fluids: vec![FluidSlottedInput::new(s("2nitrochlorobenzene"), vec![(0, 2_000)])],
                    max_sets: 16,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(label("Sodium Bisulfate Dust"), 16),
                    inputs: vec![MultiInvSlottedInput::new(label("Sodium Hydroxide Dust"), vec![(0, 0, 3)])],
                    fluids: vec![FluidSlottedInput::new(s("sulfuricacid"), vec![(0, 1_000)])],
                    max_sets: 16,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(label("Phosphorous Pentoxide Dust"), 320),
                    inputs: vec![MultiInvSlottedInput::new(label("Phosphorus Dust"), vec![(0, 0, 4)])],
                    fluids: vec![FluidSlottedInput::new(s("oxygen"), vec![(0, 10_000)])],
                    max_sets: 12,
                },
                FluidSlottedRecipe {
                    outputs: FluidOutput::new(s("ethenone"), 16_000),
                    inputs: vec![MultiInvSlottedInput::new(label("Sulfuric Acid Cell"), vec![(1, 0, 1)])],
                    fluids: vec![FluidSlottedInput::new(s("aceticacid"), vec![(0, 1_000)])],
                    max_sets: 8,
                },
                FluidSlottedRecipe {
                    outputs: FluidOutput::new(s("diphenylisophtalate"), 16_000),
                    inputs: vec![
                        MultiInvSlottedInput::new(label("Sulfuric Acid Cell"), vec![(1, 0, 1)]),
                        MultiInvSlottedInput::new(label("Phenol Cell"), vec![(2, 0, 2)]),
                    ],
                    fluids: vec![FluidSlottedInput::new(s("phtalicacid"), vec![(0, 1_000)])],
                    max_sets: 8,
                },
                FluidSlottedRecipe {
                    outputs: FluidOutput::new(s("3,3diaminobenzidine"), 16_000),
                    inputs: vec![
                        MultiInvSlottedInput::new(label("Zinc Dust"), vec![(0, 0, 1)]),
                        MultiInvSlottedInput::new(label("Ammonia Cell"), vec![(1, 0, 2)]),
                    ],
                    fluids: vec![FluidSlottedInput::new(s("3,3dichlorobenzidine"), vec![(0, 1_000)])],
                    max_sets: 8,
                },
                FluidSlottedRecipe {
                    outputs: FluidOutput::new(s("molten.polybenzimidazole"), 32_000),
                    inputs: vec![MultiInvSlottedInput::new(label("Diphenyl Isophtalate Cell"), vec![(1, 0, 1)])],
                    fluids: vec![FluidSlottedInput::new(s("3,3diaminobenzidine"), vec![(0, 1_000)])],
                    max_sets: 8,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(label("Indium Dust"), 16).or(Output::new(label("Tiny Pile of Indium Dust"), 9)),
                    inputs: vec![MultiInvSlottedInput::new(label("Aluminium Dust"), vec![(0, 0, 4)])],
                    fluids: vec![FluidSlottedInput::new(s("indiumconcentrate"), vec![(0, 8_000)])],
                    max_sets: 8,
                },
                FluidSlottedRecipe {
                    outputs: FluidOutput::new(s("tetranitromethane"), 16_000),
                    inputs: vec![MultiInvSlottedInput::new(label("Nitric Acid Cell"), vec![(1, 0, 8)])],
                    fluids: vec![FluidSlottedInput::new(s("ethenone"), vec![(0, 1_000)])],
                    max_sets: 4,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(label("NanoCPU Wafer"), 4),
                    inputs: vec![
                        MultiInvSlottedInput::new(label("Central Processing Unit (Wafer)"), vec![(0, 0, 1)]),
                        MultiInvSlottedInput::new(label("Raw Carbon Fibre"), vec![(0, 1, 16)]),
                    ],
                    fluids: vec![FluidSlottedInput::new(s("molten.glowstone"), vec![(0, 576)])],
                    max_sets: 4,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(label("QBit Wafer"), 4),
                    inputs: vec![
                        MultiInvSlottedInput::new(label("NanoCPU Wafer"), vec![(0, 0, 1)]),
                        MultiInvSlottedInput::new(label("Indium Gallium Phosphide Dust"), vec![(0, 1, 1)]),
                    ],
                    fluids: vec![FluidSlottedInput::new(s("radon"), vec![(0, 50)])],
                    max_sets: 4,
                },
            ],
        });
        factory.add_process(FluidSlottedConfig {
            name: s("EV-LCR-24"),
            input_slots: vec![vec![0, 1], vec![0], vec![0], vec![0]],
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
                    outputs: ignore_outputs(64.),
                    inputs: vec![
                        MultiInvSlottedInput::new(label("Magnesiumchloride Dust"), vec![(0, 0, 6)]),
                        MultiInvSlottedInput::new(label("Sodium Dust"), vec![(0, 1, 4)]),
                    ],
                    fluids: vec![],
                    max_sets: 64,
                },
                FluidSlottedRecipe {
                    outputs: FluidOutput::new(s("biodiesel"), 128_000),
                    inputs: vec![
                        MultiInvSlottedInput::new(label("Tiny Pile of Sodium Hydroxide Dust"), vec![(0, 0, 1)]),
                        MultiInvSlottedInput::new(label("Ethanol Cell"), vec![(1, 0, 1)]),
                    ],
                    fluids: vec![FluidSlottedInput::new(s("seedoil"), vec![(0, 6_000)])],
                    max_sets: 10,
                },
                FluidSlottedRecipe {
                    outputs: FluidOutput::new(s("sulfuricacid"), 256_000),
                    inputs: vec![
                        MultiInvSlottedInput::new(label("Sulfur Dust"), vec![(0, 0, 1)]),
                        MultiInvSlottedInput::new(label("Water Cell"), vec![(1, 0, 1)]),
                    ],
                    fluids: vec![FluidSlottedInput::new(s("oxygen"), vec![(0, 3_000)])],
                    max_sets: 8,
                },
                FluidSlottedRecipe {
                    outputs: FluidOutput::new(s("nitricacid"), 256_000),
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
                    outputs: FluidOutput::new(s("aceticacid"), 16_000),
                    inputs: vec![
                        MultiInvSlottedInput::new(label("Carbon Dust"), vec![(0, 0, 2)]),
                        MultiInvSlottedInput::new(label("Hydrogen Cell"), vec![(1, 0, 4)]),
                    ],
                    fluids: vec![FluidSlottedInput::new(s("oxygen"), vec![(0, 2_000)])],
                    max_sets: 8,
                },
                FluidSlottedRecipe {
                    outputs: FluidOutput::new(s("ironiiichloride"), 128_000),
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
                FluidSlottedRecipe {
                    outputs: FluidOutput::new(s("titaniumtetrachloride"), 16_000),
                    inputs: vec![
                        MultiInvSlottedInput::new(label("Rutile Dust"), vec![(0, 0, 3)]),
                        MultiInvSlottedInput::new(label("Carbon Dust"), vec![(0, 1, 2)]),
                    ],
                    fluids: vec![FluidSlottedInput::new(s("chlorine"), vec![(0, 4_000)])],
                    max_sets: 8,
                },
                FluidSlottedRecipe {
                    outputs: FluidOutput::new(s("tetrafluoroethylene"), 16_000),
                    inputs: vec![
                        MultiInvSlottedInput::new(label("Methane Cell"), vec![(1, 0, 2)]),
                        MultiInvSlottedInput::new(label("Chlorine Cell"), vec![(2, 0, 12)]),
                    ],
                    fluids: vec![FluidSlottedInput::new(s("hydrofluoricacid_gt5u"), vec![(0, 4_000)])],
                    max_sets: 8,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(label("Scheelite Dust"), 16),
                    inputs: vec![MultiInvSlottedInput::new(label("Calcium Chloride Dust"), vec![(0, 0, 3)])],
                    fluids: vec![FluidSlottedInput::new(s("sodium tungstate"), vec![(0, 1_000)])],
                    max_sets: 8,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(label("Tungstic Acid Dust"), 16),
                    inputs: vec![MultiInvSlottedInput::new(label("Scheelite Dust"), vec![(0, 0, 6)])],
                    fluids: vec![FluidSlottedInput::new(s("hydrochloricacid_gt5u"), vec![(0, 2_000)])],
                    max_sets: 8,
                },
                FluidSlottedRecipe {
                    outputs: FluidOutput::new(s("phtalicacid"), 16_000),
                    inputs: vec![
                        MultiInvSlottedInput::new(label("Potassium Dichromate Dust"), vec![(0, 0, 1)]),
                        MultiInvSlottedInput::new(label("1,2-Dimethylbenzene Cell"), vec![(1, 0, 9)]),
                    ],
                    fluids: vec![FluidSlottedInput::new(s("oxygen"), vec![(0, 54_000)])],
                    max_sets: 1,
                },
            ],
        });
        factory.add_process(FluidSlottedConfig {
            name: s("EV-LCR-2"),
            input_slots: vec![vec![0, 1], vec![0], vec![0]],
            input_tanks: vec![vec![0]],
            accesses: vec![InvTankAccess {
                client: s("c5"),
                invs: vec![
                    EachInvAccess { addr: s("cdc"), bus_side: NORTH, inv_side: WEST },
                    EachInvAccess { addr: s("cdc"), bus_side: NORTH, inv_side: SOUTH },
                    EachInvAccess { addr: s("cdc"), bus_side: NORTH, inv_side: UP },
                ],
                tanks: vec![vec![EachBusOfTank { addr: s("01e"), bus_side: NORTH, tank_side: EAST }]],
            }],
            to_extract: multi_inv_extract_all(),
            fluid_extract: None,
            strict_priority: false,
            recipes: vec![
                FluidSlottedRecipe {
                    outputs: FluidOutput::new(s("silicontetrachloride"), 16_000),
                    inputs: vec![MultiInvSlottedInput::new(label("Raw Silicon Dust"), vec![(0, 0, 1)])],
                    fluids: vec![FluidSlottedInput::new(s("chlorine"), vec![(0, 4_000)])],
                    max_sets: 16,
                },
                FluidSlottedRecipe {
                    outputs: FluidOutput::new(s("vinylacetate"), 16_000),
                    inputs: vec![
                        MultiInvSlottedInput::new(label("Oxygen Cell"), vec![(1, 0, 1)]),
                        MultiInvSlottedInput::new(label("Ethylene Cell"), vec![(2, 0, 1)]),
                    ],
                    fluids: vec![FluidSlottedInput::new(s("aceticacid"), vec![(0, 1_000)])],
                    max_sets: 16,
                },
                FluidSlottedRecipe {
                    outputs: FluidOutput::new(s("radon"), 16_000),
                    inputs: vec![
                        MultiInvSlottedInput::new(label("Plutonium 239 Ingot"), vec![(0, 0, 8)]),
                        MultiInvSlottedInput::new(label("Tiny Pile of Uranium 238 Dust"), vec![(0, 1, 1)]),
                    ],
                    fluids: vec![FluidSlottedInput::new(s("air"), vec![(0, 1_000)])],
                    max_sets: 8,
                },
            ],
        });
        let ebf_11_recipes = |idx| {
            [
                // Circuit-N
                FluidSlottedRecipe {
                    outputs: Output::new(label("Tungsten Trioxide Dust"), 16),
                    inputs: vec![MultiInvSlottedInput::new(label("Tungstic Acid Dust"), vec![(idx, 0, 7)])],
                    fluids: vec![],
                    max_sets: 16,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(label("Gallium Arsenide Crystal"), 1),
                    inputs: vec![
                        MultiInvSlottedInput::new(label("Gallium Dust"), vec![(idx, 0, 1)]),
                        MultiInvSlottedInput::new(label("Arsenic Dust"), vec![(idx, 1, 1)]),
                    ],
                    fluids: vec![],
                    max_sets: 1,
                },
                // Circuit-11
                FluidSlottedRecipe {
                    outputs: FluidOutput::new(s("sulfurdioxide"), 16_000).and(Output::new(label("Roasted Iron Dust"), 16)),
                    inputs: vec![MultiInvSlottedInput::new(label("Pyrite Dust"), vec![(idx, 0, 1)])],
                    fluids: vec![FluidSlottedInput::new(s("oxygen"), vec![(0, 3_000)])],
                    max_sets: 16,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(label("Iron Ingot"), 16),
                    inputs: vec![
                        MultiInvSlottedInput::new(label("Roasted Iron Dust"), vec![(idx, 0, 2)]),
                        MultiInvSlottedInput::new(label("Carbon Dust"), vec![(idx, 1, 1)]),
                    ],
                    fluids: vec![],
                    max_sets: 16,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(label("Steel Ingot"), 16),
                    inputs: vec![MultiInvSlottedInput::new(label("Iron Dust"), vec![(idx, 0, 1)])],
                    fluids: vec![FluidSlottedInput::new(s("oxygen"), vec![(0, 1_000)])],
                    max_sets: 16,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(label("Black Steel Ingot"), 16),
                    inputs: vec![MultiInvSlottedInput::new(label("Black Steel Dust"), vec![(idx, 0, 1)])],
                    fluids: vec![FluidSlottedInput::new(s("oxygen"), vec![(0, 1_000)])],
                    max_sets: 16,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(label("Blue Steel Ingot"), 16),
                    inputs: vec![MultiInvSlottedInput::new(label("Blue Steel Dust"), vec![(idx, 0, 1)])],
                    fluids: vec![FluidSlottedInput::new(s("oxygen"), vec![(0, 1_000)])],
                    max_sets: 16,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(label("Stainless Steel Ingot"), 17),
                    inputs: vec![MultiInvSlottedInput::new(label("Stainless Steel Dust"), vec![(idx, 0, 1)])],
                    fluids: vec![FluidSlottedInput::new(s("oxygen"), vec![(0, 1_000)])],
                    max_sets: 16,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(label("Energetic Alloy Ingot"), 16),
                    inputs: vec![MultiInvSlottedInput::new(label("Energetic Alloy Dust"), vec![(idx, 0, 1)])],
                    fluids: vec![FluidSlottedInput::new(s("hydrogen"), vec![(0, 1_000)])],
                    max_sets: 16,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(label("Aluminium Ingot"), 17),
                    inputs: vec![MultiInvSlottedInput::new(label("Aluminium Dust"), vec![(idx, 0, 1)])],
                    fluids: vec![FluidSlottedInput::new(s("nitrogen"), vec![(0, 1_000)])],
                    max_sets: 16,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(label("Hot Tungstensteel Ingot"), 16),
                    inputs: vec![MultiInvSlottedInput::new(label("Tungstensteel Dust"), vec![(idx, 0, 1)])],
                    fluids: vec![FluidSlottedInput::new(s("nitrogen"), vec![(0, 1_000)])],
                    max_sets: 1,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(label("Hot Tantalum Ingot"), 16),
                    inputs: vec![MultiInvSlottedInput::new(label("Tantalum Dust"), vec![(idx, 0, 1)])],
                    fluids: vec![FluidSlottedInput::new(s("nitrogen"), vec![(0, 1_000)])],
                    max_sets: 1,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(label("Hot Nichrome Ingot"), 16),
                    inputs: vec![MultiInvSlottedInput::new(label("Nichrome Dust"), vec![(idx, 0, 1)])],
                    fluids: vec![FluidSlottedInput::new(s("nitrogen"), vec![(0, 1_000)])],
                    max_sets: 1,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(label("Hot HSS-G Ingot"), 16),
                    inputs: vec![MultiInvSlottedInput::new(label("HSS-G Dust"), vec![(idx, 0, 1)])],
                    fluids: vec![FluidSlottedInput::new(s("nitrogen"), vec![(0, 1_000)])],
                    max_sets: 1,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(label("Hot Vanadium-Gallium Ingot"), 16),
                    inputs: vec![MultiInvSlottedInput::new(label("Vanadium-Gallium Dust"), vec![(idx, 0, 1)])],
                    fluids: vec![FluidSlottedInput::new(s("helium"), vec![(0, 1_000)])],
                    max_sets: 1,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(label("Hot HSS-E Ingot"), 16),
                    inputs: vec![MultiInvSlottedInput::new(label("HSS-E Dust"), vec![(idx, 0, 1)])],
                    fluids: vec![FluidSlottedInput::new(s("helium"), vec![(0, 1_000)])],
                    max_sets: 1,
                },
            ]
        };
        factory.add_process(FluidSlottedConfig {
            name: s("EV-EBF-11-12-2"),
            input_slots: vec![vec![0, 1], vec![0], vec![0]],
            input_tanks: vec![vec![0]],
            accesses: vec![InvTankAccess {
                client: s("c5"),
                invs: vec![
                    EachInvAccess { addr: s("fb3"), bus_side: DOWN, inv_side: SOUTH },
                    EachInvAccess { addr: s("fb3"), bus_side: DOWN, inv_side: EAST },
                    EachInvAccess { addr: s("fb3"), bus_side: DOWN, inv_side: UP },
                ],
                tanks: vec![vec![EachBusOfTank { addr: s("e7e"), bus_side: WEST, tank_side: EAST }]],
            }],
            to_extract: None,
            fluid_extract: None,
            strict_priority: false,
            recipes: (ebf_11_recipes(0).into_iter())
                .chain([
                    // Circuit-12
                    FluidSlottedRecipe {
                        outputs: Output::new(label("Hot Titanium Ingot"), 16),
                        inputs: vec![MultiInvSlottedInput::new(label("Magnesium Dust"), vec![(1, 0, 2)])],
                        fluids: vec![FluidSlottedInput::new(s("titaniumtetrachloride"), vec![(0, 1_000)])],
                        max_sets: 16,
                    },
                    // Circuit-2
                    FluidSlottedRecipe {
                        outputs: Output::new(label("Tungsten Dust"), 16),
                        inputs: vec![MultiInvSlottedInput::new(label("Tungsten Trioxide Dust"), vec![(2, 0, 4)])],
                        fluids: vec![FluidSlottedInput::new(s("hydrogen"), vec![(0, 6_000)])],
                        max_sets: 10,
                    },
                ])
                .collect(),
        });
        factory.add_process(FluidSlottedConfig {
            name: s("EV-EBF-1-11"),
            input_slots: vec![vec![0], vec![0, 1]],
            input_tanks: vec![vec![0]],
            accesses: vec![InvTankAccess {
                client: s("c3"),
                invs: vec![
                    EachInvAccess { addr: s("0fa"), bus_side: WEST, inv_side: EAST },
                    EachInvAccess { addr: s("0fa"), bus_side: WEST, inv_side: NORTH },
                ],
                tanks: vec![vec![EachBusOfTank { addr: s("f1b"), bus_side: UP, tank_side: WEST }]],
            }],
            to_extract: None,
            fluid_extract: None,
            strict_priority: false,
            recipes: (ebf_11_recipes(1).into_iter())
                .chain([
                    // Circuit-1
                    FluidSlottedRecipe {
                        outputs: Output::new(label("Chrome Ingot"), 16),
                        inputs: vec![MultiInvSlottedInput::new(label("Chrome Dust"), vec![(0, 0, 1)])],
                        fluids: vec![],
                        max_sets: 16,
                    },
                    FluidSlottedRecipe {
                        outputs: Output::new(label("Clear Glass"), 16),
                        inputs: vec![MultiInvSlottedInput::new(label("Glass"), vec![(0, 0, 1)])],
                        fluids: vec![],
                        max_sets: 16,
                    },
                ])
                .collect(),
        });
        factory.add_process(FluidSlottedConfig {
            name: s("IV-EBF-3-22"),
            input_slots: vec![vec![0, 1, 2], vec![0]],
            input_tanks: vec![vec![0]],
            accesses: vec![InvTankAccess {
                client: s("c5"),
                invs: vec![
                    EachInvAccess { addr: s("64f"), bus_side: EAST, inv_side: SOUTH },
                    EachInvAccess { addr: s("64f"), bus_side: EAST, inv_side: WEST },
                ],
                tanks: vec![vec![EachBusOfTank { addr: s("01e"), bus_side: NORTH, tank_side: SOUTH }]],
            }],
            to_extract: None,
            fluid_extract: None,
            strict_priority: false,
            recipes: vec![
                // Circuit-3
                FluidSlottedRecipe {
                    outputs: Output::new(label("Phosphorus doped Monocrystalline Silicon Boule"), 1),
                    inputs: vec![
                        MultiInvSlottedInput::new(label("Silicon Solar Grade (Poly SI) Dust"), vec![(0, 0, 64)]),
                        MultiInvSlottedInput::new(label("Small Gallium Arsenide Crystal"), vec![(0, 1, 2)]),
                        MultiInvSlottedInput::new(label("Phosphorus Dust"), vec![(0, 2, 8)]),
                    ],
                    fluids: vec![FluidSlottedInput::new(s("nitrogen"), vec![(0, 8_000)])],
                    max_sets: 1,
                },
                // Circuit-22
                FluidSlottedRecipe {
                    outputs: Output::new(label("Graphite Dust"), 64),
                    inputs: vec![MultiInvSlottedInput::new(label("Silicon Carbide Dust"), vec![(1, 0, 2)])],
                    fluids: vec![FluidSlottedInput::new(s("nitrogen"), vec![(0, 500)])],
                    max_sets: 8,
                },
            ],
        });
        factory.add_process(FluidSlottedConfig {
            name: s("IV-mixer-1"),
            input_slots: vec![vec![5, 6, 7, 8]],
            input_tanks: vec![vec![0]],
            accesses: vec![InvTankAccess {
                client: s("main"),
                invs: vec![EachInvAccess { addr: s("039"), bus_side: SOUTH, inv_side: UP }],
                tanks: vec![vec![EachBusOfTank { addr: s("039"), bus_side: NORTH, tank_side: UP }]],
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
                    outputs: Output::new(label("Electrum Dust"), 16),
                    inputs: vec![
                        MultiInvSlottedInput::new(label("Gold Dust"), vec![(0, 5, 1)]),
                        MultiInvSlottedInput::new(label("Silver Dust"), vec![(0, 6, 1)]),
                    ],
                    fluids: vec![],
                    max_sets: 8,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(label("Indium Gallium Phosphide Dust"), 16),
                    inputs: vec![
                        MultiInvSlottedInput::new(label("Indium Dust"), vec![(0, 5, 1)]),
                        MultiInvSlottedInput::new(label("Gallium Dust"), vec![(0, 6, 1)]),
                        MultiInvSlottedInput::new(label("Phosphorus Dust"), vec![(0, 7, 1)]),
                    ],
                    fluids: vec![],
                    max_sets: 8,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(label("Black Bronze Dust"), 16),
                    inputs: vec![
                        MultiInvSlottedInput::new(label("Copper Dust"), vec![(0, 5, 3)]),
                        MultiInvSlottedInput::new(label("Electrum Dust"), vec![(0, 6, 2)]),
                    ],
                    fluids: vec![],
                    max_sets: 8,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(label("Black Steel Dust"), 16),
                    inputs: vec![
                        MultiInvSlottedInput::new(label("Black Bronze Dust"), vec![(0, 5, 1)]),
                        MultiInvSlottedInput::new(label("Nickel Dust"), vec![(0, 6, 1)]),
                        MultiInvSlottedInput::new(label("Steel Dust"), vec![(0, 7, 3)]),
                    ],
                    fluids: vec![],
                    max_sets: 8,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(label("Blue Steel Dust"), 16),
                    inputs: vec![
                        MultiInvSlottedInput::new(label("Rose Gold Dust"), vec![(0, 5, 1)]),
                        MultiInvSlottedInput::new(label("Brass Dust"), vec![(0, 6, 1)]),
                        MultiInvSlottedInput::new(label("Black Steel Dust"), vec![(0, 7, 4)]),
                        MultiInvSlottedInput::new(label("Steel Dust"), vec![(0, 8, 2)]),
                    ],
                    fluids: vec![],
                    max_sets: 8,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(label("Soldering Alloy Dust"), 16),
                    inputs: vec![
                        MultiInvSlottedInput::new(label("Tin Dust"), vec![(0, 5, 9)]),
                        MultiInvSlottedInput::new(label("Antimony Dust"), vec![(0, 6, 1)]),
                    ],
                    fluids: vec![],
                    max_sets: 8,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(label("Nichrome Dust"), 16),
                    inputs: vec![
                        MultiInvSlottedInput::new(label("Nickel Dust"), vec![(0, 5, 4)]),
                        MultiInvSlottedInput::new(label("Chrome Dust"), vec![(0, 6, 1)]),
                    ],
                    fluids: vec![],
                    max_sets: 8,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(label("Brass Dust"), 16),
                    inputs: vec![
                        MultiInvSlottedInput::new(label("Copper Dust"), vec![(0, 5, 3)]),
                        MultiInvSlottedInput::new(label("Zinc Dust"), vec![(0, 6, 1)]),
                    ],
                    fluids: vec![],
                    max_sets: 8,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(label("Vanadium-Gallium Dust"), 16),
                    inputs: vec![
                        MultiInvSlottedInput::new(label("Vanadium Dust"), vec![(0, 5, 3)]),
                        MultiInvSlottedInput::new(label("Gallium Dust"), vec![(0, 6, 1)]),
                    ],
                    fluids: vec![],
                    max_sets: 8,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(label("Energium Dust"), 16),
                    inputs: vec![
                        MultiInvSlottedInput::new(label("Redstone"), vec![(0, 5, 5)]),
                        MultiInvSlottedInput::new(label("Ruby Dust"), vec![(0, 6, 4)]),
                    ],
                    fluids: vec![],
                    max_sets: 8,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(label("Tungstensteel Dust"), 16),
                    inputs: vec![
                        MultiInvSlottedInput::new(label("Tungsten Dust"), vec![(0, 5, 1)]),
                        MultiInvSlottedInput::new(label("Steel Dust"), vec![(0, 6, 1)]),
                    ],
                    fluids: vec![],
                    max_sets: 8,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(label("HSS-G Dust"), 16),
                    inputs: vec![
                        MultiInvSlottedInput::new(label("Tungstensteel Dust"), vec![(0, 5, 5)]),
                        MultiInvSlottedInput::new(label("Chrome Dust"), vec![(0, 6, 1)]),
                        MultiInvSlottedInput::new(label("Molybdenum Dust"), vec![(0, 7, 2)]),
                        MultiInvSlottedInput::new(label("Vanadium Dust"), vec![(0, 8, 1)]),
                    ],
                    fluids: vec![],
                    max_sets: 8,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(label("HSS-E Dust"), 16),
                    inputs: vec![
                        MultiInvSlottedInput::new(label("HSS-G Dust"), vec![(0, 5, 6)]),
                        MultiInvSlottedInput::new(label("Cobalt Dust"), vec![(0, 6, 1)]),
                        MultiInvSlottedInput::new(label("Manganese Dust"), vec![(0, 7, 1)]),
                        MultiInvSlottedInput::new(label("Raw Silicon Dust"), vec![(0, 8, 1)]),
                    ],
                    fluids: vec![],
                    max_sets: 8,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(label("Nitration Mixture Cell"), 16),
                    inputs: vec![
                        MultiInvSlottedInput::new(label("Sulfuric Acid Cell"), vec![(0, 5, 1)]),
                        MultiInvSlottedInput::new(label("Nitric Acid Cell"), vec![(0, 6, 1)]),
                    ],
                    fluids: vec![],
                    max_sets: 8,
                },
            ],
        });
        factory.add_process(FluidSlottedConfig {
            name: s("EV-mixer-2"),
            input_slots: vec![vec![5, 6, 7, 8]],
            input_tanks: vec![vec![0]],
            accesses: vec![InvTankAccess {
                client: s("c2"),
                invs: vec![EachInvAccess { addr: s("f34"), bus_side: EAST, inv_side: UP }],
                tanks: vec![vec![EachBusOfTank { addr: s("f34"), bus_side: WEST, tank_side: UP }]],
            }],
            to_extract: None,
            fluid_extract: fluid_extract_all(),
            strict_priority: false,
            recipes: vec![
                FluidSlottedRecipe {
                    outputs: FluidOutput::new(s("advancedglue"), 16_000),
                    inputs: vec![MultiInvSlottedInput::new(label("Polyvinyl Acetate Cell"), vec![(0, 5, 1)])],
                    fluids: vec![FluidSlottedInput::new(s("acetone"), vec![(0, 1_500)])],
                    max_sets: 8,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(label("Redstone Alloy Dust"), 16),
                    inputs: vec![
                        MultiInvSlottedInput::new(label("Redstone"), vec![(0, 5, 1)]),
                        MultiInvSlottedInput::new(label("Raw Silicon Dust"), vec![(0, 6, 1)]),
                        MultiInvSlottedInput::new(label("Coal Dust"), vec![(0, 7, 1)]),
                    ],
                    fluids: vec![],
                    max_sets: 8,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(label("Conductive Iron Dust"), 16),
                    inputs: vec![
                        MultiInvSlottedInput::new(label("Redstone Alloy Dust"), vec![(0, 5, 1)]),
                        MultiInvSlottedInput::new(label("Iron Dust"), vec![(0, 6, 1)]),
                        MultiInvSlottedInput::new(label("Silver Dust"), vec![(0, 7, 1)]),
                    ],
                    fluids: vec![],
                    max_sets: 8,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(label("Stainless Steel Dust"), 16),
                    inputs: vec![
                        MultiInvSlottedInput::new(label("Iron Dust"), vec![(0, 5, 6)]),
                        MultiInvSlottedInput::new(label("Nickel Dust"), vec![(0, 6, 1)]),
                        MultiInvSlottedInput::new(label("Manganese Dust"), vec![(0, 7, 1)]),
                        MultiInvSlottedInput::new(label("Chrome Dust"), vec![(0, 8, 1)]),
                    ],
                    fluids: vec![],
                    max_sets: 8,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(label("Energetic Alloy Dust"), 16),
                    inputs: vec![
                        MultiInvSlottedInput::new(label("Conductive Iron Dust"), vec![(0, 5, 1)]),
                        MultiInvSlottedInput::new(label("Gold Dust"), vec![(0, 6, 1)]),
                        MultiInvSlottedInput::new(label("Black Steel Dust"), vec![(0, 7, 1)]),
                    ],
                    fluids: vec![],
                    max_sets: 8,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(label("Electrical Steel Dust"), 16),
                    inputs: vec![
                        MultiInvSlottedInput::new(label("Steel Dust"), vec![(0, 5, 1)]),
                        MultiInvSlottedInput::new(label("Coal Dust"), vec![(0, 6, 1)]),
                        MultiInvSlottedInput::new(label("Raw Silicon Dust"), vec![(0, 7, 1)]),
                    ],
                    fluids: vec![],
                    max_sets: 8,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(label("Rose Gold Dust"), 16),
                    inputs: vec![
                        MultiInvSlottedInput::new(label("Copper Dust"), vec![(0, 5, 1)]),
                        MultiInvSlottedInput::new(label("Gold Dust"), vec![(0, 6, 4)]),
                    ],
                    fluids: vec![],
                    max_sets: 8,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(label("Red Alloy Dust"), 16),
                    inputs: vec![
                        MultiInvSlottedInput::new(label("Copper Dust"), vec![(0, 5, 1)]),
                        MultiInvSlottedInput::new(label("Redstone"), vec![(0, 6, 4)]),
                    ],
                    fluids: vec![],
                    max_sets: 8,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(label("Lapotron Dust"), 30),
                    inputs: vec![
                        MultiInvSlottedInput::new(label("Energium Dust"), vec![(0, 5, 3)]),
                        MultiInvSlottedInput::new(label("Lapis Dust"), vec![(0, 6, 2)]),
                    ],
                    fluids: vec![],
                    max_sets: 8,
                },
            ],
        });
        factory.add_process(SlottedConfig {
            name: s("EV-mixer-8"),
            accesses: vec![InvAccess { client: s("c3"), addr: s("378"), bus_side: WEST, inv_side: SOUTH }],
            input_slots: vec![5, 6],
            to_extract: None,
            strict_priority: false,
            recipes: vec![SlottedRecipe {
                outputs: Output::new(label("Red Illumar"), 16),
                inputs: vec![SlottedInput::new(label("Glowstone Dust"), vec![(5, 1)]), SlottedInput::new(label("Red Dye"), vec![(6, 1)])],
                max_sets: 8,
            }],
        });
        factory.add_process(FluidSlottedConfig {
            name: s("EV-mixer-12"),
            input_slots: vec![vec![5, 6, 7, 8]],
            input_tanks: vec![vec![0]],
            accesses: vec![InvTankAccess {
                client: s("c2"),
                invs: vec![EachInvAccess { addr: s("655"), bus_side: WEST, inv_side: SOUTH }],
                tanks: vec![vec![EachBusOfTank { addr: s("655"), bus_side: EAST, tank_side: SOUTH }]],
            }],
            to_extract: None,
            fluid_extract: fluid_extract_all(),
            strict_priority: false,
            recipes: vec![FluidSlottedRecipe {
                outputs: Output::new(label("Silicon Carbide Dust"), 16),
                inputs: vec![
                    MultiInvSlottedInput::new(label("Raw Silicon Dust"), vec![(0, 5, 1)]),
                    MultiInvSlottedInput::new(label("Carbon Dust"), vec![(0, 6, 1)]),
                ],
                fluids: vec![],
                max_sets: 16,
            }],
        });
        factory.add_process(FluidSlottedConfig {
            name: s("electrolyzer-1"),
            input_slots: vec![(0..16).collect()],
            input_tanks: vec![vec![0]],
            accesses: vec![InvTankAccess {
                client: s("c5"),
                invs: vec![EachInvAccess { addr: s("065"), bus_side: SOUTH, inv_side: NORTH }],
                tanks: vec![vec![EachBusOfTank { addr: s("ec0"), bus_side: WEST, tank_side: EAST }]],
            }],
            to_extract: None,
            fluid_extract: None,
            strict_priority: false,
            recipes: vec![
                FluidSlottedRecipe {
                    outputs: ignore_outputs(64.),
                    inputs: vec![],
                    fluids: vec![FluidSlottedInput::new(s("hydrochloricacid_gt5u"), vec![(0, 1_000)]).extra_backup(256_000)],
                    max_sets: 128,
                },
                FluidSlottedRecipe {
                    outputs: ignore_outputs(64.),
                    inputs: (0..4).map(|i| MultiInvSlottedInput::new(label("Magnesia Dust"), vec![(0, i, 1)])).collect(),
                    fluids: vec![],
                    max_sets: 64,
                },
                FluidSlottedRecipe {
                    outputs: ignore_outputs(64.),
                    inputs: vec![],
                    fluids: vec![FluidSlottedInput::new(s("saltwater"), vec![(0, 1_000)]).extra_backup(1)],
                    max_sets: 128,
                },
                FluidSlottedRecipe {
                    outputs: ignore_outputs(64.),
                    inputs: vec![],
                    fluids: vec![FluidSlottedInput::new(s("carbonmonoxide"), vec![(0, 1_000)]).extra_backup(1)],
                    max_sets: 128,
                },
                FluidSlottedRecipe {
                    outputs: ignore_outputs(64.),
                    inputs: vec![],
                    fluids: vec![FluidSlottedInput::new(s("carbondioxide"), vec![(0, 1_000)]).extra_backup(1)],
                    max_sets: 128,
                },
                FluidSlottedRecipe {
                    outputs: ignore_outputs(64.),
                    inputs: vec![],
                    fluids: vec![FluidSlottedInput::new(s("solution.greenvitriol"), vec![(0, 2_000)]).extra_backup(1)],
                    max_sets: 64,
                },
                FluidSlottedRecipe {
                    outputs: ignore_outputs(64.),
                    inputs: vec![],
                    fluids: vec![FluidSlottedInput::new(s("leadzincsolution"), vec![(0, 8_000)]).extra_backup(1)],
                    max_sets: 16,
                },
                FluidSlottedRecipe {
                    outputs: ignore_outputs(64.),
                    inputs: vec![MultiInvSlottedInput::new(label("Bauxite Dust"), (0..13).map(|i| (0, i, 3)).collect())],
                    fluids: vec![],
                    max_sets: 21,
                },
                FluidSlottedRecipe {
                    outputs: hydrogen_output(),
                    inputs: vec![],
                    fluids: vec![FluidSlottedInput::new(s("phosphoricacid_gt5u"), vec![(0, 1_000)])],
                    max_sets: 128,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(label("Sulfur Dust"), 16),
                    inputs: vec![],
                    fluids: vec![FluidSlottedInput::new(s("sulfurdioxide"), vec![(0, 1_000)])],
                    max_sets: 128,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(label("Carbon Dust"), 16),
                    inputs: vec![MultiInvSlottedInput::new(label("Charcoal Dust"), vec![(0, 0, 1)])],
                    fluids: vec![],
                    max_sets: 64,
                },
                FluidSlottedRecipe {
                    outputs: FluidOutput::new(s("chlorine"), 256_000),
                    inputs: vec![MultiInvSlottedInput::new(both("Rock Salt", "gregtech:gt.metaitem.01"), vec![(0, 0, 2)])],
                    fluids: vec![],
                    max_sets: 32,
                },
                FluidSlottedRecipe {
                    outputs: FluidOutput::new(s("chlorine"), 256_000).and(Output::new(label("Sodium Dust"), 16)),
                    inputs: vec![MultiInvSlottedInput::new(both("Salt", "gregtech:gt.metaitem.01"), vec![(0, 0, 2)])],
                    fluids: vec![],
                    max_sets: 32,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(label("Aluminium Dust"), 16),
                    inputs: vec![MultiInvSlottedInput::new(label("Alumina Dust"), vec![(0, 0, 5)])],
                    fluids: vec![],
                    max_sets: 12,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(label("Chrome Dust"), 16).and(Output::new(label("Alumina Dust"), 16)),
                    inputs: vec![MultiInvSlottedInput::new(label("Ruby Dust"), vec![(0, 0, 6)])],
                    fluids: vec![],
                    max_sets: 10,
                },
                FluidSlottedRecipe {
                    outputs: FluidOutput::new(s("sodiumpersulfate"), 16_000),
                    inputs: vec![MultiInvSlottedInput::new(label("Sodium Bisulfate Dust"), vec![(0, 0, 7)])],
                    fluids: vec![],
                    max_sets: 9,
                },
                FluidSlottedRecipe {
                    outputs: FluidOutput::new(s("fluorine"), 16_000),
                    inputs: vec![MultiInvSlottedInput::new(label("Cryolite Dust"), vec![(0, 0, 10)])],
                    fluids: vec![],
                    max_sets: 6,
                },
            ],
        });
        factory.add_process(FluidSlottedConfig {
            name: s("EV-assembler-1"),
            input_slots: vec![vec![5, 6, 7, 8, 9, 10]],
            input_tanks: vec![vec![0]],
            accesses: vec![InvTankAccess {
                client: s("c2"),
                invs: vec![EachInvAccess { addr: s("7b0"), bus_side: WEST, inv_side: SOUTH }],
                tanks: vec![vec![EachBusOfTank { addr: s("7b0"), bus_side: EAST, tank_side: SOUTH }]],
            }],
            to_extract: None,
            fluid_extract: None,
            strict_priority: false,
            recipes: vec![
                FluidSlottedRecipe {
                    outputs: Output::new(label("Computer Case (Tier 3)"), 4),
                    inputs: vec![
                        MultiInvSlottedInput::new(label("EV Machine Casing"), vec![(0, 5, 1)]),
                        MultiInvSlottedInput::new(label("Titanium Plate"), vec![(0, 6, 2)]),
                        MultiInvSlottedInput::new(label("More Advanced Circuit Board"), vec![(0, 7, 1)]),
                        MultiInvSlottedInput::new(label("Microchip (Tier 3)"), vec![(0, 8, 1)]),
                        MultiInvSlottedInput::new(label("Aluminium Rotor"), vec![(0, 9, 2)]),
                        MultiInvSlottedInput::new(label("Aluminium Casing"), vec![(0, 10, 2)]),
                    ],
                    fluids: vec![FluidSlottedInput::new(s("molten.plastic"), vec![(0, 72)])],
                    max_sets: 4,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(label("Inventory Upgrade"), 4),
                    inputs: vec![
                        MultiInvSlottedInput::new(plastic_pcb(), vec![(0, 5, 1)]),
                        MultiInvSlottedInput::new(label("Aluminium Casing"), vec![(0, 6, 2)]),
                        MultiInvSlottedInput::new(label("Chest"), vec![(0, 7, 1)]),
                        MultiInvSlottedInput::new(label("Microchip (Tier 1)"), vec![(0, 8, 1)]),
                        MultiInvSlottedInput::new(label("Silver Bolt"), vec![(0, 9, 8)]),
                    ],
                    fluids: vec![FluidSlottedInput::new(s("molten.solderingalloy"), vec![(0, 72)])],
                    max_sets: 4,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(label("Tank Upgrade"), 4),
                    inputs: vec![
                        MultiInvSlottedInput::new(plastic_pcb(), vec![(0, 5, 1)]),
                        MultiInvSlottedInput::new(label("Aluminium Casing"), vec![(0, 6, 2)]),
                        MultiInvSlottedInput::new(label("Tank"), vec![(0, 7, 1)]),
                        MultiInvSlottedInput::new(label("Silver Bolt"), vec![(0, 8, 8)]),
                    ],
                    fluids: vec![FluidSlottedInput::new(s("molten.solderingalloy"), vec![(0, 72)])],
                    max_sets: 4,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(label("Inventory Controller Upgrade"), 4),
                    inputs: vec![
                        MultiInvSlottedInput::new(label("Inventory Upgrade"), vec![(0, 5, 1)]),
                        MultiInvSlottedInput::new(label("Aluminium Casing"), vec![(0, 6, 2)]),
                        MultiInvSlottedInput::new(label("Microchip (Tier 1)"), vec![(0, 7, 1)]),
                        MultiInvSlottedInput::new(label("Electrum Bolt"), vec![(0, 8, 8)]),
                    ],
                    fluids: vec![FluidSlottedInput::new(s("molten.solderingalloy"), vec![(0, 72)])],
                    max_sets: 4,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(label("Tank Controller Upgrade"), 4),
                    inputs: vec![
                        MultiInvSlottedInput::new(label("Tank Upgrade"), vec![(0, 5, 1)]),
                        MultiInvSlottedInput::new(label("Aluminium Casing"), vec![(0, 6, 2)]),
                        MultiInvSlottedInput::new(label("Microchip (Tier 1)"), vec![(0, 7, 1)]),
                        MultiInvSlottedInput::new(label("Electrum Bolt"), vec![(0, 8, 8)]),
                    ],
                    fluids: vec![FluidSlottedInput::new(s("molten.solderingalloy"), vec![(0, 72)])],
                    max_sets: 4,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(label("Transposer"), 4),
                    inputs: vec![
                        MultiInvSlottedInput::new(label("Low Voltage Chest Buffer"), vec![(0, 5, 1)]),
                        MultiInvSlottedInput::new(label("Inventory Controller Upgrade"), vec![(0, 6, 1)]),
                        MultiInvSlottedInput::new(label("Tank Controller Upgrade"), vec![(0, 7, 1)]),
                        MultiInvSlottedInput::new(label("Polyethylene Sheet"), vec![(0, 8, 2)]),
                        MultiInvSlottedInput::new(label("Cable"), vec![(0, 9, 2)]),
                    ],
                    fluids: vec![FluidSlottedInput::new(s("molten.plastic"), vec![(0, 72)])],
                    max_sets: 4,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(label("Cable"), 16),
                    inputs: vec![
                        MultiInvSlottedInput::new(label("1x Gold Cable"), vec![(0, 5, 9)]),
                        MultiInvSlottedInput::new(label("Redstone Alloy Dust"), vec![(0, 6, 1)]),
                    ],
                    fluids: vec![],
                    max_sets: 1,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(label("Graphene Dust"), 16),
                    inputs: vec![
                        MultiInvSlottedInput::new(label("Graphite Dust"), vec![(0, 5, 64)]),
                        MultiInvSlottedInput::new(label("Phosphorus doped Wafer"), vec![(0, 6, 32)]),
                    ],
                    fluids: vec![FluidSlottedInput::new(s("advancedglue"), vec![(0, 500)])],
                    max_sets: 1,
                },
            ],
        });
        factory.add_process(FluidSlottedConfig {
            name: s("HV-assembler-2"),
            input_slots: vec![vec![5, 6, 7]],
            input_tanks: vec![vec![0]],
            accesses: vec![InvTankAccess {
                client: s("main"),
                invs: vec![EachInvAccess { addr: s("7c4"), bus_side: SOUTH, inv_side: EAST }],
                tanks: vec![vec![EachBusOfTank { addr: s("7c4"), bus_side: NORTH, tank_side: EAST }]],
            }],
            to_extract: None,
            fluid_extract: None,
            strict_priority: false,
            recipes: vec![
                FluidSlottedRecipe {
                    outputs: Output::new(label("Tank"), 16),
                    inputs: vec![
                        MultiInvSlottedInput::new(label("Obsidian Glass"), vec![(0, 5, 1)]),
                        MultiInvSlottedInput::new(label("Iron Ring"), vec![(0, 6, 2)]),
                    ],
                    fluids: vec![],
                    max_sets: 8,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(label("LV Machine Hull"), 4),
                    inputs: vec![
                        MultiInvSlottedInput::new(label("LV Machine Casing"), vec![(0, 5, 1)]),
                        MultiInvSlottedInput::new(label("1x Tin Cable"), vec![(0, 6, 2)]),
                    ],
                    fluids: vec![FluidSlottedInput::new(s("molten.plastic"), vec![(0, 288)])],
                    max_sets: 4,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(label("Card Base"), 4),
                    inputs: vec![
                        MultiInvSlottedInput::new(label("Iron Rod"), vec![(0, 5, 1)]),
                        MultiInvSlottedInput::new(plastic_pcb(), vec![(0, 6, 1)]),
                        MultiInvSlottedInput::new(label("Gold Foil"), vec![(0, 7, 1)]),
                    ],
                    fluids: vec![],
                    max_sets: 4,
                },
            ],
        });
        factory.add_process(FluidSlottedConfig {
            name: s("EV-assembler-3"),
            input_slots: vec![vec![5, 6]],
            input_tanks: vec![vec![0]],
            accesses: vec![InvTankAccess {
                client: s("c2"),
                invs: vec![EachInvAccess { addr: s("f34"), bus_side: EAST, inv_side: NORTH }],
                tanks: vec![vec![EachBusOfTank { addr: s("f34"), bus_side: WEST, tank_side: NORTH }]],
            }],
            to_extract: None,
            fluid_extract: None,
            strict_priority: false,
            recipes: vec![
                FluidSlottedRecipe {
                    outputs: Output::new(label("SMD Resistor"), 16),
                    inputs: vec![
                        MultiInvSlottedInput::new(label("Carbon Dust"), vec![(0, 5, 1)]),
                        MultiInvSlottedInput::new(label("Fine Electrum Wire"), vec![(0, 6, 4)]),
                    ],
                    fluids: vec![FluidSlottedInput::new(s("molten.plastic"), vec![(0, 288)])],
                    max_sets: 8,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(label("SMD Capacitor"), 16),
                    inputs: vec![
                        MultiInvSlottedInput::new(label("Thin Polyvinyl Chloride Sheet"), vec![(0, 5, 4)]),
                        MultiInvSlottedInput::new(label("Aluminium Foil"), vec![(0, 6, 2)]),
                    ],
                    fluids: vec![FluidSlottedInput::new(s("molten.plastic"), vec![(0, 144)])],
                    max_sets: 8,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(label("SMD Transistor"), 16),
                    inputs: vec![
                        MultiInvSlottedInput::new(label("Gallium Foil"), vec![(0, 5, 1)]),
                        MultiInvSlottedInput::new(label("Fine Tantalum Wire"), vec![(0, 6, 8)]),
                    ],
                    fluids: vec![FluidSlottedInput::new(s("molten.plastic"), vec![(0, 288)])],
                    max_sets: 8,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(label("SMD Inductor"), 16),
                    inputs: vec![
                        MultiInvSlottedInput::new(label("Neodymium Ring"), vec![(0, 5, 1)]),
                        MultiInvSlottedInput::new(label("Fine Tantalum Wire"), vec![(0, 6, 8)]),
                    ],
                    fluids: vec![FluidSlottedInput::new(s("molten.plastic"), vec![(0, 36)])],
                    max_sets: 8,
                },
            ],
        });
        factory.add_process(SlottedConfig {
            name: s("MV-assembler-8"),
            accesses: vec![InvAccess { client: s("c1"), addr: s("fe0"), bus_side: NORTH, inv_side: UP }],
            input_slots: vec![5, 6],
            to_extract: None,
            strict_priority: false,
            recipes: ([("LV", "Steel"), ("EV", "Titanium")].into_iter())
                .map(|(v, i)| SlottedRecipe {
                    outputs: Output::new(label!("{v} Machine Casing"), 4),
                    inputs: vec![SlottedInput::new(label!("{i} Plate"), vec![(5, 8)])],
                    max_sets: 4,
                })
                .chain([SlottedRecipe {
                    outputs: Output::new(name("minecraft:stone_pressure_plate"), 16),
                    inputs: vec![SlottedInput::new(label("Stone Slab"), vec![(5, 2)]), SlottedInput::new(label("Iron Spring"), vec![(6, 1)])],
                    max_sets: 8,
                }])
                .collect(),
        });
        factory.add_process(FluidSlottedConfig {
            name: s("HV-assembler-24"),
            input_slots: vec![vec![5, 6]],
            input_tanks: vec![vec![0]],
            accesses: vec![InvTankAccess {
                client: s("main"),
                invs: vec![EachInvAccess { addr: s("7c4"), bus_side: SOUTH, inv_side: UP }],
                tanks: vec![vec![EachBusOfTank { addr: s("7c4"), bus_side: NORTH, tank_side: UP }]],
            }],
            to_extract: None,
            fluid_extract: None,
            strict_priority: false,
            recipes: ([(16, "Gold"), (16, "Tin")].into_iter())
                .map(|(q, x)| FluidSlottedRecipe {
                    outputs: Output::new(label!("1x {x} Cable"), q),
                    inputs: vec![MultiInvSlottedInput::new(label!("1x {x} Wire"), vec![(0, 5, 1)])],
                    fluids: vec![FluidSlottedInput::new(s("molten.silicone"), vec![(0, 72)])],
                    max_sets: 16,
                })
                .collect(),
        });
        factory.add_process(SlottedConfig {
            name: s("fluidExtractor-rotorSolidifier"),
            accesses: vec![InvAccess { client: s("c1"), addr: s("fe0"), bus_side: NORTH, inv_side: EAST }],
            input_slots: vec![5],
            to_extract: None,
            strict_priority: false,
            recipes: (["Aluminium", "Stainless Steel"].into_iter())
                .map(|x| SlottedRecipe {
                    outputs: Output::new(label!("{x} Rotor"), 4),
                    inputs: vec![SlottedInput::new(label!("{x} Ingot"), vec![(5, 17)])],
                    max_sets: 1,
                })
                .collect(),
        });
        factory.add_process(SlottedConfig {
            name: s("boltExtruder"),
            accesses: vec![InvAccess { client: s("c1"), addr: s("fe0"), bus_side: NORTH, inv_side: WEST }],
            input_slots: vec![5],
            to_extract: None,
            strict_priority: false,
            recipes: (["Silver", "Electrum"].into_iter())
                .map(|x| SlottedRecipe {
                    outputs: Output::new(label!("{x} Bolt"), 16),
                    inputs: vec![SlottedInput::new(label!("{x} Ingot"), vec![(5, 1)])],
                    max_sets: 4,
                })
                .collect(),
        });
        factory.add_process(SlottedConfig {
            name: s("ringExtruder"),
            accesses: vec![InvAccess { client: s("c3"), addr: s("189"), bus_side: EAST, inv_side: SOUTH }],
            input_slots: vec![5],
            to_extract: None,
            strict_priority: false,
            recipes: (["Iron"].into_iter())
                .map(|x| SlottedRecipe {
                    outputs: Output::new(label!("{x} Ring"), 16),
                    inputs: vec![SlottedInput::new(label!("{x} Ingot"), vec![(5, 1)])],
                    max_sets: 8,
                })
                .collect(),
        });
        factory.add_process(SlottedConfig {
            name: s("alloySmelter"),
            accesses: vec![InvAccess { client: s("c3"), addr: s("0fa"), bus_side: WEST, inv_side: SOUTH }],
            input_slots: vec![5, 6],
            to_extract: None,
            strict_priority: false,
            recipes: vec![SlottedRecipe {
                outputs: Output::new(label("Obsidian Glass"), 16),
                inputs: vec![SlottedInput::new(label("Thickened Glass"), vec![(5, 1)]), SlottedInput::new(label("Obsidian Dust"), vec![(6, 4)])],
                max_sets: 8,
            }],
        });
        factory.add_process(FluidSlottedConfig {
            name: s("autoclave"),
            input_slots: vec![vec![5, 6]],
            input_tanks: vec![vec![0]],
            accesses: vec![InvTankAccess {
                client: s("c2"),
                invs: vec![EachInvAccess { addr: s("f34"), bus_side: EAST, inv_side: SOUTH }],
                tanks: vec![vec![EachBusOfTank { addr: s("f34"), bus_side: WEST, tank_side: SOUTH }]],
            }],
            to_extract: None,
            fluid_extract: fluid_extract_slots(|_, slot| slot == 1),
            strict_priority: false,
            recipes: vec![
                FluidSlottedRecipe {
                    outputs: Output::new(label("Raw Carbon Fibre"), 16),
                    inputs: vec![MultiInvSlottedInput::new(label("Carbon Dust"), vec![(0, 5, 4)])],
                    fluids: vec![FluidSlottedInput::new(s("molten.epoxid"), vec![(0, 9)])],
                    max_sets: 64,
                },
                FluidSlottedRecipe {
                    outputs: FluidOutput::new(s("sodium tungstate"), 16_000),
                    inputs: vec![
                        MultiInvSlottedInput::new(label("Tungstate Dust"), vec![(0, 5, 7)]),
                        MultiInvSlottedInput::new(label("Sodium Dust"), vec![(0, 6, 2)]),
                    ],
                    fluids: vec![FluidSlottedInput::new(s("water"), vec![(0, 4_000)])],
                    max_sets: 32,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(label("Raw Lapotron Crystal"), 16),
                    inputs: vec![MultiInvSlottedInput::new(label("Lapotron Dust"), vec![(0, 5, 30)])],
                    fluids: vec![FluidSlottedInput::new(s("molten.energeticalloy"), vec![(0, 576)])],
                    max_sets: 2,
                },
            ],
        });
        factory.add_process(FluidSlottedConfig {
            name: s("arcFurnace"),
            input_slots: vec![vec![5]],
            input_tanks: vec![vec![0]],
            accesses: vec![InvTankAccess {
                client: s("c1"),
                invs: vec![EachInvAccess { addr: s("9f3"), bus_side: SOUTH, inv_side: UP }],
                tanks: vec![vec![EachBusOfTank { addr: s("9f3"), bus_side: NORTH, tank_side: UP }]],
            }],
            to_extract: None,
            fluid_extract: None,
            strict_priority: false,
            recipes: vec![FluidSlottedRecipe {
                outputs: Output::new(label("Glass"), 16),
                inputs: vec![MultiInvSlottedInput::new(label("Sand"), vec![(0, 5, 1)])],
                fluids: vec![FluidSlottedInput::new(s("oxygen"), vec![(0, 2)])],
                max_sets: 64,
            }],
        });
        factory.add_process(FluidSlottedConfig {
            name: s("cuttingMachine"),
            input_slots: vec![vec![5]],
            input_tanks: vec![vec![0]],
            accesses: vec![InvTankAccess {
                client: s("cr"),
                invs: vec![EachInvAccess { addr: s("36a"), bus_side: WEST, inv_side: UP }],
                tanks: vec![vec![EachBusOfTank { addr: s("36a"), bus_side: EAST, tank_side: UP }]],
            }],
            to_extract: None,
            fluid_extract: None,
            strict_priority: false,
            recipes: [
                (16, name("minecraft:stone_button"), name("minecraft:stone_pressure_plate"), 1, 8),
                (16, label("Stone Slab"), label("Stone"), 1, 8),
                (16, label("Obsidian Plate"), label("Obsidian"), 5, 8),
                (16, label("Aluminium Casing"), label("Aluminium Plate"), 1, 8),
                (16, label("Central Processing Unit"), label("Central Processing Unit (Wafer)"), 84, 4),
                (16, label("Nanocomponent Central Processing Unit"), label("NanoCPU Wafer"), 250, 4),
                (16, label("QBit Processing Unit"), label("QBit Wafer"), 250, 4),
                (16, label("Random Access Memory Chip"), label("Random Access Memory Chip (Wafer)"), 67, 4),
                (16, label("NAND Memory Chip"), label("NAND Memory Chip (Wafer)"), 135, 4),
                (16, label("Integrated Logic Circuit"), label("Integrated Logic Circuit (Wafer)"), 45, 4),
                (32, label("Phosphorus doped Wafer"), label("Phosphorus doped Monocrystalline Silicon Boule"), 75, 1),
            ]
            .into_iter()
            .map(|(q, o, i, l, max_sets)| FluidSlottedRecipe {
                outputs: Output::new(o, q),
                inputs: vec![MultiInvSlottedInput::new(i, vec![(0, 5, 1)])],
                fluids: vec![FluidSlottedInput::new(s("lubricant"), vec![(0, l)])],
                max_sets,
            })
            .collect(),
        });
        factory.add_process(FluidSlottedConfig {
            name: s("IV-circuitAssembler-CR"),
            input_slots: vec![vec![5, 6, 7, 8, 9, 10]],
            input_tanks: vec![vec![0]],
            accesses: vec![InvTankAccess {
                client: s("cr"),
                invs: vec![EachInvAccess { addr: s("36a"), bus_side: WEST, inv_side: NORTH }],
                tanks: vec![vec![EachBusOfTank { addr: s("36a"), bus_side: EAST, tank_side: NORTH }]],
            }],
            to_extract: None,
            fluid_extract: None,
            strict_priority: false,
            recipes: vec![
                FluidSlottedRecipe {
                    outputs: Output::new(label("Microprocessor"), 16),
                    inputs: vec![
                        MultiInvSlottedInput::new(plastic_pcb(), vec![(0, 5, 1)]),
                        MultiInvSlottedInput::new(label("Central Processing Unit"), vec![(0, 6, 1)]),
                        MultiInvSlottedInput::new(label("SMD Resistor"), vec![(0, 7, 2)]),
                        MultiInvSlottedInput::new(label("SMD Capacitor"), vec![(0, 8, 2)]),
                        MultiInvSlottedInput::new(label("SMD Transistor"), vec![(0, 9, 2)]),
                        MultiInvSlottedInput::new(label("Fine Copper Wire"), vec![(0, 10, 2)]),
                    ],
                    fluids: vec![FluidSlottedInput::new(s("molten.solderingalloy"), vec![(0, 72)])],
                    max_sets: 4,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(label("Integrated Processor"), 16),
                    inputs: vec![
                        MultiInvSlottedInput::new(plastic_pcb(), vec![(0, 5, 1)]),
                        MultiInvSlottedInput::new(label("Central Processing Unit"), vec![(0, 6, 1)]),
                        MultiInvSlottedInput::new(label("SMD Resistor"), vec![(0, 7, 4)]),
                        MultiInvSlottedInput::new(label("SMD Capacitor"), vec![(0, 8, 4)]),
                        MultiInvSlottedInput::new(label("SMD Transistor"), vec![(0, 9, 4)]),
                        MultiInvSlottedInput::new(label("Fine Red Alloy Wire"), vec![(0, 10, 4)]),
                    ],
                    fluids: vec![FluidSlottedInput::new(s("molten.solderingalloy"), vec![(0, 72)])],
                    max_sets: 4,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(label("Nanoprocessor"), 16),
                    inputs: vec![
                        MultiInvSlottedInput::new(label("Advanced Circuit Board"), vec![(0, 5, 1)]),
                        MultiInvSlottedInput::new(label("Nanocomponent Central Processing Unit"), vec![(0, 6, 1)]),
                        MultiInvSlottedInput::new(label("SMD Resistor"), vec![(0, 7, 8)]),
                        MultiInvSlottedInput::new(label("SMD Capacitor"), vec![(0, 8, 8)]),
                        MultiInvSlottedInput::new(label("SMD Transistor"), vec![(0, 9, 8)]),
                        MultiInvSlottedInput::new(label("Fine Electrum Wire"), vec![(0, 10, 8)]),
                    ],
                    fluids: vec![FluidSlottedInput::new(s("molten.solderingalloy"), vec![(0, 72)])],
                    max_sets: 4,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(label("Quantumprocessor"), 16),
                    inputs: vec![
                        MultiInvSlottedInput::new(label("More Advanced Circuit Board"), vec![(0, 5, 1)]),
                        MultiInvSlottedInput::new(label("QBit Processing Unit"), vec![(0, 6, 1)]),
                        MultiInvSlottedInput::new(label("Nanocomponent Central Processing Unit"), vec![(0, 7, 1)]),
                        MultiInvSlottedInput::new(label("SMD Capacitor"), vec![(0, 8, 12)]),
                        MultiInvSlottedInput::new(label("SMD Transistor"), vec![(0, 9, 12)]),
                        MultiInvSlottedInput::new(label("Fine Platinum Wire"), vec![(0, 10, 16)]),
                    ],
                    fluids: vec![FluidSlottedInput::new(s("molten.solderingalloy"), vec![(0, 72)])],
                    max_sets: 4,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(label("Quantumprocessor Assembly"), 16),
                    inputs: vec![
                        MultiInvSlottedInput::new(label("More Advanced Circuit Board"), vec![(0, 5, 1)]),
                        MultiInvSlottedInput::new(label("Quantumprocessor"), vec![(0, 6, 2)]),
                        MultiInvSlottedInput::new(label("SMD Inductor"), vec![(0, 7, 12)]),
                        MultiInvSlottedInput::new(label("SMD Capacitor"), vec![(0, 8, 16)]),
                        MultiInvSlottedInput::new(label("Random Access Memory Chip"), vec![(0, 9, 4)]),
                        MultiInvSlottedInput::new(label("Fine Platinum Wire"), vec![(0, 10, 24)]),
                    ],
                    fluids: vec![FluidSlottedInput::new(s("molten.solderingalloy"), vec![(0, 144)])],
                    max_sets: 4,
                },
            ],
        });
        factory.add_process(FluidSlottedConfig {
            name: s("HV-circuitAssembler-1"),
            input_slots: vec![vec![5, 6, 7, 8, 9, 10]],
            input_tanks: vec![vec![0]],
            accesses: vec![InvTankAccess {
                client: s("main"),
                invs: vec![EachInvAccess { addr: s("7c4"), bus_side: SOUTH, inv_side: WEST }],
                tanks: vec![vec![EachBusOfTank { addr: s("7c4"), bus_side: NORTH, tank_side: WEST }]],
            }],
            to_extract: None,
            fluid_extract: None,
            strict_priority: false,
            recipes: vec![
                FluidSlottedRecipe {
                    outputs: Output::new(label("Arithmetic Logic Unit (ALU)"), 16),
                    inputs: vec![
                        MultiInvSlottedInput::new(plastic_pcb(), vec![(0, 5, 1)]),
                        MultiInvSlottedInput::new(label("Integrated Logic Circuit"), vec![(0, 6, 16)]),
                    ],
                    fluids: vec![FluidSlottedInput::new(s("molten.solderingalloy"), vec![(0, 72)])],
                    max_sets: 4,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(label("Control Unit (CU)"), 4),
                    inputs: vec![
                        MultiInvSlottedInput::new(label("SMD Transistor"), vec![(0, 5, 4)]),
                        MultiInvSlottedInput::new(label("Nanoprocessor"), vec![(0, 6, 1)]),
                    ],
                    fluids: vec![FluidSlottedInput::new(s("molten.solderingalloy"), vec![(0, 72)])],
                    max_sets: 4,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(label("Microchip (Tier 1)"), 16),
                    inputs: vec![
                        MultiInvSlottedInput::new(plastic_pcb(), vec![(0, 5, 1)]),
                        MultiInvSlottedInput::new(label("Integrated Processor"), vec![(0, 6, 1)]),
                        MultiInvSlottedInput::new(label("SMD Transistor"), vec![(0, 7, 4)]),
                        MultiInvSlottedInput::new(label("Electrum Foil"), vec![(0, 8, 4)]),
                    ],
                    fluids: vec![FluidSlottedInput::new(s("molten.solderingalloy"), vec![(0, 72)])],
                    max_sets: 4,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(label("Microchip (Tier 3)"), 4),
                    inputs: vec![
                        MultiInvSlottedInput::new(label("More Advanced Circuit Board"), vec![(0, 5, 1)]),
                        MultiInvSlottedInput::new(label("Quantumprocessor"), vec![(0, 6, 1)]),
                        MultiInvSlottedInput::new(label("SMD Transistor"), vec![(0, 7, 16)]),
                        MultiInvSlottedInput::new(label("Electrum Foil"), vec![(0, 8, 16)]),
                    ],
                    fluids: vec![FluidSlottedInput::new(s("molten.solderingalloy"), vec![(0, 72)])],
                    max_sets: 4,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(label("EEPROM"), 4),
                    inputs: vec![
                        MultiInvSlottedInput::new(plastic_pcb(), vec![(0, 5, 1)]),
                        MultiInvSlottedInput::new(label("Microchip (Tier 1)"), vec![(0, 6, 1)]),
                        MultiInvSlottedInput::new(label("SMD Transistor"), vec![(0, 7, 6)]),
                        MultiInvSlottedInput::new(label("Gold Foil"), vec![(0, 8, 1)]),
                    ],
                    fluids: vec![FluidSlottedInput::new(s("molten.solderingalloy"), vec![(0, 72)])],
                    max_sets: 4,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(label("Network Card"), 4),
                    inputs: vec![
                        MultiInvSlottedInput::new(label("Card Base"), vec![(0, 5, 1)]),
                        MultiInvSlottedInput::new(label("Cable"), vec![(0, 6, 2)]),
                        MultiInvSlottedInput::new(label("Microchip (Tier 3)"), vec![(0, 7, 1)]),
                        MultiInvSlottedInput::new(label("Electrum Bolt"), vec![(0, 8, 8)]),
                    ],
                    fluids: vec![FluidSlottedInput::new(s("molten.solderingalloy"), vec![(0, 72)])],
                    max_sets: 4,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(label("Wireless Network Card (Tier 2)"), 4),
                    inputs: vec![
                        MultiInvSlottedInput::new(label("Network Card"), vec![(0, 5, 1)]),
                        MultiInvSlottedInput::new(label("Advanced Circuit Board"), vec![(0, 6, 1)]),
                        MultiInvSlottedInput::new(label("Microchip (Tier 3)"), vec![(0, 7, 1)]),
                        MultiInvSlottedInput::new(label("Emitter (HV)"), vec![(0, 8, 1)]),
                        MultiInvSlottedInput::new(label("Sensor (HV)"), vec![(0, 9, 1)]),
                    ],
                    fluids: vec![FluidSlottedInput::new(s("molten.solderingalloy"), vec![(0, 72)])],
                    max_sets: 4,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(label("Internet Card"), 4),
                    inputs: vec![
                        MultiInvSlottedInput::new(label("Wireless Network Card (Tier 2)"), vec![(0, 5, 1)]),
                        MultiInvSlottedInput::new(label("Interweb"), vec![(0, 6, 1)]),
                        MultiInvSlottedInput::new(label("Microchip (Tier 3)"), vec![(0, 7, 1)]),
                        MultiInvSlottedInput::new(label("Obsidian Plate"), vec![(0, 8, 1)]),
                        MultiInvSlottedInput::new(label("Red Illumar Button"), vec![(0, 9, 1)]),
                    ],
                    fluids: vec![FluidSlottedInput::new(s("molten.solderingalloy"), vec![(0, 144)])],
                    max_sets: 4,
                },
            ],
        });
        factory.add_process(FluidSlottedConfig {
            name: s("EV-circuitAssembler-2"),
            input_slots: vec![vec![5, 6, 7, 8, 9, 10]],
            input_tanks: vec![vec![0]],
            accesses: vec![InvTankAccess {
                client: s("main"),
                invs: vec![EachInvAccess { addr: s("039"), bus_side: SOUTH, inv_side: EAST }],
                tanks: vec![vec![EachBusOfTank { addr: s("039"), bus_side: NORTH, tank_side: EAST }]],
            }],
            to_extract: None,
            fluid_extract: None,
            strict_priority: false,
            recipes: vec![FluidSlottedRecipe {
                outputs: Output::new(label("Memory (Tier 3.5)"), 4),
                inputs: vec![
                    MultiInvSlottedInput::new(label("More Advanced Circuit Board"), vec![(0, 5, 1)]),
                    MultiInvSlottedInput::new(label("Random Access Memory Chip"), vec![(0, 6, 64)]),
                    MultiInvSlottedInput::new(label("NAND Memory Chip"), vec![(0, 7, 64)]),
                    MultiInvSlottedInput::new(label("Microchip (Tier 3)"), vec![(0, 8, 4)]),
                    MultiInvSlottedInput::new(label("Electrum Foil"), vec![(0, 9, 16)]),
                ],
                fluids: vec![FluidSlottedInput::new(s("molten.solderingalloy"), vec![(0, 72)])],
                max_sets: 1,
            }],
        });
        factory.add_process(FluidSlottedConfig {
            name: s("HV-circuitAssembler-3"),
            input_slots: vec![vec![5, 6, 7, 8, 9, 10]],
            input_tanks: vec![vec![0]],
            accesses: vec![InvTankAccess {
                client: s("c1"),
                invs: vec![EachInvAccess { addr: s("9f3"), bus_side: SOUTH, inv_side: WEST }],
                tanks: vec![vec![EachBusOfTank { addr: s("9f3"), bus_side: NORTH, tank_side: WEST }]],
            }],
            to_extract: None,
            fluid_extract: None,
            strict_priority: false,
            recipes: vec![FluidSlottedRecipe {
                outputs: Output::new(label("Central Processing Unit (CPU) (Tier 3)"), 4),
                inputs: vec![
                    MultiInvSlottedInput::new(label("Arithmetic Logic Unit (ALU)"), vec![(0, 5, 16)]),
                    MultiInvSlottedInput::new(label("Control Unit (CU)"), vec![(0, 6, 4)]),
                    MultiInvSlottedInput::new(label("Quantumprocessor"), vec![(0, 7, 1)]),
                    MultiInvSlottedInput::new(label("Aluminium Casing"), vec![(0, 8, 1)]),
                ],
                fluids: vec![FluidSlottedInput::new(s("molten.solderingalloy"), vec![(0, 144)])],
                max_sets: 4,
            }],
        });
        factory.add_process(FluidSlottedConfig {
            name: s("fluidExtractor"),
            input_slots: vec![vec![5]],
            input_tanks: vec![vec![]],
            accesses: vec![InvTankAccess {
                client: s("c3"),
                invs: vec![EachInvAccess { addr: s("5b8"), bus_side: EAST, inv_side: NORTH }],
                tanks: vec![vec![EachBusOfTank { addr: s("0c0"), bus_side: EAST, tank_side: SOUTH }]],
            }],
            to_extract: None,
            fluid_extract: fluid_extract_all(),
            strict_priority: false,
            recipes: [
                (16_000, "molten.glass", "Glass"),
                (256_000, "seedoil", "Sesame Seeds"),
                (16_000, "molten.glowstone", "Glowstone Dust"),
                (16_000, "molten.solderingalloy", "Soldering Alloy Dust"),
                (16_000, "molten.energeticalloy", "Energetic Alloy Ingot"),
            ]
            .into_iter()
            .map(|(q, o, i)| FluidSlottedRecipe {
                outputs: FluidOutput::new(s(o), q),
                inputs: vec![MultiInvSlottedInput::new(label(i), vec![(0, 5, 1)])],
                fluids: vec![],
                max_sets: 64,
            })
            .collect(),
        });
        factory.add_process(FluidSlottedConfig {
            name: s("centrifuge"),
            input_slots: vec![vec![0]],
            input_tanks: vec![vec![0]],
            accesses: vec![InvTankAccess {
                client: s("c3"),
                invs: vec![EachInvAccess { addr: s("5b8"), bus_side: EAST, inv_side: WEST }],
                tanks: vec![vec![EachBusOfTank { addr: s("0c0"), bus_side: EAST, tank_side: WEST }]],
            }],
            to_extract: None,
            fluid_extract: fluid_extract_all(),
            strict_priority: false,
            recipes: vec![
                FluidSlottedRecipe {
                    outputs: ignore_outputs(64.),
                    inputs: vec![MultiInvSlottedInput::new(custom("impureDust", |x| x.label.starts_with("Impure Pile of ")), vec![(0, 0, 1)])],
                    fluids: vec![],
                    max_sets: 64,
                },
                FluidSlottedRecipe {
                    outputs: ignore_outputs(64.),
                    inputs: vec![MultiInvSlottedInput::new(label("Ilmenite Slag Dust"), vec![(0, 0, 1)])],
                    fluids: vec![],
                    max_sets: 64,
                },
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
                    outputs: Output::new(label("Silicon Dioxide Dust"), 256),
                    inputs: vec![MultiInvSlottedInput::new(label("Glass Dust"), vec![(0, 0, 1)])],
                    fluids: vec![],
                    max_sets: 64,
                },
                FluidSlottedRecipe {
                    outputs: Output::new(label("Ruby Dust"), 16)
                        .and(Output::new(label("Pyrite Dust"), 16))
                        .and(FluidOutput::new(s("mercury"), 16_000)),
                    inputs: vec![MultiInvSlottedInput::new(label("Redstone"), vec![(0, 0, 10)])],
                    fluids: vec![],
                    max_sets: 64,
                },
                FluidSlottedRecipe {
                    outputs: FluidOutput::new(s("nitrogen"), 256_000),
                    inputs: vec![],
                    fluids: vec![FluidSlottedInput::new(s("air"), vec![(0, 10_000)])],
                    max_sets: 12,
                },
            ],
        });
        factory.add_process(SlottedConfig {
            name: s("compressor"),
            accesses: vec![InvAccess { client: s("c2"), addr: s("2ec"), bus_side: EAST, inv_side: NORTH }],
            input_slots: vec![5],
            to_extract: None,
            strict_priority: false,
            recipes: vec![
                SlottedRecipe {
                    outputs: Output::new(label("Plantball"), 16),
                    inputs: vec![SlottedInput::new(label("Sesame Seeds"), vec![(5, 8)])],
                    max_sets: 64,
                },
                SlottedRecipe {
                    outputs: Output::new(label("Compressed Air Cell"), 64),
                    inputs: vec![SlottedInput::new(label("Empty Cell"), vec![(5, 1)])],
                    max_sets: 64,
                },
            ],
        });
        factory.add_process(SlottedConfig {
            name: s("polarizer"),
            accesses: vec![InvAccess { client: s("c3"), addr: s("378"), bus_side: WEST, inv_side: NORTH }],
            input_slots: vec![5],
            to_extract: None,
            strict_priority: false,
            recipes: vec![SlottedRecipe {
                outputs: Output::new(label("Magnetic Iron Rod"), 16),
                inputs: vec![SlottedInput::new(label("Iron Rod"), vec![(5, 1)])],
                max_sets: 8,
            }],
        });
        factory.add_process(SlottedConfig {
            name: s("rodExtruder"),
            accesses: vec![InvAccess { client: s("c3"), addr: s("378"), bus_side: WEST, inv_side: EAST }],
            input_slots: vec![5],
            to_extract: None,
            strict_priority: false,
            recipes: ["Iron", "Chrome"]
                .into_iter()
                .map(|x| SlottedRecipe {
                    outputs: Output::new(label!("{x} Rod"), 16),
                    inputs: vec![SlottedInput::new(label!("{x} Ingot"), vec![(5, 1)])],
                    max_sets: 8,
                })
                .collect(),
        });
        factory.add_process(SlottedConfig {
            name: s("extractor"),
            accesses: vec![InvAccess { client: s("c3"), addr: s("5b8"), bus_side: EAST, inv_side: SOUTH }],
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
            name: s("thermalCentrifuge"),
            accesses: vec![InvAccess { client: s("c3"), addr: s("189"), bus_side: EAST, inv_side: NORTH }],
            input_slots: vec![5],
            to_extract: None,
            strict_priority: false,
            recipes: (["Silver", "Rock Salt", "Salt"].into_iter())
                .map(|x| SlottedRecipe {
                    outputs: Output::new(label!("Centrifuged {x} Ore"), 16),
                    inputs: vec![SlottedInput::new(label!("Purified {x} Ore"), vec![(5, 1)])],
                    max_sets: 16,
                })
                .collect(),
        });
        factory.add_process(SlottedConfig {
            name: s("furnace"),
            accesses: vec![InvAccess { client: s("c3"), addr: s("189"), bus_side: EAST, inv_side: UP }],
            input_slots: vec![5],
            to_extract: None,
            strict_priority: false,
            recipes: (["Tin", "Gold", "Silver", "Gallium", "Electrum", "Red Alloy", "Plutonium 239"].into_iter())
                .map(|x| SlottedRecipe {
                    outputs: Output::new(label!("{x} Ingot"), 16),
                    inputs: vec![SlottedInput::new(label!("{x} Dust"), vec![(5, 1)])],
                    max_sets: 16,
                })
                .chain([SlottedRecipe {
                    outputs: Output::new(label("Stone"), 16),
                    inputs: vec![SlottedInput::new(label("Cobblestone"), vec![(5, 1)])],
                    max_sets: 16,
                }])
                .collect(),
        });
        for (color, out, access) in [
            ("white", "Central Processing Unit (Wafer)", InvAccess { client: s("cr"), addr: s("b20"), bus_side: EAST, inv_side: WEST }),
            ("cyan", "Random Access Memory Chip (Wafer)", InvAccess { client: s("cr"), addr: s("b20"), bus_side: EAST, inv_side: NORTH }),
            ("ender", "NAND Memory Chip (Wafer)", InvAccess { client: s("cr"), addr: s("b20"), bus_side: EAST, inv_side: SOUTH }),
            ("red", "Integrated Logic Circuit (Wafer)", InvAccess { client: s("cr"), addr: s("36a"), bus_side: WEST, inv_side: SOUTH }),
        ] {
            factory.add_process(SlottedConfig {
                name: local_fmt!("{color}Engraver"),
                accesses: vec![access],
                input_slots: vec![5],
                to_extract: None,
                strict_priority: false,
                recipes: vec![SlottedRecipe {
                    outputs: Output::new(label(out), 4),
                    inputs: vec![SlottedInput::new(label("Phosphorus doped Wafer"), vec![(5, 1)])],
                    max_sets: 1,
                }],
            });
        }
        factory.add_process(SlottedConfig {
            name: s("rockBreaker-1"),
            accesses: vec![InvAccess { client: s("c2"), addr: s("2ec"), bus_side: EAST, inv_side: SOUTH }],
            input_slots: vec![5],
            to_extract: None,
            strict_priority: false,
            recipes: vec![SlottedRecipe {
                outputs: Output::new(label("Obsidian"), 16),
                inputs: vec![SlottedInput::new(label("Redstone"), vec![(5, 1)])],
                max_sets: 16,
            }],
        });
        factory.add_process(SlottedConfig {
            name: s("bender-1"),
            accesses: vec![InvAccess { client: s("c2"), addr: s("2ec"), bus_side: EAST, inv_side: UP }],
            input_slots: vec![5],
            to_extract: None,
            strict_priority: false,
            recipes: (["Steel", "Aluminium", "Titanium", "Stainless Steel"].into_iter())
                .map(|x| SlottedRecipe {
                    outputs: Output::new(label!("{x} Plate"), 16),
                    inputs: vec![SlottedInput::new(label!("{x} Ingot"), vec![(5, 1)])],
                    max_sets: 8,
                })
                .chain([
                    SlottedRecipe {
                        outputs: Output::new(label("Thin Polyvinyl Chloride Sheet"), 16),
                        inputs: vec![SlottedInput::new(label("Polyvinyl Chloride Sheet"), vec![(5, 1)])],
                        max_sets: 8,
                    },
                    SlottedRecipe {
                        outputs: Output::new(label("Iron Spring"), 16),
                        inputs: vec![SlottedInput::new(label("Long Iron Rod"), vec![(5, 1)])],
                        max_sets: 8,
                    },
                ])
                .collect(),
        });
        factory.add_process(SlottedConfig {
            name: s("bender-10"),
            accesses: vec![InvAccess { client: s("c2"), addr: s("2ec"), bus_side: EAST, inv_side: WEST }],
            input_slots: vec![5],
            to_extract: None,
            strict_priority: false,
            recipes: ["Gold", "Copper", "Gallium", "Electrum", "Aluminium", "Energetic Alloy"]
                .into_iter()
                .map(|x| SlottedRecipe {
                    outputs: Output::new(label!("{x} Foil"), 16),
                    inputs: vec![SlottedInput::new(label!("{x} Ingot"), vec![(5, 1)])],
                    max_sets: 8,
                })
                .collect(),
        });
        factory.add_process(SlottedConfig {
            name: s("wiremill-1"),
            accesses: vec![InvAccess { client: s("c1"), addr: s("fe0"), bus_side: NORTH, inv_side: SOUTH }],
            input_slots: vec![5],
            to_extract: None,
            strict_priority: false,
            recipes: ([(16, "Tin"), (16, "Gold"), (16, "Copper"), (16, "HSS-G")].into_iter())
                .map(|(q, x)| SlottedRecipe {
                    outputs: Output::new(label!("1x {x} Wire"), q),
                    inputs: vec![SlottedInput::new(label!("{x} Ingot"), vec![(5, 1)])],
                    max_sets: 8,
                })
                .collect(),
        });
        factory.add_process(SlottedConfig {
            name: s("wiremill-3"),
            accesses: vec![InvAccess { client: s("c3"), addr: s("189"), bus_side: EAST, inv_side: WEST }],
            input_slots: vec![5],
            to_extract: None,
            strict_priority: false,
            recipes: ([(16, "Red Alloy"), (16, "Electrum"), (16, "Tantalum"), (24, "Platinum"), (16, "Copper")].into_iter())
                .map(|(q, x)| SlottedRecipe {
                    outputs: Output::new(label!("Fine {x} Wire"), q),
                    inputs: vec![SlottedInput::new(label!("{x} Ingot"), vec![(5, 1)])],
                    max_sets: 8,
                })
                .collect(),
        });
        factory.add_process(SlottedConfig {
            name: s("forgeHammer"),
            accesses: vec![InvAccess { client: s("main"), addr: s("09c"), bus_side: NORTH, inv_side: UP }],
            input_slots: vec![5],
            to_extract: None,
            strict_priority: false,
            recipes: vec![
                SlottedRecipe {
                    outputs: Output::new(label("Sand"), 64),
                    inputs: vec![SlottedInput::new(label("Gravel"), vec![(5, 1)])],
                    max_sets: 64,
                },
                SlottedRecipe {
                    outputs: Output::new(label("Gravel"), 64),
                    inputs: vec![SlottedInput::new(label("Cobblestone"), vec![(5, 1)])],
                    max_sets: 64,
                },
                SlottedRecipe {
                    outputs: Output::new(label("Long Iron Rod"), 16),
                    inputs: vec![SlottedInput::new(label("Iron Rod"), vec![(5, 2)])],
                    max_sets: 8,
                },
                SlottedRecipe {
                    outputs: Output::new(label("Small Gallium Arsenide Crystal"), 2),
                    inputs: vec![SlottedInput::new(label("Gallium Arsenide Crystal"), vec![(5, 1)])],
                    max_sets: 1,
                },
            ],
        });
        factory.add_process(SlottedConfig {
            name: s("macerator"),
            accesses: vec![InvAccess { client: s("c3"), addr: s("5b8"), bus_side: EAST, inv_side: UP }],
            input_slots: vec![5],
            to_extract: None,
            strict_priority: false,
            recipes: [
                both("Salt", "bartworks:gt.bwMetaGeneratedgem"),
                label("Exquisite Salt"),
                label("Flawless Salt"),
                label("Chipped Salt"),
                label("Flawed Salt"),
                both("Rock Salt", "bartworks:gt.bwMetaGeneratedgem"),
                label("Exquisite Rock Salt"),
                label("Flawless Rock Salt"),
                label("Chipped Rock Salt"),
                label("Flawed Rock Salt"),
            ]
            .into_iter()
            .map(|x| SlottedRecipe { outputs: ignore_outputs(64.), inputs: vec![SlottedInput::new(x, vec![(5, 1)])], max_sets: 64 })
            .chain((["Ilmenite", "Galena", "Silver", "Sphalerite", "Salt", "Rock Salt"].into_iter()).map(|x| SlottedRecipe {
                outputs: Output::new(label!("Crushed {x} Ore"), 16),
                inputs: vec![SlottedInput::new(label!("{x} Ore"), vec![(5, 1)])],
                max_sets: 8,
            }))
            .chain([
                SlottedRecipe {
                    outputs: ignore_outputs(64.),
                    inputs: vec![SlottedInput::new(label("Block of Redstone"), vec![(5, 1)])],
                    max_sets: 64,
                },
                SlottedRecipe {
                    outputs: Output::new(label("Charcoal Dust"), 16),
                    inputs: vec![SlottedInput::new(label("Charcoal"), vec![(5, 1)])],
                    max_sets: 64,
                },
                SlottedRecipe {
                    outputs: Output::new(label("Glass Dust"), 64),
                    inputs: vec![SlottedInput::new(label("Glass"), vec![(5, 1)])],
                    max_sets: 64,
                },
                SlottedRecipe {
                    outputs: Output::new(both("Salt", "gregtech:gt.metaitem.01"), 64),
                    inputs: vec![SlottedInput::new(label("Centrifuged Salt Ore"), vec![(5, 1)])],
                    max_sets: 16,
                },
                SlottedRecipe {
                    outputs: Output::new(both("Rock Salt", "gregtech:gt.metaitem.01"), 64),
                    inputs: vec![SlottedInput::new(label("Centrifuged Rock Salt Ore"), vec![(5, 1)])],
                    max_sets: 16,
                },
                SlottedRecipe {
                    outputs: Output::new(label("Plant Mass"), 16),
                    inputs: vec![SlottedInput::new(label("Plantball"), vec![(5, 2)])],
                    max_sets: 16,
                },
                SlottedRecipe {
                    outputs: Output::new(label("Silver Dust"), 16),
                    inputs: vec![SlottedInput::new(label("Centrifuged Silver Ore"), vec![(5, 1)])],
                    max_sets: 8,
                },
                SlottedRecipe {
                    outputs: Output::new(label("Obsidian Dust"), 16),
                    inputs: vec![SlottedInput::new(label("Obsidian"), vec![(5, 1)])],
                    max_sets: 8,
                },
                SlottedRecipe {
                    outputs: Output::new(label("Iron Dust"), 16),
                    inputs: vec![SlottedInput::new(label("Iron Ingot"), vec![(5, 1)])],
                    max_sets: 8,
                },
                SlottedRecipe {
                    outputs: Output::new(label("Steel Dust"), 16),
                    inputs: vec![SlottedInput::new(label("Steel Ingot"), vec![(5, 1)])],
                    max_sets: 8,
                },
                SlottedRecipe {
                    outputs: Output::new(label("Copper Dust"), 16),
                    inputs: vec![SlottedInput::new(label("Copper Ingot"), vec![(5, 1)])],
                    max_sets: 8,
                },
            ])
            .collect(),
        });
        factory.add_process(BlockingOutputConfig {
            accesses: vec![InvAccess { client: s("c3"), addr: s("378"), bus_side: WEST, inv_side: UP }],
            slot_filter: None,
            outputs: vec![Output { item: label("Cobblestone"), n_wanted: 64 }],
        });
        factory.add_process(BlockingFluidOutputConfig {
            accesses: vec![TankAccess { client: s("main"), buses: vec![EachBusOfTank { addr: s("608"), bus_side: SOUTH, tank_side: WEST }] }],
            outputs: vec![FluidOutput { fluid: s("water"), n_wanted: 128_000 }],
        })
    })
}
