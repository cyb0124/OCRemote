use super::{
    extract_output, list_inv, EachInv, EachInvConfig, IntoProcess, MultiInvExtractFilter, MultiInvSlottedInput, Process,
};
use crate::access::{EachTank, InvAccess, InvTankAccess};
use crate::action::{ActionFuture, Call};
use crate::factory::{read_tanks, tanks_to_fluid_map, Factory, Tank};
use crate::item::ItemStack;
use crate::recipe::{resolve_inputs, Demand, Outputs, Recipe};
use crate::util::{alive, join_outputs, join_tasks, spawn};
use abort_on_drop::ChildTask;
use flexstr::{local_str, LocalStr};
use fnv::{FnvHashMap, FnvHashSet};
use std::collections::hash_map::Entry;
use std::{
    cell::RefCell,
    rc::{Rc, Weak},
};

#[derive(Clone)]
pub struct FluidSlottedInput {
    fluid: LocalStr,
    size: i64,
    tanks: Vec<(usize, i64)>,
    allow_backup: bool,
    extra_backup: i64,
}

impl FluidSlottedInput {
    pub fn new(fluid: LocalStr, tanks: Vec<(usize, i64)>) -> Self {
        let size = tanks.iter().map(|(_, size)| size).sum();
        Self { fluid, size, tanks, allow_backup: false, extra_backup: 0 }
    }

    pub fn allow_backup(mut self) -> Self {
        self.allow_backup = true;
        self
    }

    pub fn extra_backup(mut self, size: i64) -> Self {
        self.extra_backup += size;
        self
    }
}

impl_recipe!(FluidSlottedRecipe, MultiInvSlottedInput);
#[derive(Clone)]
pub struct FluidSlottedRecipe {
    pub outputs: Rc<dyn Outputs>,
    pub inputs: Vec<MultiInvSlottedInput>,
    pub fluids: Vec<FluidSlottedInput>,
    pub max_sets: i32,
}

pub type FluidExtractFilter = Box<dyn Fn(&Factory, usize, Vec<Tank>) -> FnvHashMap<LocalStr, (usize, i64)>>;
pub fn fluid_extract_all() -> Option<FluidExtractFilter> { Some(Box::new(|_, _, tanks| tanks_to_fluid_map(&tanks))) }
pub fn fluid_extract_slots(slots: impl Fn(usize, usize) -> bool + 'static) -> Option<FluidExtractFilter> {
    Some(Box::new(move |_, i, tanks| {
        let mut result = FnvHashMap::default();
        for (slot, tank) in tanks.into_iter().enumerate() {
            let Some(fluid) = tank.fluid else { continue };
            if slots(i, slot) {
                result.entry(fluid.clone()).or_insert((slot, 0)).1 += tank.qty
            }
        }
        result
    }))
}

pub struct FluidSlottedConfig {
    pub name: LocalStr,
    pub input_slots: Vec<Vec<usize>>,
    pub input_tanks: Vec<Vec<usize>>,
    pub accesses: Vec<InvTankAccess>,
    pub to_extract: Option<MultiInvExtractFilter>,
    pub fluid_extract: Option<FluidExtractFilter>,
    pub recipes: Vec<FluidSlottedRecipe>,
    pub strict_priority: bool,
}

pub struct FluidSlottedProcess {
    weak: Weak<RefCell<FluidSlottedProcess>>,
    name: LocalStr,
    accesses: Vec<InvTankAccess>,
    factory: Weak<RefCell<Factory>>,
    to_extract: Option<MultiInvExtractFilter>,
    fluid_extract: Option<FluidExtractFilter>,
    recipes: Vec<FluidSlottedRecipe>,
    strict_priority: bool,
    invs: Vec<Rc<RefCell<EachInv>>>,
    input_tanks: Vec<Vec<usize>>,
}

impl IntoProcess for FluidSlottedConfig {
    type Output = FluidSlottedProcess;
    fn into_process(self, factory: &Factory) -> Rc<RefCell<Self::Output>> {
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
                        factory: factory.weak.clone(),
                    })
                })
            }));
            RefCell::new(Self::Output {
                weak: weak.clone(),
                name: self.name,
                accesses,
                factory: factory.weak.clone(),
                to_extract: self.to_extract,
                fluid_extract: self.fluid_extract,
                recipes: self.recipes,
                strict_priority: self.strict_priority,
                invs,
                input_tanks: self.input_tanks,
            })
        })
    }
}

