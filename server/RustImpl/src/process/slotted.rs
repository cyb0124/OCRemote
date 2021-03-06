use super::super::access::InvAccess;
use super::super::action::{ActionFuture, Call};
use super::super::factory::Factory;
use super::super::item::ItemStack;
use super::super::recipe::{compute_demands, Demand, Output, Recipe, SlottedInput};
use super::super::util::{alive, join_outputs, join_tasks, spawn, AbortOnDrop};
use super::{extract_output, list_inv, ExtractFilter, IntoProcess, InvProcess, Process};
use fnv::{FnvHashMap, FnvHashSet};
use std::{
    cell::RefCell,
    cmp::min,
    rc::{Rc, Weak},
};

pub struct SlottedRecipe {
    pub outputs: Vec<Output>,
    pub inputs: Vec<SlottedInput>,
    pub max_per_slot: i32,
}

impl_recipe!(SlottedRecipe, SlottedInput);

pub struct SlottedConfig {
    pub name: &'static str,
    pub accesses: Vec<InvAccess>,
    pub input_slots: Vec<usize>,
    pub to_extract: Option<ExtractFilter>,
    pub recipes: Vec<SlottedRecipe>,
}

pub struct SlottedProcess {
    weak: Weak<RefCell<SlottedProcess>>,
    config: SlottedConfig,
    factory: Weak<RefCell<Factory>>,
}

impl_inv_process!(SlottedProcess);

impl IntoProcess for SlottedConfig {
    type Output = SlottedProcess;
    fn into_process(self, factory: &Weak<RefCell<Factory>>) -> Rc<RefCell<Self::Output>> {
        Rc::new_cyclic(|weak| RefCell::new(Self::Output { weak: weak.clone(), config: self, factory: factory.clone() }))
    }
}

impl Process for SlottedProcess {
    fn run(&self, factory: &Factory) -> AbortOnDrop<Result<(), String>> {
        if self.config.to_extract.is_none() && compute_demands(factory, &self.config.recipes).is_empty() {
            return spawn(async { Ok(()) });
        }
        let stacks = list_inv(self, factory);
        let weak = self.weak.clone();
        spawn(async move {
            let stacks = stacks.await?;
            let mut tasks = Vec::new();
            {
                alive!(weak, this);
                upgrade_mut!(this.factory, factory);
                let mut existing_inputs = FnvHashMap::<usize, Option<ItemStack>>::default();
                for slot in &this.config.input_slots {
                    existing_inputs.insert(*slot, None);
                }
                for (slot, stack) in stacks.into_iter().enumerate() {
                    if let Some(stack) = stack {
                        if let Some(existing_input) = existing_inputs.get_mut(&slot) {
                            *existing_input = Some(stack)
                        } else if let Some(ref to_extract) = this.config.to_extract {
                            if to_extract(slot, &stack) {
                                tasks.push(extract_output(this, factory, slot, stack.item.max_size))
                            }
                        }
                    }
                }
                let demands = compute_demands(factory, &this.config.recipes);
                'recipe: for mut demand in demands.into_iter() {
                    let recipe = &this.config.recipes[demand.i_recipe];
                    let mut used_slots = FnvHashSet::<usize>::default();
                    for (i_input, input) in recipe.inputs.iter().enumerate() {
                        let size_per_slot = input.size / input.slots.len() as i32;
                        for slot in &input.slots {
                            let existing_input = existing_inputs.get(&slot).unwrap();
                            let existing_size = if let Some(existing_input) = existing_input {
                                if existing_input.item != demand.inputs.items[i_input] {
                                    continue 'recipe;
                                }
                                existing_input.size
                            } else {
                                0
                            };
                            demand.inputs.n_sets = min(
                                demand.inputs.n_sets,
                                (min(recipe.max_per_slot, demand.inputs.items[i_input].max_size) - existing_size)
                                    / size_per_slot,
                            );
                            if demand.inputs.n_sets <= 0 {
                                continue 'recipe;
                            }
                            used_slots.insert(*slot);
                        }
                    }
                    for (slot, existing_input) in &existing_inputs {
                        if existing_input.is_some() && !used_slots.contains(slot) {
                            continue 'recipe;
                        }
                    }
                    tasks.push(this.execute_recipe(factory, demand));
                    break;
                }
            }
            join_tasks(tasks).await
        })
    }
}

impl SlottedProcess {
    fn execute_recipe(&self, factory: &mut Factory, demand: Demand) -> AbortOnDrop<Result<(), String>> {
        let mut bus_slots = Vec::new();
        let slots_to_free = Rc::new(RefCell::new(Vec::new()));
        let recipe = &self.config.recipes[demand.i_recipe];
        for (i_input, input) in recipe.inputs.iter().enumerate() {
            let reservation = factory.reserve_item(
                self.config.name,
                &demand.inputs.items[i_input],
                demand.inputs.n_sets * input.size,
            );
            let bus_slot = factory.bus_allocate();
            let slots_to_free = slots_to_free.clone();
            let weak = self.factory.clone();
            bus_slots.push(spawn(async move {
                let bus_slot = bus_slot.await?;
                slots_to_free.borrow_mut().push(bus_slot);
                let extraction = reservation.extract(&*alive(&weak)?.borrow(), bus_slot);
                extraction.await.map(|_| bus_slot)
            }))
        }
        let weak = self.weak.clone();
        spawn(async move {
            let bus_slots = join_outputs(bus_slots).await;
            let slots_to_free = Rc::try_unwrap(slots_to_free)
                .map_err(|_| "slots_to_free should be exclusively owned here")
                .unwrap()
                .into_inner();
            let task = async {
                let bus_slots = bus_slots?;
                let mut tasks = Vec::new();
                {
                    alive!(weak, this);
                    upgrade!(this.factory, factory);
                    let server = factory.borrow_server();
                    let access = server.load_balance(&this.config.accesses).1;
                    let mut group = Vec::new();
                    let recipe = &this.config.recipes[demand.i_recipe];
                    for (i_input, input) in recipe.inputs.iter().enumerate() {
                        let size_per_slot = input.size / input.slots.len() as i32;
                        for inv_slot in &input.slots {
                            let action = ActionFuture::from(Call {
                                addr: access.addr,
                                func: "transferItem",
                                args: vec![
                                    access.bus_side.into(),
                                    access.inv_side.into(),
                                    (demand.inputs.n_sets * size_per_slot).into(),
                                    (bus_slots[i_input] + 1).into(),
                                    (inv_slot + 1).into(),
                                ],
                            });
                            group.push(action.clone().into());
                            tasks.push(spawn(async move { action.await.map(|_| ()) }));
                        }
                    }
                    server.enqueue_request_group(access.client, group)
                }
                join_tasks(tasks).await?;
                alive!(weak, this);
                upgrade_mut!(this.factory, factory);
                for slot_to_free in &slots_to_free {
                    factory.bus_free(*slot_to_free)
                }
                Ok(())
            };
            let result = task.await;
            if result.is_err() {
                alive!(weak, this);
                upgrade_mut!(this.factory, factory);
                factory.bus_deposit(slots_to_free);
            }
            result
        })
    }
}
