use super::super::access::{Access, CraftingRobotAccess, WorkbenchAccess};
use super::super::action::{ActionFuture, Call};
use super::super::factory::Factory;
use super::super::item::Filter;
use super::super::recipe::{compute_demands, resolve_inputs, Demand, Input, Outputs, Recipe, ResolvedInputs};
use super::super::side::{DOWN, UP};
use super::super::util::{alive, join_outputs, join_tasks, spawn};
use super::{IntoProcess, Process};
use abort_on_drop::ChildTask;
use flexstr::{local_str, LocalStr};
use std::{
    cell::RefCell,
    rc::{Rc, Weak},
};

#[derive(Clone)]
pub struct CraftingGridInput {
    item: Filter,
    size: i32,
    slots: Vec<usize>,
    allow_backup: bool,
    extra_backup: i32,
}

impl_input!(CraftingGridInput);
impl CraftingGridInput {
    pub fn new(item: Filter, slots: Vec<usize>) -> Self {
        CraftingGridInput { item, size: slots.len() as i32, slots, allow_backup: false, extra_backup: 0 }
    }
}

#[derive(Clone)]
pub struct NonConsumable {
    // for crafting robot:
    //   3, 7, 11, 12, 13, 14
    //   with extra inventory: 16, 17, ..
    pub storage_slot: usize,
    pub crafting_grid_slot: usize,
}

impl_recipe!(CraftingGridRecipe, CraftingGridInput);
#[derive(Clone)]
pub struct CraftingGridRecipe {
    pub outputs: Rc<dyn Outputs>,
    // slots:
    //   0, 1, 2
    //   3, 4, 5
    //   6, 7, 8
    pub inputs: Vec<CraftingGridInput>,
    // can't craft more than one stack at a time.
    pub max_sets: i32,
    pub non_consumables: Vec<NonConsumable>,
}

trait CraftingGridProcess: 'static {
    type Access: Access;
    fn get_accesses(&self) -> &Vec<Self::Access>;
    fn get_recipes(&self) -> &Vec<CraftingGridRecipe>;
    fn get_factory(&self) -> &Weak<RefCell<Factory>>;
    fn get_weak(&self) -> &Weak<RefCell<Self>>;
    fn get_name(&self) -> &str;
    fn load_input(group: &mut Vec<Call>, access: &Self::Access, bus_slot: usize, inv_slot: usize, size: i32);
    fn load_non_consumable(group: &mut Vec<Call>, access: &Self::Access, non_consumable: &NonConsumable);
    fn store_output(group: &mut Vec<Call>, access: &Self::Access, bus_slot: usize, n_sets: i32);
    fn store_non_consumable(group: &mut Vec<Call>, access: &Self::Access, non_consumable: &NonConsumable);
}

macro_rules! impl_crafting_grid_process {
    () => {
        fn get_accesses(&self) -> &Vec<Self::Access> { &self.config.accesses }
        fn get_recipes(&self) -> &Vec<CraftingGridRecipe> { &self.config.recipes }
        fn get_factory(&self) -> &Weak<RefCell<Factory>> { &self.factory }
        fn get_weak(&self) -> &Weak<RefCell<Self>> { &self.weak }
        fn get_name(&self) -> &str { &self.config.name }
    };
}

