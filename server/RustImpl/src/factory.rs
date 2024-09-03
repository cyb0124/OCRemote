use crate::access::{Access, EachTank, FluidAccess, SidedAccess, TankAccess};
use crate::action::{ActionFuture, Call, List, Print};
use crate::item::{Filter, Item, ItemStack};
use crate::lua_value::{call_result, table_remove, table_to_vec, Table};
use crate::process::{IntoProcess, Process};
use crate::server::Server;
use crate::storage::{DepositResult, Extractor, IntoStorage, Provider, Storage};
use crate::util::{alive, join_outputs, join_tasks, make_local_one_shot, spawn, LocalReceiver, LocalSender};
use crate::Tui;
use abort_on_drop::ChildTask;
use flexstr::{local_fmt, local_str, LocalStr};
use fnv::{FnvHashMap, FnvHashSet};
use std::{
    cell::{Ref, RefCell},
    cmp::{max, min},
    collections::{hash_map::Entry, BinaryHeap, VecDeque},
    future::Future,
    mem::take,
    rc::{Rc, Weak},
    time::Duration,
};
use tokio::time::{sleep_until, Instant};

pub struct ItemInfo {
    pub n_stored: i32,
    n_backup: i32,
    providers: BinaryHeap<Provider>,
}

impl ItemInfo {
    pub fn provide(&mut self, provider: Provider) {
        let n_provided = provider.n_provided.get();
        if n_provided > 0 {
            self.n_stored += n_provided;
            self.providers.push(provider)
        }
    }

    pub fn get_availability(&self, allow_backup: bool, extra_backup: i32) -> i32 {
        let mut result = self.n_stored - extra_backup;
        if !allow_backup {
            result -= self.n_backup;
        }
        max(0, result)
    }

    fn reserve(&mut self, mut size: i32) -> Reservation {
        let mut extractors = Vec::new();
        while size > 0 {
            let best = self.providers.peek().unwrap();
            let mut n_provided = best.n_provided.get();
            let to_reserve = min(size, n_provided);
            extractors.push((best.extractor.clone(), to_reserve));
            self.n_stored -= to_reserve;
            n_provided -= to_reserve;
            size -= to_reserve;
            if n_provided <= 0 {
                self.providers.pop();
            } else {
                best.n_provided.set(n_provided);
            }
        }
        Reservation { extractors }
    }
}

pub struct Reservation {
    extractors: Vec<(Rc<dyn Extractor>, i32)>,
}

impl Reservation {
    pub fn extract(self, factory: &Factory, bus_slot: usize) -> impl Future<Output = Result<(), LocalStr>> {
        let tasks = self.extractors.into_iter().map(|(extractor, size)| extractor.extract(factory, size, bus_slot)).collect();
        join_tasks(tasks)
    }
}

pub struct FluidReservation {
    extractors: Vec<(Weak<RefCell<FluidStorage>>, i64)>,
}

impl FluidReservation {
    pub fn extract(self, bus: usize) -> impl Future<Output = Result<(), LocalStr>> {
        join_tasks(Vec::from_iter(self.extractors.into_iter().map(|(storage, qty)| {
            spawn(async move {
                let task;
                {
                    alive!(storage, storage);
                    upgrade!(storage.factory, factory);
                    let server = factory.config.server.borrow();
                    let access = server.load_balance(&storage.config.accesses).1;
                    let bus_of_tank = &access.buses[bus];
                    task = ActionFuture::from(Call {
                        addr: bus_of_tank.addr.clone(),
                        func: local_str!("transferFluid"),
                        args: vec![bus_of_tank.tank_side.into(), bus_of_tank.bus_side.into(), qty.into(), (storage.slot.unwrap() + 1).into()],
                    });
                    server.enqueue_request_group(&access.client, vec![task.clone().into()])
                }
                task.await?;
                alive_mut!(storage, storage);
                Ok(storage.n_stored_hi -= qty)
            })
        })))
    }
}

