use super::super::access::ComponentAccess;
use super::super::action::{ActionFuture, Call, Print};
use super::super::factory::Factory;
use super::super::item::Filter;
use super::super::lua_value::call_result;
use super::super::util::{alive, join_outputs, spawn};
use super::{IntoProcess, Process};
use abort_on_drop::ChildTask;
use flexstr::{local_fmt, local_str, LocalStr};
use std::{
    cell::RefCell,
    future::Future,
    rc::{Rc, Weak},
};
use tokio::time::Instant;

trait ReactorProcess {
    fn get_accesses(&self) -> &Vec<ComponentAccess>;
    fn n_cyanite_wanted(&self) -> i32;
    fn has_turbine(&self) -> bool;
}

macro_rules! impl_reactor_process {
    ($i:ident) => {
        impl ReactorProcess for $i {
            fn get_accesses(&self) -> &Vec<ComponentAccess> { &self.config.accesses }
            fn n_cyanite_wanted(&self) -> i32 { self.config.n_cyanite_wanted }
            fn has_turbine(&self) -> bool { self.config.has_turbine }
        }
    };
}

fn run_reactor<T, U, F>(this: &T, factory: &Factory, run: F) -> ChildTask<Result<(), LocalStr>>
where
    T: ReactorProcess,
    U: Future<Output = Result<(), LocalStr>>,
    F: FnOnce(f64) -> U + 'static,
{
    if this.n_cyanite_wanted() > 0 && factory.search_n_stored(&Filter::Label(local_str!("Cyanite Ingot"))) < this.n_cyanite_wanted() {
        return spawn(async move { run(0.0).await });
    }
    let server = factory.borrow_server();
    if this.has_turbine() {
        let states = ["getHotFluidAmount", "getHotFluidAmountMax"]
            .into_iter()
            .map(|x| {
                let access = server.load_balance(this.get_accesses()).1;
                let action = ActionFuture::from(Call { addr: access.addr.clone(), func: LocalStr::from_static(x), args: Vec::new() });
                server.enqueue_request_group(&access.client, vec![action.clone().into()]);
                spawn(async move { call_result::<f64>(action.await?) })
            })
            .collect();
        spawn(async move { run(join_outputs(states).await.map(|states| states[0] / states[1])?).await })
    } else {
        let access = server.load_balance(this.get_accesses()).1;
        let action = ActionFuture::from(Call { addr: access.addr.clone(), func: local_str!("getEnergyStored"), args: Vec::new() });
        server.enqueue_request_group(&access.client, vec![action.clone().into()]);
        spawn(async move { run(action.await.and_then(call_result).map(|x: f64| x / 1E7)?).await })
    }
}

pub struct HysteresisReactorConfig {
    pub name: LocalStr,
    pub accesses: Vec<ComponentAccess>,
    pub n_cyanite_wanted: i32,
    pub has_turbine: bool,
    pub lower_bound: f64, // typical: 0.3
    pub upper_bound: f64, // typical: 0.7
}

pub struct HysteresisReactorProcess {
    weak: Weak<RefCell<HysteresisReactorProcess>>,
    config: HysteresisReactorConfig,
    factory: Weak<RefCell<Factory>>,
    prev_on: Option<bool>,
}

impl_reactor_process!(HysteresisReactorProcess);

impl IntoProcess for HysteresisReactorConfig {
    type Output = HysteresisReactorProcess;
    fn into_process(self, factory: &Factory) -> Rc<RefCell<Self::Output>> {
        Rc::new_cyclic(|weak| RefCell::new(Self::Output { weak: weak.clone(), config: self, factory: factory.weak.clone(), prev_on: None }))
    }
}

impl Process for HysteresisReactorProcess {
    fn run(&self, factory: &Factory) -> ChildTask<Result<(), LocalStr>> {
        let weak = self.weak.clone();
        run_reactor(self, factory, |pv| async move {
            let action;
            let on;
            {
                alive!(weak, this);
                on = if pv < this.config.lower_bound {
                    true
                } else if pv > this.config.upper_bound {
                    false
                } else {
                    return Ok(());
                };
                if this.prev_on == Some(on) {
                    return Ok(());
                }
                upgrade!(this.factory, factory);
                factory.log(Print { text: local_fmt!("{}: {}", this.config.name, if on { "on" } else { "off" }), color: 0xFF4FFF, beep: None });
                let server = factory.borrow_server();
                let access = server.load_balance(&this.config.accesses).1;
                action = ActionFuture::from(Call { addr: access.addr.clone(), func: local_str!("setActive"), args: vec![on.into()] });
                server.enqueue_request_group(&access.client, vec![action.clone().into()]);
            };
            action.await?;
            alive(&weak)?.borrow_mut().prev_on = Some(on);
            Ok(())
        })
    }
}

