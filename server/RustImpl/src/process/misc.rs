use super::super::access::{ComponentAccess, InvAccess, SidedAccess};
use super::super::action::{ActionFuture, Call, Print};
use super::super::factory::Factory;
use super::super::item::Filter;
use super::super::lua_value::{call_result, table_remove};
use super::super::recipe::Input;
use super::super::util::{alive, join_tasks, spawn};
use super::{list_inv, IntoProcess, Inventory, Process, RedstoneEmitterConfig, RedstoneEmitterProcess, ScatteringInput};
use abort_on_drop::ChildTask;
use flexstr::{local_fmt, local_str, LocalStr};
use std::{
    cell::RefCell,
    fs::read_to_string,
    iter::once,
    rc::{Rc, Weak},
    str::FromStr,
};

pub struct ConditionalConfig<T: IntoProcess> {
    pub condition: Box<dyn Fn(&Factory) -> bool>,
    pub child: T,
}

pub struct ConditionalProcess<T: Process> {
    condition: Box<dyn Fn(&Factory) -> bool>,
    child: Rc<RefCell<T>>,
}

impl<T: IntoProcess> IntoProcess for ConditionalConfig<T> {
    type Output = ConditionalProcess<T::Output>;
    fn into_process(self, factory: &Factory) -> Rc<RefCell<Self::Output>> {
        Rc::new(RefCell::new(Self::Output { condition: self.condition, child: self.child.into_process(factory) }))
    }
}

impl<T: Process> Process for ConditionalProcess<T> {
    fn run(&self, factory: &Factory) -> ChildTask<Result<(), LocalStr>> {
        if (self.condition)(factory) {
            self.child.borrow().run(factory)
        } else {
            spawn(async { Ok(()) })
        }
    }
}

pub struct PlasticMixerConfig {
    pub name: LocalStr,
    pub accesses: Vec<ComponentAccess>,
    pub n_wanted: i32,
}

pub struct PlasticMixerProcess {
    weak: Weak<RefCell<PlasticMixerProcess>>,
    config: PlasticMixerConfig,
    prev_choice: Option<u8>,
}

impl IntoProcess for PlasticMixerConfig {
    type Output = PlasticMixerProcess;
    fn into_process(self, _factory: &Factory) -> Rc<RefCell<Self::Output>> {
        Rc::new_cyclic(|weak| RefCell::new(Self::Output { weak: weak.clone(), config: self, prev_choice: None }))
    }
}

#[rustfmt::skip]
const COLORS: [&'static str; 16] = [
    "Black", "Red", "Green", "Brown", "Blue", "Purple", "Cyan", "Light Gray",
    "Gray", "Pink", "Lime", "Yellow", "Light Blue", "Magenta", "Orange", "White",
];

impl Process for PlasticMixerProcess {
    fn run(&self, factory: &Factory) -> ChildTask<Result<(), LocalStr>> {
        let (i, color, n_stored) = COLORS
            .iter()
            .enumerate()
            .map(|(i, color)| (i, *color, factory.search_n_stored(&Filter::Label(LocalStr::from_static(*color)))))
            .min_by_key(|(_, _, n_stored)| *n_stored)
            .unwrap();
        let text;
        let choice;
        if n_stored < self.config.n_wanted {
            text = local_fmt!("{}: making {} Plastic", self.config.name, color);
            choice = i as u8 + 1;
        } else {
            text = local_fmt!("{}: off", self.config.name);
            choice = 0;
        }
        factory.log(Print { text, color: 0xFF4FFF, beep: None });
        if Some(choice) == self.prev_choice {
            return spawn(async { Ok(()) });
        }
        let server = factory.borrow_server();
        let access = server.load_balance(&self.config.accesses).1;
        let action = ActionFuture::from(Call { addr: access.addr.clone(), func: local_str!("selectColor"), args: vec![choice.into()] });
        server.enqueue_request_group(&access.client, vec![action.clone().into()]);
        let weak = self.weak.clone();
        spawn(async move {
            alive_mut!(weak, this);
            this.prev_choice = Some(choice);
            Ok(())
        })
    }
}