pub struct FactoryConfig {
    pub tui: Rc<Tui>,
    pub server: Rc<RefCell<Server>>,
    pub min_cycle_time: Duration,
    pub log_clients: Vec<LocalStr>,
    pub bus_accesses: Vec<SidedAccess>,
    pub fluid_bus_accesses: Vec<FluidAccess>,
    pub fluid_bus_capacity: i64,
    pub backups: Vec<(Filter, i32)>,
    pub fluid_backups: Vec<(LocalStr, i64)>,
}

pub struct FluidStorageConfig {
    pub accesses: Vec<TankAccess>,
    pub fluid: LocalStr,
}

struct FluidStorage {
    weak: Weak<RefCell<FluidStorage>>,
    factory: Weak<RefCell<Factory>>,
    config: FluidStorageConfig,
    slot: Option<usize>,
    capacity: i64,
    n_stored_hi: i64,
    n_stored_lo: i64,
}

pub struct Factory {
    pub weak: Weak<RefCell<Factory>>,
    _task: ChildTask<Result<(), LocalStr>>,
    pub config: FactoryConfig,
    storages: Vec<Rc<RefCell<dyn Storage>>>,
    processes: Vec<Rc<RefCell<dyn Process>>>,
    fluid_storages: Vec<Rc<RefCell<FluidStorage>>>,

    pub items: FnvHashMap<Rc<Item>, RefCell<ItemInfo>>,
    label_map: FnvHashMap<LocalStr, Vec<Rc<Item>>>,
    name_map: FnvHashMap<LocalStr, Vec<Rc<Item>>>,
    fluid_backups: FnvHashMap<LocalStr, i64>,

    bus_task: Option<ChildTask<Result<(), LocalStr>>>,
    bus_allocations: FnvHashSet<usize>,
    bus_wait_queue: VecDeque<LocalSender<usize>>,
    bus_free_queue: Vec<usize>,
    n_bus_updates: usize,

    fluid_bus_task: Option<ChildTask<Result<(), LocalStr>>>,
    fluid_bus_allocations: FnvHashSet<usize>,
    fluid_bus_wait_queue: VecDeque<LocalSender<usize>>,
    fluid_bus_free_queue: Vec<usize>,
    n_fluid_bus_updates: usize,
}

impl FactoryConfig {
    pub fn build(self, builder: impl FnOnce(&mut Factory)) -> Rc<RefCell<Factory>> {
        let mut fluid_backups = FnvHashMap::default();
        for (fluid, qty) in &self.fluid_backups {
            *fluid_backups.entry(fluid.clone()).or_default() += qty
        }
        Rc::new_cyclic(|weak| {
            let mut factory = Factory {
                weak: weak.clone(),
                _task: spawn(factory_main(weak.clone())),
                config: self,
                storages: Vec::new(),
                processes: Vec::new(),
                fluid_storages: Vec::new(),

                items: FnvHashMap::default(),
                label_map: FnvHashMap::default(),
                name_map: FnvHashMap::default(),
                fluid_backups,

                bus_task: None,
                bus_allocations: FnvHashSet::default(),
                bus_wait_queue: VecDeque::new(),
                bus_free_queue: Vec::new(),
                n_bus_updates: 0,

                fluid_bus_task: None,
                fluid_bus_allocations: FnvHashSet::default(),
                fluid_bus_wait_queue: VecDeque::new(),
                fluid_bus_free_queue: Vec::new(),
                n_fluid_bus_updates: 0,
            };
            builder(&mut factory);
            RefCell::new(factory)
        })
    }
}

impl Factory {
    pub fn add_storage(&mut self, storage: impl IntoStorage) { self.storages.push(storage.into_storage(&self)) }
    pub fn add_process(&mut self, process: impl IntoProcess) { self.processes.push(process.into_process(&self)) }
    pub fn get_n_stored(&self, item: &Rc<Item>) -> i32 { self.items.get(item).map_or(0, |info| info.borrow().n_stored) }
    pub fn borrow_server(&self) -> Ref<Server> { self.config.server.borrow() }
    pub fn add_fluid_storage(&mut self, config: FluidStorageConfig) {
        self.fluid_storages.push(Rc::new_cyclic(|weak| {
            RefCell::new(FluidStorage {
                weak: weak.clone(),
                factory: self.weak.clone(),
                config,
                slot: None,
                capacity: 0,
                n_stored_hi: 0,
                n_stored_lo: 0,
            })
        }))
    }

