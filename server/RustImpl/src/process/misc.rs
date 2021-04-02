use super::super::access::{ComponentAccess, SidedAccess};
use super::super::action::{ActionFuture, Call, Print};
use super::super::factory::Factory;
use super::super::item::Filter;
use super::super::lua_value::{call_result, Table};
use super::super::util::{alive, join_tasks, spawn, AbortOnDrop};
use super::{IntoProcess, Process, RedstoneEmitterConfig, RedstoneEmitterProcess};
use std::{
    cell::RefCell,
    convert::TryInto,
    rc::{Rc, Weak},
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
    fn into_process(self, factory: &Weak<RefCell<Factory>>) -> Rc<RefCell<Self::Output>> {
        Rc::new(RefCell::new(Self::Output { condition: self.condition, child: self.child.into_process(factory) }))
    }
}

impl<T: Process> Process for ConditionalProcess<T> {
    fn run(&self, factory: &Factory) -> AbortOnDrop<Result<(), String>> {
        if (self.condition)(factory) {
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
            .map(|(i, color)| (i, *color, factory.search_n_stored(&Filter::Label(*color))))
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
    pub output: Box<dyn Fn(f64) -> i32>,
}

pub struct FluxNetworkConfig {
    pub name: &'static str,
    pub accesses: Vec<ComponentAccess>,
    pub outputs: Vec<FluxNetworkOutput>,
}

pub struct FluxNetworkProcess {
    weak: Weak<RefCell<FluxNetworkProcess>>,
    factory: Weak<RefCell<Factory>>,
    name: &'static str,
    accesses: Vec<ComponentAccess>,
    outputs: Vec<Rc<RefCell<RedstoneEmitterProcess>>>,
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
                    .map(|FluxNetworkOutput { output, accesses }| {
                        let weak = weak.clone();
                        RedstoneEmitterConfig {
                            accesses: accesses,
                            output: Box::new(move |_factory| {
                                upgrade!(weak, this);
                                output(this.energy)
                            }),
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