struct InputInfo {
    n_available: i64,
    n_needed: i64,
}

fn compute_fluid_demands(factory: &Factory, recipes: &[FluidSlottedRecipe]) -> Vec<Demand> {
    let mut result = Vec::new();
    for (i_recipe, recipe) in recipes.iter().enumerate() {
        let Some(mut priority) = recipe.get_outputs().get_priority(factory) else { continue };
        let Some(mut inputs) = resolve_inputs(factory, recipe) else { continue };
        let mut infos = FnvHashMap::<LocalStr, InputInfo>::default();
        let mut bus_bound = i64::MAX;
        for input in &recipe.fluids {
            match infos.entry(input.fluid.clone()) {
                Entry::Occupied(input_info) => input_info.into_mut().n_needed += input.size,
                Entry::Vacant(input_info) => {
                    input_info.insert(InputInfo {
                        // Note: backup params are considered for only the first input of the same fluid.
                        n_available: factory.get_fluid_availability(
                            &*input.fluid,
                            input.allow_backup,
                            input.extra_backup,
                        ),
                        n_needed: input.size,
                    });
                }
            }
            bus_bound = bus_bound.min(factory.config.fluid_bus_capacity / input.size)
        }
        let mut availability_bound = i64::MAX;
        for (_, input_info) in infos {
            availability_bound = availability_bound.min(input_info.n_available / input_info.n_needed)
        }
        inputs.n_sets = (inputs.n_sets as i64).min(bus_bound).min(availability_bound) as _;
        if inputs.n_sets > 0 {
            inputs.priority = (inputs.priority as i64).min(availability_bound) as _;
            priority *= inputs.priority as f64;
            result.push(Demand { i_recipe, inputs, priority })
        }
    }
    result.sort_by(|x: &Demand, y: &Demand| x.priority.partial_cmp(&y.priority).unwrap().reverse());
    result
}

impl Process for FluidSlottedProcess {
    fn run(&self, factory: &Factory) -> ChildTask<Result<(), LocalStr>> {
        if self.to_extract.is_none()
            && self.fluid_extract.is_none()
            && compute_fluid_demands(factory, &self.recipes).is_empty()
        {
            return spawn(async { Ok(()) });
        }
        let stacks = Vec::from_iter(self.invs.iter().map(|inv| spawn(list_inv(&*inv.borrow(), factory))));
        let tanks = Vec::from_iter((0..self.input_tanks.len()).map(|i| {
            spawn(read_tanks(&*factory.borrow_server(), &self.accesses, |access| {
                let bus_of_tank = &access.tanks[i][0];
                EachTank { addr: bus_of_tank.addr.clone(), side: bus_of_tank.tank_side }
            }))
        }));
        let weak = self.weak.clone();
        spawn(async move {
            let stacks = join_outputs(stacks).await?;
            let tanks = join_outputs(tanks).await?;
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
                            } else if let Some(to_extract) = &this.to_extract {
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
                let existing_fluids = Vec::from_iter(tanks.into_iter().enumerate().map(|(i, tanks)| {
                    let mut fluid_map = FnvHashMap::<LocalStr, i64>::default();
                    for &slot in &this.input_tanks[i] {
                        let tank = &tanks[slot];
                        let Some(fluid) = &tank.fluid else { continue };
                        *fluid_map.entry(fluid.clone()).or_default() += tank.qty
                    }
                    if let Some(fluid_extract) = &this.fluid_extract {
                        this.extract_fluids(factory, i, fluid_extract(factory, i, tanks), &mut tasks)
                    }
                    fluid_map
                }));
                let mut demands = compute_fluid_demands(factory, &this.recipes);
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
                    let mut mismatched_fluids = FnvHashSet::from_iter(
                        (existing_fluids.iter().enumerate())
                            .flat_map(|(i, fluid_map)| fluid_map.keys().map(move |fluid| (i, fluid.clone()))),
                    );
                    for input in &recipe.fluids {
                        for &(i, mult) in &input.tanks {
                            mismatched_fluids.remove(&(i, input.fluid.clone()));
                            let fluid_map = &existing_fluids[i];
                            let existing_size = fluid_map.get(&input.fluid).copied().unwrap_or_default();
                            demand.inputs.n_sets = (demand.inputs.n_sets)
                                .min(((recipe.max_sets as i64 * mult - existing_size) / mult).clamp(0, i32::MAX as _)
                                    as _);
                            if demand.inputs.n_sets <= 0 {
                                continue 'recipe;
                            }
                        }
                    }
                    if !mismatched_fluids.is_empty() {
                        continue;
                    }
                    tasks.push(this.execute_recipe(factory, demand));
                    break;
                }
            }
            join_tasks(tasks).await
        })
    }
}