    pub fn log(&self, action: Print) {
        self.config.tui.log(action.text.to_std_string(), action.color);
        let server = self.borrow_server();
        for client in &self.config.log_clients {
            server.enqueue_request_group(client, vec![ActionFuture::from(action.clone()).into()]);
        }
    }

    pub fn register_stored_item(&mut self, item: Rc<Item>) -> &mut ItemInfo {
        match self.items.entry(item) {
            Entry::Occupied(x) => x.into_mut().get_mut(),
            Entry::Vacant(x) => {
                let item = x.key();
                self.label_map.entry(item.label.clone()).or_default().push(item.clone());
                self.name_map.entry(item.name.clone()).or_default().push(item.clone());
                x.insert(RefCell::new(ItemInfo { n_stored: 0, n_backup: 0, providers: BinaryHeap::new() })).get_mut()
            }
        }
    }

    pub fn search_item<'a>(&'a self, filter: &Filter) -> Option<(&'a Rc<Item>, &'a RefCell<ItemInfo>)> {
        let mut best: Option<(&'a Rc<Item>, &'a RefCell<ItemInfo>)> = None;
        let mut on_candidate = |(new_item, new_info): (&'a Rc<Item>, &'a RefCell<ItemInfo>)| {
            if let Some((_, old_info)) = best {
                if new_info.borrow().n_stored <= old_info.borrow().n_stored {
                    return;
                }
            }
            best = Some((new_item, new_info))
        };
        match filter {
            Filter::Label(label) => {
                if let Some(items) = self.label_map.get(&*label) {
                    for item in items {
                        on_candidate(self.items.get_key_value(item).unwrap())
                    }
                }
            }
            Filter::Name(name) => {
                if let Some(items) = self.name_map.get(&*name) {
                    for item in items {
                        on_candidate(self.items.get_key_value(item).unwrap())
                    }
                }
            }
            Filter::Both { label, name } => {
                if let Some(items) = self.label_map.get(&*label) {
                    for item in items {
                        if item.name == *name {
                            on_candidate(self.items.get_key_value(item).unwrap())
                        }
                    }
                }
            }
            Filter::Custom { func, .. } => {
                for (item, info) in &self.items {
                    if func(item) {
                        on_candidate((item, info))
                    }
                }
            }
        }
        best
    }

    pub fn search_n_stored(&self, filter: &Filter) -> i32 { self.search_item(filter).map_or(0, |(_, info)| info.borrow().n_stored) }

    pub fn bus_allocate(&mut self) -> LocalReceiver<usize> {
        let (sender, receiver) = make_local_one_shot();
        self.bus_wait_queue.push_back(sender);
        if self.bus_task.is_none() {
            self.bus_task = Some(spawn(bus_main(self.weak.clone())))
        }
        receiver
    }

    pub fn bus_free(&mut self, slot: usize) {
        if let Some(state) = self.bus_wait_queue.pop_front() {
            state.send(Ok(slot))
        } else {
            self.bus_allocations.remove(&slot);
        }
    }

    pub fn bus_deposit(&mut self, slots: impl IntoIterator<Item = usize>) {
        if self.bus_task.is_none() {
            let mut ever_freed = false;
            for slot in slots {
                self.bus_allocations.remove(&slot);
                ever_freed = true
            }
            if ever_freed {
                self.bus_task = Some(spawn(bus_main(self.weak.clone())))
            }
        } else {
            self.bus_free_queue.extend(slots)
        }
    }

    fn deposit_item(&self, bus_slot: usize, mut stack: ItemStack, tasks: &mut Vec<ChildTask<Result<(), LocalStr>>>) {
        self.log(Print { text: local_fmt!("{}*{}", stack.item.label, stack.size), color: 0xFFA500, beep: None });
        while stack.size > 0 {
            let mut best: Option<(&Rc<RefCell<dyn Storage>>, i32)> = None;
            for storage in &self.storages {
                let Some(prio) = storage.borrow_mut().deposit_priority(&stack.item) else { continue };
                if best.as_ref().map_or(true, |&(_, best)| prio > best) {
                    best = Some((storage, prio))
                }
            }
            if let Some((storage, _)) = best {
                let DepositResult { n_deposited, task } = storage.borrow_mut().deposit(self, &stack, bus_slot);
                stack.size -= n_deposited;
                tasks.push(task)
            } else {
                tasks.push(spawn(async { Err(local_str!("storage is full")) }));
                break;
            }
        }
    }

    pub fn reserve_item(&self, reason: &str, item: &Rc<Item>, size: i32) -> Reservation {
        self.log(Print { text: local_fmt!("{reason}: {}*{size}", item.label), color: 0x55ABEC, beep: None });
        self.items.get(item).unwrap().borrow_mut().reserve(size)
    }

    pub fn search_n_fluid(&self, fluid: &str) -> i64 {
        let mut sum = 0;
        for storage in &self.fluid_storages {
            let storage = storage.borrow();
            if storage.config.fluid == fluid {
                sum += storage.n_stored_lo
            }
        }
        sum
    }

    pub fn get_fluid_availability(&self, fluid: &str, allow_backup: bool, extra_backup: i64) -> i64 {
        let mut n_available = self.search_n_fluid(fluid) - extra_backup;
        if !allow_backup {
            n_available -= self.fluid_backups.get(fluid).copied().unwrap_or_default()
        }
        n_available.max(0)
    }

    pub fn fluid_bus_allocate(&mut self) -> LocalReceiver<usize> {
        let (sender, receiver) = make_local_one_shot();
        self.fluid_bus_wait_queue.push_back(sender);
        if self.fluid_bus_task.is_none() {
            self.fluid_bus_task = Some(spawn(fluid_bus_main(self.weak.clone())))
        }
        receiver
    }

    pub fn fluid_bus_free(&mut self, bus: usize) {
        if let Some(state) = self.fluid_bus_wait_queue.pop_front() {
            state.send(Ok(bus))
        } else {
            self.fluid_bus_allocations.remove(&bus);
        }
    }

    pub fn fluid_bus_deposit(&mut self, buses: impl IntoIterator<Item = usize>) {
        if self.fluid_bus_task.is_none() {
            let mut ever_freed = false;
            for bus in buses {
                self.fluid_bus_allocations.remove(&bus);
                ever_freed = true
            }
            if ever_freed {
                self.fluid_bus_task = Some(spawn(fluid_bus_main(self.weak.clone())))
            }
        } else {
            self.fluid_bus_free_queue.extend(buses)
        }
    }

    fn fluid_deposit(&self, bus: usize, slot: usize, fluid: LocalStr, mut qty: i64, tasks: &mut Vec<ChildTask<Result<(), LocalStr>>>) {
        self.log(Print { text: local_fmt!("{fluid}*{qty}"), color: 0xFFA500, beep: None });
        let server = self.config.server.borrow();
        while qty > 0 {
            let mut best: Option<(&Rc<RefCell<FluidStorage>>, i64)> = None;
            for storage in &self.fluid_storages {
                let sto = storage.borrow();
                if sto.config.fluid == fluid && sto.n_stored_hi < sto.capacity && best.as_ref().map_or(true, |&(_, best)| sto.n_stored_hi > best) {
                    best = Some((storage, sto.n_stored_hi))
                }
            }
            if let Some((storage, _)) = best {
                let mut sto = storage.borrow_mut();
                let n_deposited = qty.min(sto.capacity - sto.n_stored_hi);
                sto.n_stored_hi += n_deposited;
                qty -= n_deposited;
                let access = server.load_balance(&sto.config.accesses).1;
                let bus_of_tank = &access.buses[bus];
                let task = ActionFuture::from(Call {
                    addr: bus_of_tank.addr.clone(),
                    func: local_str!("transferFluid"),
                    args: vec![bus_of_tank.bus_side.into(), bus_of_tank.tank_side.into(), n_deposited.into(), (slot + 1).into()],
                });
                server.enqueue_request_group(&access.client, vec![task.clone().into()]);
                tasks.push(spawn(async move { task.await.map(|_| ()) }))
            } else {
                tasks.push(spawn(async move { Err(local_fmt!("{fluid} is full")) }));
                break;
            }
        }
    }

    pub fn reserve_fluid(&self, reason: &str, fluid: &str, mut qty: i64) -> FluidReservation {
        self.log(Print { text: local_fmt!("{reason}: {fluid}*{qty}"), color: 0x55ABEC, beep: None });
        let mut extractors = Vec::new();
        while qty > 0 {
            let mut best = None;
            for storage in &self.fluid_storages {
                let sto = storage.borrow();
                if sto.config.fluid == fluid && sto.n_stored_lo > 0 && best.as_ref().map_or(true, |&(_, best)| sto.n_stored_lo < best) {
                    best = Some((storage.clone(), sto.n_stored_lo))
                }
            }
            let (storage, _) = best.unwrap();
            let mut storage = storage.borrow_mut();
            let to_reserve = qty.min(storage.n_stored_lo);
            storage.n_stored_lo -= to_reserve;
            qty -= to_reserve;
            extractors.push((storage.weak.clone(), to_reserve))
        }
        FluidReservation { extractors }
    }

    fn end_of_cycle(&mut self) {
        for storage in &self.storages {
            storage.borrow_mut().cleanup()
        }
        for storage in &self.fluid_storages {
            let mut storage = storage.borrow_mut();
            storage.slot = None;
            storage.capacity = 0;
            storage.n_stored_hi = 0;
            storage.n_stored_lo = 0
        }
        self.items.clear();
        self.label_map.clear();
        self.name_map.clear();
    }
}