pub struct FluxNetworkOutput {
    pub accesses: Vec<SidedAccess>,
    pub output: Box<dyn Fn(f64) -> i32>,
}

pub struct FluxNetworkConfig {
    pub name: LocalStr,
    pub accesses: Vec<ComponentAccess>,
    pub outputs: Vec<FluxNetworkOutput>,
}

pub struct FluxNetworkProcess {
    weak: Weak<RefCell<FluxNetworkProcess>>,
    factory: Weak<RefCell<Factory>>,
    name: LocalStr,
    accesses: Vec<ComponentAccess>,
    outputs: Vec<Rc<RefCell<RedstoneEmitterProcess>>>,
    energy: f64,
}

impl IntoProcess for FluxNetworkConfig {
    type Output = FluxNetworkProcess;
    fn into_process(self, factory: &Factory) -> Rc<RefCell<Self::Output>> {
        Rc::new_cyclic(|weak| {
            RefCell::new(Self::Output {
                weak: weak.clone(),
                factory: factory.weak.clone(),
                name: self.name,
                accesses: self.accesses,
                outputs: Vec::from_iter(self.outputs.into_iter().map(|FluxNetworkOutput { output, accesses }| {
                    let weak = weak.clone();
                    RedstoneEmitterConfig {
                        accesses: accesses,
                        output: Box::new(move |_factory| {
                            upgrade!(weak, this);
                            output(this.energy)
                        }),
                    }
                    .into_process(factory)
                })),
                energy: f64::NAN,
            })
        })
    }
}

impl Process for FluxNetworkProcess {
    fn run(&self, factory: &Factory) -> ChildTask<Result<(), LocalStr>> {
        let server = factory.borrow_server();
        let access = server.load_balance(&self.accesses).1;
        let action = ActionFuture::from(Call { addr: access.addr.clone(), func: local_str!("getEnergyInfo"), args: Vec::new() });
        server.enqueue_request_group(&access.client, vec![action.clone().into()]);
        let weak = self.weak.clone();
        spawn(async move {
            let energy: f64 = table_remove(&mut call_result(action.await?)?, "totalEnergy")?;
            let tasks = {
                let this = alive(&weak)?;
                this.borrow_mut().energy = energy;
                let this = this.borrow();
                upgrade!(this.factory, factory);
                factory.log(Print { text: local_fmt!("{}: {:.0}", this.name, energy), color: 0xFF4FFF, beep: None });
                this.outputs.iter().map(|output| output.borrow().run(factory)).collect()
            };
            join_tasks(tasks).await
        })
    }
}

pub struct LowAlert {
    item: Filter,
    n_wanted: i32,
    log: LocalStr,
}

impl LowAlert {
    pub fn new(item: Filter, n_wanted: i32) -> Self {
        let log = match &item {
            Filter::Label(x) => x.clone(),
            Filter::Name(x) => local_fmt!("<{}>", x),
            Filter::Both { label, name } => local_fmt!("{} <{}>", label, name),
            Filter::Custom { desc, .. } => local_fmt!("<{}>", desc),
        };
        Self { item, n_wanted, log }
    }
}

impl Process for LowAlert {
    fn run(&self, factory: &Factory) -> ChildTask<Result<(), LocalStr>> {
        let n_stored = factory.search_n_stored(&self.item);
        if n_stored < self.n_wanted {
            factory.log(Print { text: local_fmt!("need {}*{}", self.log, self.n_wanted - n_stored), color: 0xF2B2CC, beep: None })
        }
        spawn(async { Ok(()) })
    }
}