impl FluidSlottedProcess {
    fn extract_fluids(
        &self,
        factory: &mut Factory,
        i: usize,
        fluids: FnvHashMap<LocalStr, (usize, i64)>,
        tasks: &mut Vec<ChildTask<Result<(), LocalStr>>>,
    ) {
        for (_, (qty, slot)) in fluids {
            let bus = factory.fluid_bus_allocate();
            let weak = self.weak.clone();
            tasks.push(spawn(async move {
                let bus = bus.await?;
                let task;
                {
                    alive!(weak, this);
                    upgrade!(this.factory, factory);
                    let server = factory.borrow_server();
                    let access = server.load_balance(&this.accesses).1;
                    let bus_of_tank = &access.tanks[i][bus];
                    task = ActionFuture::from(Call {
                        addr: bus_of_tank.addr.clone(),
                        func: local_str!("transferFluid"),
                        args: vec![
                            bus_of_tank.tank_side.into(),
                            bus_of_tank.bus_side.into(),
                            qty.into(),
                            (slot + 1).into(),
                        ],
                    });
                    server.enqueue_request_group(&access.client, vec![task.clone().into()])
                }
                let result = task.await.map(|_| ());
                alive(&weak)?.borrow().factory.upgrade().unwrap().borrow_mut().fluid_bus_deposit([bus]);
                result
            }))
        }
    }

    fn execute_recipe(&self, factory: &mut Factory, demand: Demand) -> ChildTask<Result<(), LocalStr>> {
        let mut bus_slots = Vec::new();
        let slots_to_free = Rc::new(RefCell::new(Vec::new()));
        let mut fluid_buses = Vec::new();
        let fluid_buses_to_free = Rc::new(RefCell::new(Vec::new()));
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
        for input in &recipe.fluids {
            let reservation =
                factory.reserve_fluid(&self.name, &*input.fluid, input.size * demand.inputs.n_sets as i64);
            let fluid_bus = factory.fluid_bus_allocate();
            let fluid_buses_to_free = fluid_buses_to_free.clone();
            fluid_buses.push(spawn(async move {
                let fluid_bus = fluid_bus.await?;
                fluid_buses_to_free.borrow_mut().push(fluid_bus);
                let extraction = reservation.extract(fluid_bus);
                extraction.await.map(|_| fluid_bus)
            }))
        }
        let weak = self.weak.clone();
        spawn(async move {
            let bus_slots = join_outputs(bus_slots).await;
            let fluid_buses = join_outputs(fluid_buses).await;
            let slots_to_free = Rc::into_inner(slots_to_free).unwrap().into_inner();
            let fluid_buses_to_free = Rc::into_inner(fluid_buses_to_free).unwrap().into_inner();
            let task = async {
                let bus_slots = bus_slots?;
                let fluid_buses = fluid_buses?;
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
                    for (input, fluid_bus) in recipe.fluids.iter().zip(fluid_buses) {
                        for &(i, mult) in &input.tanks {
                            let bus_of_tank = &access.tanks[i][fluid_bus];
                            let action = ActionFuture::from(Call {
                                addr: bus_of_tank.addr.clone(),
                                func: local_str!("transferFluid"),
                                args: vec![
                                    bus_of_tank.bus_side.into(),
                                    bus_of_tank.tank_side.into(),
                                    (demand.inputs.n_sets as i64 * mult).into(),
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
                for &fluid_bus_to_free in &fluid_buses_to_free {
                    factory.fluid_bus_free(fluid_bus_to_free)
                }
                Ok(())
            };
            let result = task.await;
            if result.is_err() {
                alive!(weak, this);
                upgrade_mut!(this.factory, factory);
                factory.bus_deposit(slots_to_free);
                factory.fluid_bus_deposit(fluid_buses_to_free)
            }
            result
        })
    }
}