async fn factory_main(factory: Weak<RefCell<Factory>>) -> Result<(), LocalStr> {
    let mut cycle_start_last: Option<Instant> = None;
    let mut n_cycles: usize = 0;
    loop {
        let cycle_start_time = Instant::now();
        {
            alive_mut!(factory, this);
            let text = if let Some(last) = cycle_start_last {
                local_fmt!(
                    "OCRemote #{}, nBusUpdates={},{}, cycleTime={:.3}",
                    n_cycles,
                    this.n_bus_updates,
                    this.n_fluid_bus_updates,
                    (cycle_start_time - last).as_secs_f64()
                )
            } else {
                local_str!("OCRemote started")
            };
            this.log(Print { text, color: 0xFFFFFF, beep: None });
            this.n_bus_updates = 0;
            this.n_fluid_bus_updates = 0
        }
        let result = async {
            update_storages(&factory).await?;
            run_processes(&factory).await
        }
        .await;
        let mut bus_task;
        let mut fluid_bus_task;
        {
            alive_mut!(factory, this);
            bus_task = this.bus_task.take();
            fluid_bus_task = this.fluid_bus_task.take();
            if let Err(e) = result {
                this.log(Print { text: local_fmt!("cycle failed: {}", e), color: 0xFF0000, beep: Some(880.0) })
            } else {
                n_cycles += 1;
                if bus_task.is_none() && this.n_bus_updates == 0 {
                    bus_task = Some(spawn(bus_main(factory.clone())))
                }
                if fluid_bus_task.is_none() && this.n_fluid_bus_updates == 0 {
                    fluid_bus_task = Some(spawn(fluid_bus_main(factory.clone())))
                }
            }
        }
        for task in [bus_task, fluid_bus_task].into_iter().flatten() {
            task.await.unwrap()?
        }
        let min_cycle_time = {
            alive_mut!(factory, this);
            this.end_of_cycle();
            this.config.min_cycle_time
        };
        sleep_until(cycle_start_time + min_cycle_time).await;
        cycle_start_last = Some(cycle_start_time)
    }
}

