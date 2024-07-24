use super::{extract_output, list_inv, IntoProcess, Inventory, Process};
use crate::access::{InvAccess, MultiInvAccess};
use crate::action::{ActionFuture, Call};
use crate::factory::Factory;
use crate::item::{Filter, ItemStack};
use crate::recipe::{compute_demands, Demand, Input, Outputs, Recipe};
use crate::util::{alive, join_outputs, join_tasks, spawn};
use abort_on_drop::ChildTask;
use flexstr::{local_str, LocalStr};
use fnv::{FnvHashMap, FnvHashSet};
use std::{
    cell::RefCell,
    rc::{Rc, Weak},
};

impl_input!(MultiInvSlottedInput);
#[derive(Clone)]
pub struct MultiInvSlottedInput {
    item: Filter,
    pub size: i32,
    pub slots: Vec<(usize, usize, i32)>,
    allow_backup: bool,
    extra_backup: i32,
}

impl MultiInvSlottedInput {
    pub fn new(item: Filter, slots: Vec<(usize, usize, i32)>) -> Self {
        let size = slots.iter().map(|(_, _, size)| size).sum();
        Self { item, size, slots, allow_backup: false, extra_backup: 0 }
    }
}

impl_recipe!(MultiInvSlottedRecipe, MultiInvSlottedInput);
#[derive(Clone)]
pub struct MultiInvSlottedRecipe {
    pub outputs: Rc<dyn Outputs>,
    pub inputs: Vec<MultiInvSlottedInput>,
    pub max_sets: i32,
}

pub type MultiInvExtractFilter = Box<dyn Fn(&Factory, usize, usize, &ItemStack) -> bool>;
pub fn multi_inv_extract_all() -> Option<MultiInvExtractFilter> { Some(Box::new(|_, _, _, _| true)) }

pub struct MultiInvSlottedConfig {
    pub name: LocalStr,
    pub accesses: Vec<MultiInvAccess>,
    pub input_slots: Vec<Vec<usize>>,
    pub to_extract: Option<MultiInvExtractFilter>,
    pub recipes: Vec<MultiInvSlottedRecipe>,
    pub strict_priority: bool,
}

pub struct EachInvConfig {
    pub accesses: Vec<InvAccess>,
    pub input_slots: Vec<usize>,
}

pub struct EachInv {
    pub weak: Weak<RefCell<EachInv>>,
    pub config: EachInvConfig,
    pub factory: Weak<RefCell<Factory>>,
}

impl_inventory!(EachInv);

pub struct MultiInvSlottedProcess {
    weak: Weak<RefCell<MultiInvSlottedProcess>>,
    factory: Weak<RefCell<Factory>>,
    name: LocalStr,
    accesses: Vec<MultiInvAccess>,
    to_extract: Option<MultiInvExtractFilter>,
    recipes: Vec<MultiInvSlottedRecipe>,
    strict_priority: bool,
    invs: Vec<Rc<RefCell<EachInv>>>,
}

impl IntoProcess for MultiInvSlottedConfig {
    type Output = MultiInvSlottedProcess;
    fn into_process(self, factory: &Weak<RefCell<Factory>>) -> Rc<RefCell<Self::Output>> {
        Rc::new_cyclic(|weak| {
            let accesses = self.accesses;
            let invs = Vec::from_iter(self.input_slots.into_iter().enumerate().map(|(i, input_slots)| {
                Rc::new_cyclic(|weak| {
                    RefCell::new(EachInv {
                        weak: weak.clone(),
                        config: EachInvConfig {
                            accesses: Vec::from_iter(accesses.iter().map(|access| {
                                let each = &access.invs[i];
                                InvAccess {
                                    client: access.client.clone(),
                                    addr: each.addr.clone(),
                                    bus_side: each.bus_side,
                                    inv_side: each.inv_side,
                                }
                            })),
                            input_slots,
                        },
                        factory: factory.clone(),
                    })
                })
            }));
            RefCell::new(Self::Output {
                weak: weak.clone(),
                factory: factory.clone(),
                name: self.name,
                accesses,
                to_extract: self.to_extract,
                recipes: self.recipes,
                strict_priority: self.strict_priority,
                invs,
            })
        })
    }
}

