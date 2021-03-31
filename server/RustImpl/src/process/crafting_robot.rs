use super::super::access::CraftingRobotAccess;
use super::super::action::{ActionFuture, Call};
use super::super::factory::Factory;
use super::super::recipe::{compute_demands, resolve_inputs, CraftingGridRecipe, Demand, ResolvedInputs};
use super::super::util::{alive, join_outputs, join_tasks, spawn, AbortOnDrop};
use super::{IntoProcess, Process};
use std::{
    cell::RefCell,
    cmp::min,
    rc::{Rc, Weak},
};

pub struct CraftingRobotConfig {
    pub name: &'static str,
    pub accesses: Vec<CraftingRobotAccess>,
    pub recipes: Vec<CraftingGridRecipe>,
}

struct CraftingRobotProcess {
    weak: Weak<RefCell<CraftingRobotProcess>>,
    config: CraftingRobotConfig,
    factory: Weak<RefCell<Factory>>,
}

impl IntoProcess for CraftingRobotConfig {
    fn into_process(self, factory: Weak<RefCell<Factory>>) -> Rc<RefCell<dyn Process>> {
        Rc::new_cyclic(|weak| RefCell::new(CraftingRobotProcess { weak: weak.clone(), config: self, factory }))
    }
}

fn map_crafting_grid(slot: usize) -> usize {
    if slot >= 6 {
        slot + 2
    } else if slot >= 3 {
        slot + 1
    } else {
        slot
    }
}

impl Process for CraftingRobotProcess {
    fn run(&self, factory: &Factory) -> AbortOnDrop<Result<(), String>> {
        let mut tasks = Vec::new();
        for Demand { i_recipe, .. } in compute_demands(factory, &self.config.recipes) {
            let recipe = &self.config.recipes[i_recipe];
            if recipe.max_sets <= 0 {
                continue;
            }
            if let Some(ResolvedInputs { mut n_sets, items }) = resolve_inputs(factory, recipe) {
                n_sets = min(n_sets, recipe.max_sets);
                let mut bus_slots = Vec::new();
                let slots_to_free = Rc::new(RefCell::new(Vec::new()));
                for (i_input, item) in items.into_iter().enumerate() {
                    let reservation =
                        factory.reserve_item(self.config.name, &item, n_sets * recipe.inputs[i_input].size);
                    let slots_to_free = slots_to_free.clone();
                    let weak = self.factory.clone();
                    bus_slots.push(spawn(async move {
                        let bus_slot = alive(&weak)?.borrow_mut().bus_allocate();
                        let bus_slot = bus_slot.await?;
                        slots_to_free.borrow_mut().push(bus_slot);
                        let extraction = reservation.extract(&*alive(&weak)?.borrow(), bus_slot);
                        extraction.await.map(|_| bus_slot)
                    }))
                }
                if bus_slots.is_empty() {
                    let slots_to_free = slots_to_free.clone();
                    let weak = self.factory.clone();
                    bus_slots.push(spawn(async move {
                        let bus_slot = alive(&weak)?.borrow_mut().bus_allocate();
                        let bus_slot = bus_slot.await?;
                        slots_to_free.borrow_mut().push(bus_slot);
                        Ok(bus_slot)
                    }))
                }
                let weak = self.weak.clone();
                tasks.push(spawn(async move {
                    let bus_slots = join_outputs(bus_slots).await;
                    let mut slots_to_free = Rc::try_unwrap(slots_to_free)
                        .map_err(|_| "slots_to_free should be exclusively owned here")
                        .unwrap()
                        .into_inner();
                    let task = async {
                        let bus_slots = bus_slots?;
                        let tasks = {
                            alive!(weak, this);
                            upgrade!(this.factory, factory);
                            let server = factory.borrow_server();
                            let access = server.load_balance(&this.config.accesses).1;
                            let mut group = Vec::new();
                            let recipe = &this.config.recipes[i_recipe];
                            for (i_input, input) in recipe.inputs.iter().enumerate() {
                                let size_per_slot = input.size / input.slots.len() as i32;
                                for inv_slot in &input.slots {
                                    // select input
                                    group.push(Call {
                                        addr: "robot",
                                        func: "select",
                                        args: vec![(map_crafting_grid(*inv_slot) + 1).into()],
                                    });
                                    // transfer input
                                    group.push(Call {
                                        addr: "inventory_controller",
                                        func: "suckFromSlot",
                                        args: vec![
                                            access.bus_side.into(),
                                            (bus_slots[i_input] + 1).into(),
                                            (size_per_slot * n_sets).into(),
                                        ],
                                    })
                                }
                            }
                            for non_consumable in &recipe.non_consumables {
                                // select non_consumable
                                group.push(Call {
                                    addr: "robot",
                                    func: "select",
                                    args: vec![(non_consumable.storage_slot + 1).into()],
                                });
                                // load non_consumable
                                group.push(Call {
                                    addr: "robot",
                                    func: "transferTo",
                                    args: vec![(map_crafting_grid(non_consumable.crafting_grid_slot) + 1).into()],
                                })
                            }
                            // select output
                            group.push(Call { addr: "robot", func: "select", args: vec![16.into()] });
                            // craft
                            group.push(Call { addr: "crafting", func: "craft", args: vec![] });
                            // transfer output
                            group.push(Call {
                                addr: "inventory_controller",
                                func: "dropIntoSlot",
                                args: vec![access.bus_side.into(), (slots_to_free[0] + 1).into()],
                            });
                            for non_consumable in &recipe.non_consumables {
                                // select non_consumable
                                group.push(Call {
                                    addr: "robot",
                                    func: "select",
                                    args: vec![(map_crafting_grid(non_consumable.crafting_grid_slot) + 1).into()],
                                });
                                // store non_consumable
                                group.push(Call {
                                    addr: "robot",
                                    func: "transferTo",
                                    args: vec![(non_consumable.storage_slot + 1).into()],
                                });
                            }
                            let group: Vec<_> = group.into_iter().map(|x| ActionFuture::from(x)).collect();
                            server
                                .enqueue_request_group(access.client, group.iter().map(|x| x.clone().into()).collect());
                            group.into_iter().map(|x| spawn(async move { x.await.map(|_| ()) })).collect()
                        };
                        join_tasks(tasks).await?;
                        alive!(weak, this);
                        upgrade_mut!(this.factory, factory);
                        while slots_to_free.len() > 1 {
                            factory.bus_free(slots_to_free.pop().unwrap())
                        }
                        Ok(())
                    };
                    let result = task.await;
                    alive!(weak, this);
                    upgrade_mut!(this.factory, factory);
                    factory.bus_deposit(slots_to_free);
                    result
                }))
            }
        }
        spawn(async move { join_tasks(tasks).await })
    }
}