async fn update_storages(factory: &Weak<RefCell<Factory>>) -> Result<(), LocalStr> {
    let mut tasks = Vec::new();
    {
        alive!(factory, this);
        tasks.extend(this.storages.iter().map(|storage| storage.borrow().update(this)));
        tasks.extend(this.fluid_storages.iter().map(|storage| storage.borrow().update()))
    };
    join_tasks(tasks).await?;
    alive!(factory, this);
    let mut n_total = 0;
    for (_, item) in &this.items {
        n_total += item.borrow().n_stored
    }
    this.log(Print { text: local_fmt!("storage: {} items, {} types", n_total, this.items.len()), color: 0x00FF00, beep: None });
    for (filter, n_backup) in &this.config.backups {
        if let Some((_, info)) = this.search_item(filter) {
            info.borrow_mut().n_backup += n_backup
        }
    }
    Ok(())
}

async fn run_processes(factory: &Weak<RefCell<Factory>>) -> Result<(), LocalStr> {
    let tasks = {
        alive!(factory, this);
        this.processes.iter().map(|process| process.borrow().run(this)).collect()
    };
    join_tasks(tasks).await
}

async fn bus_main(factory: Weak<RefCell<Factory>>) -> Result<(), LocalStr> {
    loop {
        let result = bus_update(&factory).await;
        alive_mut!(factory, this);
        match result {
            Err(e) => {
                let text = local_fmt!("bus update failed: {}", e);
                for sender in take(&mut this.bus_wait_queue) {
                    sender.send(Err(text.clone()))
                }
                this.log(Print { text, color: 0xFF0000, beep: Some(880.0) });
            }
            Ok(true) => continue,
            Ok(false) => (),
        }
        this.bus_task = None;
        break Ok(());
    }
}