pub struct FluidLowAlert(pub LocalStr, pub i64);
impl Process for FluidLowAlert {
    fn run(&self, factory: &Factory) -> ChildTask<Result<(), LocalStr>> {
        let n_stored = factory.search_n_fluid(&self.0);
        if n_stored < self.1 {
            factory.log(Print { text: local_fmt!("need {}*{}", self.0, self.1 - n_stored), color: 0xF2B2CC, beep: None })
        }
        spawn(async { Ok(()) })
    }
}

pub struct ItemCycleConfig {
    pub name: LocalStr,
    pub file_name: LocalStr,
    pub accesses: Vec<InvAccess>,
    pub slot: usize,
    pub items: Vec<ScatteringInput>,
}

pub struct ItemCycleProcess {
    weak: Weak<RefCell<ItemCycleProcess>>,
    factory: Weak<RefCell<Factory>>,
    config: ItemCycleConfig,
    next_item: usize,
}

impl_inventory!(ItemCycleProcess);

impl IntoProcess for ItemCycleConfig {
    type Output = ItemCycleProcess;
    fn into_process(self, factory: &Factory) -> Rc<RefCell<Self::Output>> {
        let next_item = read_to_string(&*self.file_name).ok().and_then(|x| usize::from_str(&x).ok()).unwrap_or_default();
        Rc::new_cyclic(|weak| RefCell::new(Self::Output { weak: weak.clone(), factory: factory.weak.clone(), config: self, next_item }))
    }
}

impl Process for ItemCycleProcess {
    fn run(&self, factory: &Factory) -> ChildTask<Result<(), LocalStr>> {
        let stacks = list_inv(self, factory);
        let weak = self.weak.clone();
        spawn(async move {
            let stacks = stacks.await?;
            let mut slot_to_free = None;
            let task = {
                alive!(weak, this);
                if this.config.slot >= stacks.len() {
                    return Err(local_fmt!("{}: invalid slot", this.config.name));
                }
                if stacks[this.config.slot].is_some() {
                    return Ok(());
                }
                upgrade_mut!(this.factory, factory);
                let input = &this.config.items[this.next_item];
                if let Some((item, info)) = factory.search_item(input.get_item()) {
                    if info.borrow().get_availability(input.get_allow_backup(), input.get_extra_backup()) < 1 {
                        return Ok(());
                    }
                    let reservation = factory.reserve_item(&this.config.name, item, 1);
                    let bus_slot = factory.bus_allocate();
                    let weak = weak.clone();
                    let slot_to_free = &mut slot_to_free;
                    async move {
                        let bus_slot = bus_slot.await?;
                        *slot_to_free = Some(bus_slot);
                        let extraction = reservation.extract(&*alive(&weak)?.borrow().factory.upgrade().unwrap().borrow(), bus_slot);
                        extraction.await?;
                        let task = {
                            alive_mut!(weak, this);
                            upgrade!(this.factory, factory);
                            let server = factory.borrow_server();
                            let access = server.load_balance(&this.config.accesses).1;
                            let action = ActionFuture::from(Call {
                                addr: access.addr.clone(),
                                func: local_str!("transferItem"),
                                args: vec![
                                    access.bus_side.into(),
                                    access.inv_side.into(),
                                    1.into(),
                                    (bus_slot + 1).into(),
                                    (this.config.slot + 1).into(),
                                ],
                            });
                            server.enqueue_request_group(&access.client, vec![action.clone().into()]);
                            action
                        };
                        task.await?;
                        alive_mut!(weak, this);
                        upgrade_mut!(this.factory, factory);
                        factory.bus_free(bus_slot);
                        *slot_to_free = None;
                        this.next_item += 1;
                        if this.next_item == this.config.items.len() {
                            this.next_item = 0
                        }
                        std::fs::write(&*this.config.file_name, this.next_item.to_string()).map_err(|e| local_fmt!("{}: {}", this.config.name, e))
                    }
                } else {
                    return Ok(());
                }
            };
            let result = task.await;
            if let Some(slot) = slot_to_free {
                alive(&weak)?.borrow().factory.upgrade().unwrap().borrow_mut().bus_deposit(once(slot))
            }
            result
        })
    }
}
