use super::super::access::{ComponentAccess, SidedAccess};
use super::super::action::{ActionFuture, Call, Print};
use super::super::factory::Factory;
use super::super::item::Filter;
use super::super::lua_value::{call_result, Table};
use super::super::util::{alive, join_tasks, spawn, AbortOnDrop};
use super::{IntoProcess, Process, RedstoneEmitterConfig, RedstoneEmitterProcess, RedstoneOutput};
use std::{
    cell::RefCell,
    convert::TryInto,
    rc::{Rc, Weak},
};

pub trait Condition {
    fn should_run(&self, factory: &Factory) -> bool;
}

impl<T: Fn(&Factory) -> bool> Condition for T {
    fn should_run(&self, factory: &Factory) -> bool { self(factory) }
}

pub struct ConditionalConfig<T, U> {
    pub condition: T,
    pub child: U,
}

pub struct ConditionalProcess<T, U> {
    condition: T,
    child: Rc<RefCell<U>>,
}

impl<T: Condition + 'static, U: IntoProcess> IntoProcess for ConditionalConfig<T, U> {
    type Output = ConditionalProcess<T, U::Output>;
    fn into_process(self, factory: &Weak<RefCell<Factory>>) -> Rc<RefCell<Self::Output>> {
        Rc::new(RefCell::new(Self::Output { condition: self.condition, child: self.child.into_process(factory) }))
    }
}

impl<T: Condition, U: Process> Process for ConditionalProcess<T, U> {
    fn run(&self, factory: &Factory) -> AbortOnDrop<Result<(), String>> {
        if self.condition.should_run(factory) {
            self.child.borrow().run(factory)
        } else {
            spawn(async { Ok(()) })
        }
    }
}

pub struct PlasticMixerConfig {
    pub name: &'static str,
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
    fn into_process(self, _factory: &Weak<RefCell<Factory>>) -> Rc<RefCell<Self::Output>> {
        Rc::new_cyclic(|weak| RefCell::new(Self::Output { weak: weak.clone(), config: self, prev_choice: None }))
    }
}

#[rustfmt::skip]
const COLORS: [&'static str; 16] = [
    "Black", "Red", "Green", "Brown", "Blue", "Purple", "Cyan", "Light Gray",
    "Gray", "Pink", "Lime", "Yellow", "Light Blue", "Magenta", "Orange", "White",
];

impl Process for PlasticMixerProcess {
    fn run(&self, factory: &Factory) -> AbortOnDrop<Result<(), String>> {
        let (i, color, n_stored) = COLORS
            .iter()
            .enumerate()
            .map(|(i, color)| {
                (i, *color, factory.search_item(&Filter::Label(*color)).map_or(0, |(_, info)| info.borrow().n_stored))
            })
            .min_by_key(|(_, _, n_stored)| *n_stored)
            .unwrap();
        let text;
        let choice;
        if n_stored < self.config.n_wanted {
            text = format!("{}: making {} Plastic", self.config.name, color);
            choice = i as u8 + 1;
        } else {
            text = format!("{}: off", self.config.name);
            choice = 0;
        }
        factory.log(Print { text, color: 0xFF4FFF, beep: None });
        if Some(choice) == self.prev_choice {
            return spawn(async { Ok(()) });
        }
        let server = factory.borrow_server();
        let access = server.load_balance(&self.config.accesses).1;
        let action = ActionFuture::from(Call { addr: access.addr, func: "selectColor", args: vec![choice.into()] });
        server.enqueue_request_group(access.client, vec![action.clone().into()]);
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
    pub value: Box<dyn Fn(&Factory, f64) -> i32>,
}

pub struct FluxNetworkConfig {
    pub name: &'static str,
    pub accesses: Vec<ComponentAccess>,
    pub outputs: Vec<FluxNetworkOutput>,
}

struct FluxNetworkRedstoneOutput {
    weak: Weak<RefCell<FluxNetworkProcess>>,
    value: Box<dyn Fn(&Factory, f64) -> i32>,
}

impl RedstoneOutput for FluxNetworkRedstoneOutput {
    fn get_value(&self, factory: &Factory) -> i32 {
        upgrade!(self.weak, this);
        (self.value)(factory, this.energy)
    }
}

pub struct FluxNetworkProcess {
    weak: Weak<RefCell<FluxNetworkProcess>>,
    factory: Weak<RefCell<Factory>>,
    name: &'static str,
    accesses: Vec<ComponentAccess>,
    outputs: Vec<Rc<RefCell<RedstoneEmitterProcess<FluxNetworkRedstoneOutput>>>>,
    energy: f64,
}

impl IntoProcess for FluxNetworkConfig {
    type Output = FluxNetworkProcess;
    fn into_process(self, factory: &Weak<RefCell<Factory>>) -> Rc<RefCell<Self::Output>> {
        Rc::new_cyclic(|weak| {
            RefCell::new(Self::Output {
                weak: weak.clone(),
                factory: factory.clone(),
                name: self.name,
                accesses: self.accesses,
                outputs: self
                    .outputs
                    .into_iter()
                    .map(|output| {
                        RedstoneEmitterConfig {
                            accesses: output.accesses,
                            output: FluxNetworkRedstoneOutput { weak: weak.clone(), value: output.value },
                        }
                        .into_process(factory)
                    })
                    .collect(),
                energy: f64::NAN,
            })
        })
    }
}

impl Process for FluxNetworkProcess {
    fn run(&self, factory: &Factory) -> AbortOnDrop<Result<(), String>> {
        let server = factory.borrow_server();
        let access = server.load_balance(&self.accesses).1;
        let action = ActionFuture::from(Call { addr: access.addr, func: "getEnergyInfo", args: Vec::new() });
        server.enqueue_request_group(access.client, vec![action.clone().into()]);
        let weak = self.weak.clone();
        spawn(async move {
            let mut energy: Table = call_result::<Table>(action.await?)?;
            let energy: f64 = energy
                .remove(&"totalEnergy".into())
                .ok_or_else(|| format!("invalid energy info: {:?}", energy))?
                .try_into()?;
            let tasks = {
                let this = alive(&weak)?;
                this.borrow_mut().energy = energy;
                let this = this.borrow();
                upgrade!(this.factory, factory);
                factory.log(Print { text: format!("{}: {:.0}", this.name, energy), color: 0xFF4FFF, beep: None });
                this.outputs.iter().map(|output| output.borrow().run(factory)).collect()
            };
            join_tasks(tasks).await
        })
    }
}