async fn bus_update(factory: &Weak<RefCell<Factory>>) -> Result<bool, LocalStr> {
    let action;
    {
        alive_mut!(factory, this);
        this.n_bus_updates += 1;
        let server = this.borrow_server();
        let access = server.load_balance(&this.config.bus_accesses).1;
        action = ActionFuture::from(List { addr: access.addr.clone(), side: access.side });
        server.enqueue_request_group(&access.client, vec![action.clone().into()]);
    }
    let stacks = action.await?;
    let mut tasks = Vec::new();
    {
        alive_mut!(factory, this);
        let mut free_slots = Vec::new();
        for (slot, stack) in stacks.into_iter().enumerate() {
            if !this.bus_allocations.contains(&slot) {
                if let Some(stack) = stack {
                    this.deposit_item(slot, stack, &mut tasks);
                } else {
                    free_slots.push(slot)
                }
            }
        }
        while !free_slots.is_empty() && !this.bus_wait_queue.is_empty() {
            let slot = free_slots.pop().unwrap();
            this.bus_allocations.insert(slot);
            this.bus_wait_queue.pop_front().unwrap().send(Ok(slot))
        }
    }
    let ever_deposited = !tasks.is_empty();
    join_tasks(tasks).await?;
    alive_mut!(factory, this);
    let mut ever_freed = false;
    for slot in take(&mut this.bus_free_queue) {
        this.bus_allocations.remove(&slot);
        ever_freed = true
    }
    Ok(ever_freed || ever_deposited && !this.bus_wait_queue.is_empty())
}

async fn fluid_bus_main(factory: Weak<RefCell<Factory>>) -> Result<(), LocalStr> {
    loop {
        let result = fluid_bus_update(&factory).await;
        alive_mut!(factory, this);
        match result {
            Err(e) => {
                let text = local_fmt!("fluid bus failed: {}", e);
                for sender in take(&mut this.fluid_bus_wait_queue) {
                    sender.send(Err(text.clone()))
                }
                this.log(Print { text, color: 0xFF0000, beep: Some(880.0) });
            }
            Ok(true) => continue,
            Ok(false) => (),
        }
        this.fluid_bus_task = None;
        break Ok(());
    }
}