fn run_crafting_grid_process<T>(this: &T, factory: &Factory) -> ChildTask<Result<(), LocalStr>>
where
    T: CraftingGridProcess,
{
    let mut tasks = Vec::new();
    for Demand { i_recipe, .. } in compute_demands(factory, this.get_recipes()) {
        let recipe = &this.get_recipes()[i_recipe];
        if recipe.max_sets <= 0 {
            continue;
        }
        if let Some(ResolvedInputs { mut n_sets, items, .. }) = resolve_inputs(factory, recipe) {
            n_sets = n_sets.min(recipe.max_sets);
            let mut bus_slots = Vec::new();
            let slots_to_free = Rc::new(RefCell::new(Vec::new()));
            for (i_input, item) in items.into_iter().enumerate() {
                let reservation = factory.reserve_item(this.get_name(), &item, n_sets * recipe.inputs[i_input].size);
                let slots_to_free = slots_to_free.clone();
                let weak = this.get_factory().clone();
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
                let weak = this.get_factory().clone();
                bus_slots.push(spawn(async move {
                    let bus_slot = alive(&weak)?.borrow_mut().bus_allocate();
                    let bus_slot = bus_slot.await?;
                    slots_to_free.borrow_mut().push(bus_slot);
                    Ok(bus_slot)
                }))
            }
            let weak = this.get_weak().clone();
            tasks.push(spawn(async move {
                let bus_slots = join_outputs(bus_slots).await;
                let mut slots_to_free = Rc::into_inner(slots_to_free).unwrap().into_inner();
                let task = async {
                    let bus_slots = bus_slots?;
                    let tasks = {
                        alive!(weak, this);
                        upgrade!(this.get_factory(), factory);
                        let server = factory.borrow_server();
                        let access = server.load_balance(this.get_accesses()).1;
                        let mut group = Vec::new();
                        let recipe = &this.get_recipes()[i_recipe];
                        for (i_input, input) in recipe.inputs.iter().enumerate() {
                            for inv_slot in &input.slots {
                                T::load_input(&mut group, access, bus_slots[i_input], *inv_slot, n_sets)
                            }
                        }
                        for non_consumable in &recipe.non_consumables {
                            T::load_non_consumable(&mut group, access, non_consumable)
                        }
                        T::store_output(&mut group, access, slots_to_free[0], n_sets);
                        for non_consumable in &recipe.non_consumables {
                            T::store_non_consumable(&mut group, access, non_consumable)
                        }
                        let group: Vec<_> = group.into_iter().map(|x| ActionFuture::from(x)).collect();
                        server.enqueue_request_group(
                            access.get_client(),
                            group.iter().map(|x| x.clone().into()).collect(),
                        );
                        group.into_iter().map(|x| spawn(async move { x.await.map(|_| ()) })).collect()
                    };
                    join_tasks(tasks).await?;
                    alive!(weak, this);
                    upgrade_mut!(this.get_factory(), factory);
                    while slots_to_free.len() > 1 {
                        factory.bus_free(slots_to_free.pop().unwrap())
                    }
                    Ok(())
                };
                let result = task.await;
                alive!(weak, this);
                upgrade_mut!(this.get_factory(), factory);
                factory.bus_deposit(slots_to_free);
                result
            }))
        }
    }
    spawn(async move { join_tasks(tasks).await })
}

pub struct CraftingRobotConfig {
    pub name: LocalStr,
    pub accesses: Vec<CraftingRobotAccess>,
    pub recipes: Vec<CraftingGridRecipe>,
}

pub struct CraftingRobotProcess {
    weak: Weak<RefCell<CraftingRobotProcess>>,
    config: CraftingRobotConfig,
    factory: Weak<RefCell<Factory>>,
}

impl IntoProcess for CraftingRobotConfig {
    type Output = CraftingRobotProcess;
    fn into_process(self, factory: &Weak<RefCell<Factory>>) -> Rc<RefCell<Self::Output>> {
        Rc::new_cyclic(|weak| RefCell::new(Self::Output { weak: weak.clone(), config: self, factory: factory.clone() }))
    }
}

fn map_robot_grid(slot: usize) -> usize {
    if slot >= 6 {
        slot + 2
    } else if slot >= 3 {
        slot + 1
    } else {
        slot
    }
}

impl CraftingGridProcess for CraftingRobotProcess {
    type Access = CraftingRobotAccess;
    impl_crafting_grid_process!();

    fn load_input(group: &mut Vec<Call>, access: &Self::Access, bus_slot: usize, inv_slot: usize, size: i32) {
        group.push(Call {
            addr: local_str!("robot"),
            func: local_str!("select"),
            args: vec![(map_robot_grid(inv_slot) + 1).into()],
        });
        group.push(Call {
            addr: local_str!("inventory_controller"),
            func: local_str!("suckFromSlot"),
            args: vec![access.bus_side.into(), (bus_slot + 1).into(), size.into()],
        })
    }

    fn load_non_consumable(group: &mut Vec<Call>, _access: &Self::Access, non_consumable: &NonConsumable) {
        group.push(Call {
            addr: local_str!("robot"),
            func: local_str!("select"),
            args: vec![(non_consumable.storage_slot + 1).into()],
        });
        group.push(Call {
            addr: local_str!("robot"),
            func: local_str!("transferTo"),
            args: vec![(map_robot_grid(non_consumable.crafting_grid_slot) + 1).into()],
        })
    }

