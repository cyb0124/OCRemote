use super::super::access::InvAccess;
use super::super::action::{ActionFuture, Call, List};
use super::super::factory::{Factory, Reservation};
use super::super::item::{Filter, ItemStack};
use super::super::recipe::{compute_demands, resolve_inputs, Demand, Input, Output, Recipe};
use super::super::util::{alive, join_tasks, spawn, AbortOnDrop};
use super::{extract_output, ExtractFilter, ExtractableProcess, IntoProcess, Process};
use fnv::FnvHashMap;
use std::{
    cell::RefCell,
    cmp::min,
    iter::once,
    rc::{Rc, Weak},
};

pub struct ScatteringInput {
    item: Filter,
    size: i32,
    allow_backup: bool,
    extra_backup: i32,
}

impl ScatteringInput {
    pub fn new(item: Filter) -> Self {
        ScatteringInput {
            item,
            size: 1,
            allow_backup: false,
            extra_backup: 0,
        }
    }
}

impl_input!(ScatteringInput);

pub struct ScatteringRecipe {
    outputs: Vec<Output>,
    inputs: Vec<ScatteringInput>,
}

impl ScatteringRecipe {
    pub fn new(outputs: Vec<Output>, input: ScatteringInput) -> Self {
        ScatteringRecipe {
            outputs,
            inputs: vec![input],
        }
    }
}

impl_recipe!(ScatteringRecipe, ScatteringInput);

pub struct ScatteringConfig {
    pub name: &'static str,
    pub accesses: Vec<InvAccess>,
    // plant_sower: 6, 7, .., 14
    pub input_slots: Vec<usize>,
    pub to_extract: Option<ExtractFilter>,
    pub recipes: Vec<ScatteringRecipe>,
    pub max_per_slot: i32,
}

struct ScatteringProcess {
    weak: Weak<RefCell<ScatteringProcess>>,
    config: ScatteringConfig,
    factory: Weak<RefCell<Factory>>,
}

impl_extractable_process!(ScatteringProcess);

impl IntoProcess for ScatteringConfig {
    fn into_process(self, factory: Weak<RefCell<Factory>>) -> Rc<RefCell<dyn Process>> {
        Rc::new_cyclic(|weak| {
            RefCell::new(ScatteringProcess {
                weak: weak.clone(),
                config: self,
                factory,
            })
        })
    }
}

impl Process for ScatteringProcess {
    fn run(&self, factory: &Factory) -> AbortOnDrop<Result<(), String>> {
        if self.config.to_extract.is_none() && compute_demands(factory, &self.config.recipes).is_empty() {
            return spawn(async { Ok(()) });
        }
        let server = factory.borrow_server();
        let access = server.load_balance(&self.config.accesses).1;
        let action = ActionFuture::from(List {
            addr: access.addr,
            side: access.inv_side,
        });
        server.enqueue_request_group(access.client, vec![action.clone().into()]);
        let weak = self.weak.clone();
        spawn(async move {
            let mut stacks = action.await?;
            let mut tasks = Vec::new();
            {
                alive!(weak, this);
                upgrade_mut!(this.factory, factory);
                let mut is_input_slot = vec![false; stacks.len()];
                for slot in &this.config.input_slots {
                    if *slot >= stacks.len() {
                        return Err(format!("{}: invalid slot", this.config.name));
                    }
                    is_input_slot[*slot] = true
                }
                if let Some(ref to_extract) = this.config.to_extract {
                    for (slot, stack) in stacks.iter().enumerate() {
                        if let Some(stack) = stack {
                            if !is_input_slot[slot] && to_extract(slot, stack) {
                                tasks.push(extract_output(this, factory, slot, stack.item.max_size))
                            }
                        }
                    }
                }
                for Demand { i_recipe, .. } in compute_demands(factory, &this.config.recipes) {
                    if let Some(mut inputs) = resolve_inputs(factory, &this.config.recipes[i_recipe]) {
                        let mut insertions = FnvHashMap::<usize, i32>::default();
                        let mut n_inserted = 0;
                        let mut is_full = false;
                        while inputs.n_sets > 0 {
                            let mut min_any = 0;
                            let mut min_recipe = None;
                            for slot in &this.config.input_slots {
                                if let Some(ref stack) = stacks[*slot] {
                                    min_any = min(min_any, stack.size);
                                    if stack.item == inputs.items[0] {
                                        if let Some((_, min_recipe)) = min_recipe {
                                            if stack.size >= min_recipe {
                                                continue;
                                            }
                                        }
                                        min_recipe = Some((*slot, stack.size))
                                    }
                                } else {
                                    min_recipe = Some((*slot, 0))
                                }
                            }
                            if min_any >= this.config.max_per_slot {
                                is_full = true;
                                break;
                            }
                            if let Some((slot, min_recipe)) = min_recipe {
                                if min_recipe >= inputs.items[0].max_size {
                                    break;
                                }
                                inputs.n_sets -= 1;
                                n_inserted += 1;
                                *insertions.entry(slot).or_default() += 1;
                                let stack = &mut stacks[slot];
                                if let Some(ref mut stack) = stack {
                                    stack.size += 1
                                } else {
                                    *stack = Some(ItemStack {
                                        item: inputs.items[0].clone(),
                                        size: 1,
                                    })
                                }
                            } else {
                                break;
                            }
                        }
                        if n_inserted > 0 {
                            let reservation = factory.reserve_item(this.config.name, &inputs.items[0], n_inserted);
                            tasks.push(this.execute_recipe(factory, reservation, insertions))
                        }
                        if is_full {
                            break;
                        }
                    }
                }
            }
            join_tasks(tasks).await
        })
    }
}

impl ScatteringProcess {
    fn execute_recipe(
        &self,
        factory: &mut Factory,
        reservation: Reservation,
        insertions: FnvHashMap<usize, i32>,
    ) -> AbortOnDrop<Result<(), String>> {
        let bus_slot = factory.bus_allocate();
        let weak = self.weak.clone();
        spawn(async move {
            let bus_slot = bus_slot.await?;
            let task = async {
                let extraction = {
                    alive!(weak, this);
                    upgrade!(this.factory, factory);
                    reservation.extract(factory, bus_slot)
                };
                extraction.await?;
                let mut tasks = Vec::new();
                {
                    alive!(weak, this);
                    upgrade!(this.factory, factory);
                    let server = factory.borrow_server();
                    for (inv_slot, size) in insertions.into_iter() {
                        let access = server.load_balance(&this.config.accesses).1;
                        let action = ActionFuture::from(Call {
                            addr: access.addr,
                            func: "transferItem",
                            args: vec![
                                access.bus_side.into(),
                                access.inv_side.into(),
                                size.into(),
                                (bus_slot + 1).into(),
                                (inv_slot + 1).into(),
                            ],
                        });
                        server.enqueue_request_group(access.client, vec![action.clone().into()]);
                        tasks.push(spawn(async move { action.await.map(|_| ()) }))
                    }
                }
                join_tasks(tasks).await?;
                alive!(weak, this);
                upgrade_mut!(this.factory, factory);
                factory.bus_free(bus_slot);
                Ok(())
            };
            let result = task.await;
            if result.is_err() {
                alive!(weak, this);
                upgrade_mut!(this.factory, factory);
                factory.bus_deposit(once(bus_slot))
            }
            result
        })
    }
}