async fn fluid_bus_update(factory: &Weak<RefCell<Factory>>) -> Result<bool, LocalStr> {
    let buses = {
        alive_mut!(factory, this);
        this.n_fluid_bus_updates += 1;
        let Some(acceess) = this.config.fluid_bus_accesses.first() else { return Ok(false) };
        let n_buses = acceess.tanks.len();
        let server = this.config.server.borrow();
        join_outputs(Vec::from_iter(
            (0..n_buses).map(|i| spawn(read_tanks(&*server, &this.config.fluid_bus_accesses, |access| access.tanks[i].clone()))),
        ))
    };
    let buses = buses.await?;
    let mut tasks = Vec::new();
    {
        alive_mut!(factory, this);
        let mut empty_buses = Vec::new();
        for (i, tanks) in buses.into_iter().enumerate() {
            if !this.fluid_bus_allocations.contains(&i) {
                let fluids = tanks_to_fluid_map(&tanks);
                if fluids.is_empty() {
                    empty_buses.push(i)
                } else {
                    for (fluid, (slot, qty)) in fluids {
                        this.fluid_deposit(i, slot, fluid, qty, &mut tasks)
                    }
                }
            }
        }
        while !empty_buses.is_empty() && !this.fluid_bus_wait_queue.is_empty() {
            let bus = empty_buses.pop().unwrap();
            this.fluid_bus_allocations.insert(bus);
            this.fluid_bus_wait_queue.pop_front().unwrap().send(Ok(bus))
        }
    }
    let ever_deposited = !tasks.is_empty();
    join_tasks(tasks).await?;
    alive_mut!(factory, this);
    let mut ever_freed = false;
    for slot in take(&mut this.fluid_bus_free_queue) {
        this.fluid_bus_allocations.remove(&slot);
        ever_freed = true
    }
    Ok(ever_freed || ever_deposited && !this.fluid_bus_wait_queue.is_empty())
}

pub struct Tank {
    pub capacity: i64,
    pub qty: i64,
    pub fluid: Option<LocalStr>,
}

pub fn read_tanks<'a, T: Access + 'a>(
    server: &Server,
    accesses: impl IntoIterator<Item = &'a T>,
    tank: impl Fn(&'a T) -> EachTank,
) -> impl Future<Output = Result<Vec<Tank>, LocalStr>> + 'static {
    let access = server.load_balance(accesses).1;
    let tank = tank(access);
    let action = ActionFuture::from(Call { addr: tank.addr, func: local_str!("getFluidInTank"), args: vec![tank.side.into()] });
    server.enqueue_request_group(access.get_client(), vec![action.clone().into()]);
    async move {
        let tanks = table_to_vec(call_result::<Table>(action.await?)?)?;
        let mut result = Vec::with_capacity(tanks.len());
        for tank in tanks {
            let mut tank = Table::try_from(tank)?;
            let capacity = table_remove(&mut tank, "capacity")?;
            let qty = table_remove(&mut tank, "amount")?;
            let fluid = if qty > 0 { Some(table_remove(&mut tank, "name")?) } else { None };
            result.push(Tank { capacity, qty, fluid })
        }
        Ok(result)
    }
}

pub fn tanks_to_fluid_map(tanks: &[Tank]) -> FnvHashMap<LocalStr, (usize, i64)> {
    let mut result = FnvHashMap::default();
    for (slot, tank) in tanks.into_iter().enumerate() {
        let Some(fluid) = &tank.fluid else { continue };
        result.entry(fluid.clone()).or_insert((slot, 0)).1 += tank.qty
    }
    result
}

impl FluidStorage {
    fn update(&self) -> ChildTask<Result<(), LocalStr>> {
        let task = read_tanks(&*self.factory.upgrade().unwrap().borrow().borrow_server(), &self.config.accesses, |access| EachTank {
            addr: access.buses[0].addr.clone(),
            side: access.buses[0].tank_side,
        });
        let weak = self.weak.clone();
        spawn(async move {
            let tanks = task.await?;
            alive_mut!(weak, this);
            for (i, tank) in tanks.into_iter().enumerate() {
                if let Some(fluid) = tank.fluid {
                    if fluid != this.config.fluid {
                        upgrade!(this.factory, this);
                        this.log(Print { text: local_fmt!("unexpected {fluid} stored"), color: 0xFF0000, beep: Some(880.0) });
                        continue;
                    }
                }
                this.slot = Some(i);
                this.capacity += tank.capacity;
                this.n_stored_hi += tank.qty;
                this.n_stored_lo += tank.qty
            }
            Ok(())
        })
    }
}
