use super::super::access::InvAccess;
use super::super::factory::Factory;
use super::super::item::{Filter, ItemStack};
use super::super::recipe::{compute_demands, resolve_inputs, Demand, Input, Output, Recipe};
use super::super::util::{alive, join_tasks, spawn};
use super::{extract_output, list_inv, scattering_insert, ExtractFilter, IntoProcess, Inventory, Process};
use abort_on_drop::ChildTask;
use flexstr::{local_fmt, LocalStr};
use fnv::FnvHashMap;
use std::{
    cell::RefCell,
    cmp::min,
    rc::{Rc, Weak},
};

pub struct ScatteringInput {
    item: Filter,
    size: i32,
    allow_backup: bool,
    extra_backup: i32,
}

impl ScatteringInput {
    pub fn new(item: Filter) -> Self { ScatteringInput { item, size: 1, allow_backup: false, extra_backup: 0 } }
}

impl_input!(ScatteringInput);

pub struct ScatteringRecipe {
    outputs: Vec<Output>,
    inputs: Vec<ScatteringInput>,
}

impl ScatteringRecipe {
    pub fn new(outputs: Vec<Output>, input: ScatteringInput) -> Self {
        ScatteringRecipe { outputs, inputs: vec![input] }
    }
}

impl_recipe!(ScatteringRecipe, ScatteringInput);

pub struct ScatteringConfig {
    pub name: LocalStr,
    pub accesses: Vec<InvAccess>,
    // plant_sower: 6, 7, .., 14
    pub input_slots: Vec<usize>,
    pub to_extract: Option<ExtractFilter>,
    pub recipes: Vec<ScatteringRecipe>,
    pub max_per_slot: i32,
}

pub struct ScatteringProcess {
    weak: Weak<RefCell<ScatteringProcess>>,
    config: ScatteringConfig,
    factory: Weak<RefCell<Factory>>,
}

impl_inventory!(ScatteringProcess);

impl IntoProcess for ScatteringConfig {
    type Output = ScatteringProcess;
    fn into_process(self, factory: &Weak<RefCell<Factory>>) -> Rc<RefCell<Self::Output>> {
        Rc::new_cyclic(|weak| RefCell::new(Self::Output { weak: weak.clone(), config: self, factory: factory.clone() }))
    }
}

impl Process for ScatteringProcess {
    fn run(&self, factory: &Factory) -> ChildTask<Result<(), LocalStr>> {
        if self.config.to_extract.is_none() && compute_demands(factory, &self.config.recipes).is_empty() {
            return spawn(async { Ok(()) });
        }
        let stacks = list_inv(self, factory);
        let weak = self.weak.clone();
        spawn(async move {
            let mut stacks = stacks.await?;
            let mut tasks = Vec::new();
            {
                alive!(weak, this);
                upgrade_mut!(this.factory, factory);
                let mut is_input_slot = vec![false; stacks.len()];
                for slot in &this.config.input_slots {
                    if *slot >= stacks.len() {
                        return Err(local_fmt!("{}: invalid slot", this.config.name));
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
                        while inputs.n_sets > 0 {
                            let mut best = None;
                            for slot in &this.config.input_slots {
                                if let Some(ref stack) = stacks[*slot] {
                                    if stack.item == inputs.items[0] {
                                        if let Some((_, best_size)) = best {
                                            if stack.size >= best_size {
                                                continue;
                                            }
                                        }
                                        best = Some((*slot, stack.size))
                                    }
                                } else {
                                    best = Some((*slot, 0));
                                    break;
                                }
                            }
                            if let Some((slot, size)) = best {
                                if size >= min(this.config.max_per_slot, inputs.items[0].max_size) {
                                    break;
                                }
                                inputs.n_sets -= 1;
                                n_inserted += 1;
                                *insertions.entry(slot).or_default() += 1;
                                let stack = &mut stacks[slot];
                                if let Some(ref mut stack) = stack {
                                    stack.size += 1
                                } else {
                                    *stack = Some(ItemStack { item: inputs.items[0].clone(), size: 1 })
                                }
                            } else {
                                break;
                            }
                        }
                        if n_inserted > 0 {
                            let reservation = factory.reserve_item(&this.config.name, &inputs.items[0], n_inserted);
                            tasks.push(scattering_insert(this, factory, reservation, insertions))
                        }
                    }
                }
            }
            join_tasks(tasks).await
        })
    }
}