impl Process for MultiInvSlottedProcess {
    fn run(&self, factory: &Factory) -> ChildTask<Result<(), LocalStr>> {
        if self.to_extract.is_none() && compute_demands(factory, &self.recipes).is_empty() {
            return spawn(async { Ok(()) });
        }
        let stacks = Vec::from_iter(self.invs.iter().map(|inv| spawn(list_inv(&*inv.borrow(), factory))));
        let weak = self.weak.clone();
        spawn(async move {
            let stacks = join_outputs(stacks).await?;
            let mut tasks = Vec::new();
            {
                alive!(weak, this);
                upgrade_mut!(this.factory, factory);
                let mut existing_inputs = FnvHashMap::<(usize, usize), Option<ItemStack>>::default();
                for (i, inv) in this.invs.iter().enumerate() {
                    for slot in &inv.borrow().config.input_slots {
                        existing_inputs.insert((i, *slot), None);
                    }
                }
                for (i, stacks) in stacks.into_iter().enumerate() {
                    for (slot, stack) in stacks.into_iter().enumerate() {
                        if let Some(stack) = stack {
                            if let Some(existing_input) = existing_inputs.get_mut(&(i, slot)) {
                                *existing_input = Some(stack)
                            } else if let Some(ref to_extract) = this.to_extract {
                                if to_extract(factory, i, slot, &stack) {
                                    tasks.push(extract_output(
                                        &*this.invs[i].borrow(),
                                        factory,
                                        slot,
                                        stack.item.max_size,
                                    ))
                                }
                            }
                        }
                    }
                }
                let mut demands = compute_demands(factory, &this.recipes);
                if this.strict_priority {
                    demands.truncate(1)
                }
                'recipe: for mut demand in demands.into_iter() {
                    let recipe = &this.recipes[demand.i_recipe];
                    let mut used_slots = FnvHashSet::<(usize, usize)>::default();
                    for (i_input, input) in recipe.inputs.iter().enumerate() {
                        for (inv, inv_slot, mult) in &input.slots {
                            let slot = (*inv, *inv_slot);
                            let existing_input = existing_inputs.get(&slot).unwrap();
                            let existing_size = if let Some(existing_input) = existing_input {
                                if existing_input.item != demand.inputs.items[i_input] {
                                    continue 'recipe;
                                }
                                existing_input.size
                            } else {
                                0
                            };
                            demand.inputs.n_sets = demand.inputs.n_sets.min(
                                ((recipe.max_sets * mult).min(demand.inputs.items[i_input].max_size) - existing_size)
                                    / mult,
                            );
                            if demand.inputs.n_sets <= 0 {
                                continue 'recipe;
                            }
                            used_slots.insert(slot);
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

impl MultiInvSlottedProcess {
    fn execute_recipe(&self, factory: &mut Factory, demand: Demand) -> ChildTask<Result<(), LocalStr>> {
        let mut bus_slots = Vec::new();
        let slots_to_free = Rc::new(RefCell::new(Vec::new()));
        let recipe = &self.recipes[demand.i_recipe];
        for (i_input, input) in recipe.inputs.iter().enumerate() {
            let reservation =
                factory.reserve_item(&self.name, &demand.inputs.items[i_input], demand.inputs.n_sets * input.size);
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
            let slots_to_free = Rc::into_inner(slots_to_free).unwrap().into_inner();
            let task = async {
                let bus_slots = bus_slots?;
                let mut tasks = Vec::new();
                {
                    alive!(weak, this);
                    upgrade!(this.factory, factory);
                    let server = factory.borrow_server();
                    let access = server.load_balance(&this.accesses).1;
                    let mut group = Vec::new();
                    let recipe = &this.recipes[demand.i_recipe];
                    for (i_input, input) in recipe.inputs.iter().enumerate() {
                        for (inv, inv_slot, mult) in &input.slots {
                            let access = &access.invs[*inv];
                            let action = ActionFuture::from(Call {
                                addr: access.addr.clone(),
                                func: local_str!("transferItem"),
                                args: vec![
                                    access.bus_side.into(),
                                    access.inv_side.into(),
                                    (demand.inputs.n_sets * mult).into(),
                                    (bus_slots[i_input] + 1).into(),
                                    (inv_slot + 1).into(),
                                ],
                            });
                            group.push(action.clone().into());
                            tasks.push(spawn(async move { action.await.map(|_| ()) }));
                        }
                    }
                    server.enqueue_request_group(&access.client, group)
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
