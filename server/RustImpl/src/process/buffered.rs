use super::super::access::InvAccess;
use super::super::action::{ActionFuture, Call};
use super::super::factory::Factory;
use super::super::item::{insert_into_inventory, jammer, Filter, InsertPlan, Item, ItemStack};
use super::super::recipe::{compute_demands, resolve_inputs, Demand, Input, Outputs, Recipe};
use super::super::util::{alive, join_outputs, join_tasks, spawn};
use super::{extract_output, list_inv, scattering_insert, ExtractFilter, IntoProcess, Inventory, Process, SlotFilter};
use abort_on_drop::ChildTask;
use flexstr::{local_str, LocalStr};
use fnv::FnvHashMap;
use std::{
    cell::RefCell,
    rc::{Rc, Weak},
};

impl_input!(BufferedInput);
#[derive(Clone)]
pub struct BufferedInput {
    item: Filter,
    size: i32,
    allow_backup: bool,
    extra_backup: i32,
}

impl BufferedInput {
    pub fn new(item: Filter, size: i32) -> Self { BufferedInput { item, size, allow_backup: false, extra_backup: 0 } }
}

impl_recipe!(BufferedRecipe, BufferedInput);
#[derive(Clone)]
pub struct BufferedRecipe {
    pub outputs: Rc<dyn Outputs>,
    pub inputs: Vec<BufferedInput>,
    pub max_inputs: i32,
}

pub struct BufferedConfig {
    pub name: LocalStr,
    pub accesses: Vec<InvAccess>,
    pub slot_filter: Option<SlotFilter>,
    pub to_extract: Option<ExtractFilter>,
    pub recipes: Vec<BufferedRecipe>,
    pub max_recipe_inputs: i32,
    pub stocks: Vec<BufferedInput>,
}

pub struct BufferedProcess {
    weak: Weak<RefCell<BufferedProcess>>,
    config: BufferedConfig,
    factory: Weak<RefCell<Factory>>,
}

impl_inventory!(BufferedProcess);

impl IntoProcess for BufferedConfig {
    type Output = BufferedProcess;
    fn into_process(self, factory: &Factory) -> Rc<RefCell<Self::Output>> {
        Rc::new_cyclic(|weak| {
            RefCell::new(Self::Output { weak: weak.clone(), config: self, factory: factory.weak.clone() })
        })
    }
}