pub struct ProportionalReactorConfig {
    pub name: LocalStr,
    pub accesses: Vec<ComponentAccess>,
    pub n_cyanite_wanted: i32,
    pub has_turbine: bool,
}

pub struct ProportionalReactorProcess {
    weak: Weak<RefCell<ProportionalReactorProcess>>,
    config: ProportionalReactorConfig,
    factory: Weak<RefCell<Factory>>,
    prev_rod: Option<i16>,
}

impl_reactor_process!(ProportionalReactorProcess);

impl IntoProcess for ProportionalReactorConfig {
    type Output = ProportionalReactorProcess;
    fn into_process(self, factory: &Factory) -> Rc<RefCell<Self::Output>> {
        Rc::new_cyclic(|weak| RefCell::new(Self::Output { weak: weak.clone(), config: self, factory: factory.weak.clone(), prev_rod: None }))
    }
}

fn to_percent(x: f64) -> i16 { (x * 100.0).round() as _ }

impl Process for ProportionalReactorProcess {
    fn run(&self, factory: &Factory) -> ChildTask<Result<(), LocalStr>> {
        let weak = self.weak.clone();
        run_reactor(self, factory, |pv| async move {
            let rod = to_percent(pv);
            let action;
            {
                alive!(weak, this);
                upgrade!(this.factory, factory);
                factory.log(Print { text: local_fmt!("{}: {}%", this.config.name, rod), color: 0xFF4FFF, beep: None });
                if this.prev_rod == Some(rod) {
                    return Ok(());
                }
                let server = factory.borrow_server();
                let access = server.load_balance(&this.config.accesses).1;
                action = ActionFuture::from(Call { addr: access.addr.clone(), func: local_str!("setAllControlRodLevels"), args: vec![rod.into()] });
                server.enqueue_request_group(&access.client, vec![action.clone().into()]);
            };
            action.await?;
            alive(&weak)?.borrow_mut().prev_rod = Some(rod);
            Ok(())
        })
    }
}

pub struct PIDReactorConfig {
    pub name: LocalStr,
    pub accesses: Vec<ComponentAccess>,
    pub n_cyanite_wanted: i32,
    pub has_turbine: bool,
    pub k_p: f64, // typical: 1.00
    pub k_i: f64, // typical: 0.01
    pub k_d: f64, // typical: 0.00
}

struct PIDReactorState {
    prev_t: Instant,
    prev_e: f64,
    accum: f64,
}

pub struct PIDReactorProcess {
    weak: Weak<RefCell<PIDReactorProcess>>,
    config: PIDReactorConfig,
    factory: Weak<RefCell<Factory>>,
    state: Option<PIDReactorState>,
    prev_rod: Option<i16>,
}

impl_reactor_process!(PIDReactorProcess);

impl IntoProcess for PIDReactorConfig {
    type Output = PIDReactorProcess;
    fn into_process(self, factory: &Factory) -> Rc<RefCell<Self::Output>> {
        Rc::new_cyclic(|weak| {
            RefCell::new(Self::Output { weak: weak.clone(), config: self, factory: factory.weak.clone(), state: None, prev_rod: None })
        })
    }
}

impl Process for PIDReactorProcess {
    fn run(&self, factory: &Factory) -> ChildTask<Result<(), LocalStr>> {
        let weak = self.weak.clone();
        run_reactor(self, factory, |pv| async move {
            let rod;
            let action;
            {
                alive_mut!(weak, this);
                let t = Instant::now();
                let e = (0.5 - pv) * 2.0;
                let mut accum = 0.0;
                let mut diff = 0.0;
                if let Some(ref state) = this.state {
                    let dt = (t - state.prev_t).as_secs_f64();
                    accum = (state.accum + dt * e * this.config.k_i).clamp(-1.0, 1.0);
                    diff = (e - state.prev_e) / dt
                }
                this.state = Some(PIDReactorState { prev_t: t, prev_e: e, accum });
                let op = e * this.config.k_p + accum + diff * this.config.k_d;
                rod = to_percent((0.5 - op).clamp(0.0, 1.0));
                upgrade!(this.factory, factory);
                factory.log(Print {
                    text: local_fmt!("{}: E={}%, I={}%, O={}%", this.config.name, to_percent(-e), to_percent(accum), 100 - rod),
                    color: 0xFF4FFF,
                    beep: None,
                });
                if this.prev_rod == Some(rod) {
                    return Ok(());
                }
                let server = factory.borrow_server();
                let access = server.load_balance(&this.config.accesses).1;
                action = ActionFuture::from(Call { addr: access.addr.clone(), func: local_str!("setAllControlRodLevels"), args: vec![rod.into()] });
                server.enqueue_request_group(&access.client, vec![action.clone().into()]);
            };
            action.await?;
            alive(&weak)?.borrow_mut().prev_rod = Some(rod);
            Ok(())
        })
    }
}