    fn store_output(group: &mut Vec<Call>, access: &Self::Access, bus_slot: usize, _n_sets: i32) {
        group.push(Call { addr: local_str!("robot"), func: local_str!("select"), args: vec![16.into()] });
        group.push(Call { addr: local_str!("crafting"), func: local_str!("craft"), args: Vec::new() });
        group.push(Call {
            addr: local_str!("inventory_controller"),
            func: local_str!("dropIntoSlot"),
            args: vec![access.bus_side.into(), (bus_slot + 1).into()],
        });
    }

    fn store_non_consumable(group: &mut Vec<Call>, _access: &Self::Access, non_consumable: &NonConsumable) {
        group.push(Call {
            addr: local_str!("robot"),
            func: local_str!("select"),
            args: vec![(map_robot_grid(non_consumable.crafting_grid_slot) + 1).into()],
        });
        group.push(Call {
            addr: local_str!("robot"),
            func: local_str!("transferTo"),
            args: vec![(non_consumable.storage_slot + 1).into()],
        });
    }
}

impl Process for CraftingRobotProcess {
    fn run(&self, factory: &Factory) -> ChildTask<Result<(), LocalStr>> { run_crafting_grid_process(self, factory) }
}

pub struct WorkbenchConfig {
    pub name: LocalStr,
    pub accesses: Vec<WorkbenchAccess>,
    pub recipes: Vec<CraftingGridRecipe>,
}

pub struct WorkbenchProcess {
    weak: Weak<RefCell<WorkbenchProcess>>,
    config: WorkbenchConfig,
    factory: Weak<RefCell<Factory>>,
}

impl IntoProcess for WorkbenchConfig {
    type Output = WorkbenchProcess;
    fn into_process(self, factory: &Weak<RefCell<Factory>>) -> Rc<RefCell<Self::Output>> {
        Rc::new_cyclic(|weak| RefCell::new(Self::Output { weak: weak.clone(), config: self, factory: factory.clone() }))
    }
}

impl CraftingGridProcess for WorkbenchProcess {
    type Access = WorkbenchAccess;
    impl_crafting_grid_process!();

    fn load_input(group: &mut Vec<Call>, access: &Self::Access, bus_slot: usize, inv_slot: usize, size: i32) {
        group.push(Call {
            addr: access.input_addr.clone(),
            func: local_str!("transferItem"),
            args: vec![
                access.input_bus_side.into(),
                DOWN.into(),
                size.into(),
                (bus_slot + 1).into(),
                (inv_slot + 1).into(),
            ],
        });
    }

    fn load_non_consumable(group: &mut Vec<Call>, access: &Self::Access, non_consumable: &NonConsumable) {
        group.push(Call {
            addr: access.input_addr.clone(),
            func: local_str!("transferItem"),
            args: vec![
                access.non_consumable_side.into(),
                DOWN.into(),
                64.into(),
                (non_consumable.storage_slot + 1).into(),
                (non_consumable.crafting_grid_slot + 1).into(),
            ],
        })
    }

    fn store_output(group: &mut Vec<Call>, access: &Self::Access, bus_slot: usize, n_sets: i32) {
        for _ in 0..n_sets {
            group.push(Call {
                addr: access.output_addr.clone(),
                func: local_str!("transferItem"),
                args: vec![UP.into(), access.output_bus_side.into(), 64.into(), 1.into(), (bus_slot + 1).into()],
            })
        }
    }

    fn store_non_consumable(group: &mut Vec<Call>, access: &Self::Access, non_consumable: &NonConsumable) {
        group.push(Call {
            addr: access.input_addr.clone(),
            func: local_str!("transferItem"),
            args: vec![
                DOWN.into(),
                access.non_consumable_side.into(),
                64.into(),
                (non_consumable.crafting_grid_slot + 1).into(),
                (non_consumable.storage_slot + 1).into(),
            ],
        });
    }
}

impl Process for WorkbenchProcess {
    fn run(&self, factory: &Factory) -> ChildTask<Result<(), LocalStr>> { run_crafting_grid_process(self, factory) }
}