impl Process for BufferedProcess {
    fn run(&self, factory: &Factory) -> ChildTask<Result<(), LocalStr>> {
        if self.config.to_extract.is_none() && self.config.stocks.is_empty() {
            if compute_demands(factory, &self.config.recipes).is_empty() {
                return spawn(async { Ok(()) });
            }
        }
        let stacks = list_inv(self, factory);
        let weak = self.weak.clone();
        spawn(async move {
            let mut stacks = stacks.await?;
            let mut tasks = Vec::new();
            {
                alive!(weak, this);
                upgrade_mut!(this.factory, factory);
                let mut remaining_size = this.config.max_recipe_inputs;
                let mut existing_size = FnvHashMap::<Rc<Item>, i32>::default();
                'slot: for (slot, stack) in stacks.iter_mut().enumerate() {
                    if let Some(ref to_extract) = this.config.to_extract {
                        if let Some(some_stack) = stack {
                            if to_extract(factory, slot, some_stack) {
                                tasks.push(extract_output(this, factory, slot, some_stack.item.max_size));
                                *stack = Some(ItemStack { item: jammer(), size: 1 });
                                continue 'slot;
                            }
                        }
                    }
                    if let Some(ref slot_filter) = this.config.slot_filter {
                        if !slot_filter(slot) {
                            *stack = Some(ItemStack { item: jammer(), size: 1 });
                            continue 'slot;
                        }
                    }
                    if let Some(stack) = stack {
                        *existing_size.entry(stack.item.clone()).or_default() += stack.size;
                        for stock in &this.config.stocks {
                            if stock.item.apply(&stack.item) {
                                continue 'slot;
                            }
                        }
                        remaining_size -= stack.size;
                    }
                }
                for stock in &this.config.stocks {
                    if let Some((item, info)) = factory.search_item(&stock.item) {
                        let existing = existing_size.entry(item.clone()).or_default();
                        let to_insert = (stock.size - *existing)
                            .min(info.borrow().get_availability(stock.allow_backup, stock.extra_backup));
                        if to_insert <= 0 {
                            continue;
                        }
                        let InsertPlan { n_inserted, insertions } = insert_into_inventory(&mut stacks, item, to_insert);
                        if n_inserted <= 0 {
                            continue;
                        }
                        *existing += n_inserted;
                        let reservation = factory.reserve_item(&this.config.name, item, n_inserted);
                        tasks.push(scattering_insert(this, factory, reservation, insertions))
                    }
                }
                if remaining_size > 0 {
                    'recipe: for Demand { i_recipe, .. } in compute_demands(factory, &this.config.recipes) {
                        let recipe = &this.config.recipes[i_recipe];
                        if let Some(mut inputs) = resolve_inputs(factory, recipe) {
                            let size_per_set: i32 = recipe.inputs.iter().map(|x| x.size).sum();
                            inputs.n_sets = inputs.n_sets.min(remaining_size / size_per_set);
                            if inputs.n_sets <= 0 {
                                continue 'recipe;
                            }
                            let existing_total: i32 =
                                inputs.items.iter().map(|item| *existing_size.entry(item.clone()).or_default()).sum();
                            inputs.n_sets = inputs.n_sets.min((recipe.max_inputs - existing_total) / size_per_set);
                            if inputs.n_sets <= 0 {
                                continue 'recipe;
                            }
                            let backup = stacks.clone();
                            let mut plans = Vec::new();
                            plans.reserve(recipe.inputs.len());
                            'retry: loop {
                                for (i_input, item) in inputs.items.iter().enumerate() {
                                    let to_insert = inputs.n_sets * recipe.inputs[i_input].size;
                                    let plan = insert_into_inventory(&mut stacks, item, to_insert);
                                    if plan.n_inserted == to_insert {
                                        plans.push(plan)
                                    } else {
                                        inputs.n_sets -= 1;
                                        if inputs.n_sets <= 0 {
                                            continue 'recipe;
                                        }
                                        plans.clear();
                                        stacks = backup.clone();
                                        continue 'retry;
                                    }
                                }
                                break 'retry;
                            }
                            for (i_input, item) in inputs.items.iter().enumerate() {
                                *existing_size.get_mut(item).unwrap() += plans[i_input].n_inserted
                            }
                            remaining_size -= inputs.n_sets * size_per_set;
                            tasks.push(this.execute_recipe(factory, inputs.items, plans));
                            if remaining_size <= 0 {
                                break 'recipe;
                            }
                        }
                    }
                }
            }
            join_tasks(tasks).await
        })
    }
}

impl BufferedProcess {
    fn execute_recipe(
        &self,
        factory: &mut Factory,
        items: Vec<Rc<Item>>,
        plans: Vec<InsertPlan>,
    ) -> ChildTask<Result<(), LocalStr>> {
        let mut bus_slots = Vec::new();
        let slots_to_free = Rc::new(RefCell::new(Vec::new()));
        for (i_input, item) in items.into_iter().enumerate() {
            let reservation = factory.reserve_item(&self.config.name, &item, plans[i_input].n_inserted);
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
                    let access = server.load_balance(this.config.accesses.iter()).1;
                    let mut group = Vec::new();
                    for (i_input, InsertPlan { insertions, .. }) in plans.into_iter().enumerate() {
                        for (inv_slot, size) in insertions {
                            let action = ActionFuture::from(Call {
                                addr: access.addr.clone(),
                                func: local_str!("transferItem"),
                                args: vec![
                                    access.bus_side.into(),
                                    access.inv_side.into(),
                                    size.into(),
                                    (bus_slots[i_input] + 1).into(),
                                    (inv_slot + 1).into(),
                                ],
                            });
                            group.push(action.clone().into());
                            tasks.push(spawn(async move { action.await.map(|_| ()) }))
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
