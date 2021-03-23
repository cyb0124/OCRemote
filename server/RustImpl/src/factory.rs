use super::access::BusAccess;
use super::action::{ActionFuture, List, Print};
use super::item::{Filter, Item, ItemStack};
use super::process::Process;
use super::server::Server;
use super::storage::{DepositResult, Extractor, Provider, Storage};
use super::util::{
    alive, join_all, make_local_one_shot, spawn, AbortOnDrop, LocalReceiver, LocalSender,
};
use fnv::{FnvHashMap, FnvHashSet};
use ordered_float::NotNan;
use std::{
    cell::{Ref, RefCell},
    cmp::{max, min},
    collections::{hash_map::Entry, BinaryHeap, VecDeque},
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
    pub async fn extract(self, bus_slot: usize) -> Result<(), String> {
        let tasks = self
            .extractors
            .into_iter()
            .map(|(extractor, size)| extractor.extract(size, bus_slot))
            .collect();
        join_all(tasks).await
    }
}

pub struct FactoryConfig {
    pub server: Rc<RefCell<Server>>,
    pub min_cycle_time: Duration,
    pub log_clients: Vec<&'static str>,
    pub bus_accesses: Vec<BusAccess>,
}

pub struct Factory {
    weak: Weak<RefCell<Factory>>,
    _task: AbortOnDrop<Result<(), String>>,
    config: FactoryConfig,
    storages: Vec<Rc<RefCell<dyn Storage>>>,
    backups: Vec<(Filter, i32)>,
    processes: Vec<Rc<RefCell<dyn Process>>>,

    items: FnvHashMap<Rc<Item>, RefCell<ItemInfo>>,
    label_map: FnvHashMap<String, Vec<Rc<Item>>>,
    name_map: FnvHashMap<String, Vec<Rc<Item>>>,

    bus_task: Option<AbortOnDrop<Result<(), String>>>,
    bus_allocations: FnvHashSet<usize>,
    bus_wait_queue: VecDeque<LocalSender<usize>>,
    bus_free_queue: Vec<usize>,
    n_bus_updates: usize,
}

impl FactoryConfig {
    pub fn into_factory(self) -> Rc<RefCell<Factory>> {
        Rc::new_cyclic(|weak| {
            RefCell::new(Factory {
                weak: weak.clone(),
                _task: spawn(factory_main(weak.clone())),
                config: self,
                storages: Vec::new(),
                backups: Vec::new(),
                processes: Vec::new(),

                items: FnvHashMap::default(),
                label_map: FnvHashMap::default(),
                name_map: FnvHashMap::default(),

                bus_task: None,
                bus_allocations: FnvHashSet::default(),
                bus_wait_queue: VecDeque::new(),
                bus_free_queue: Vec::new(),
                n_bus_updates: 0,
            })
        })
    }
}

impl Factory {
    pub fn add_storage(&mut self, storage: Rc<RefCell<dyn Storage>>) {
        self.storages.push(storage)
    }

    pub fn add_backup(&mut self, filter: Filter, n_backup: i32) {
        self.backups.push((filter, n_backup))
    }

    pub fn add_process(&mut self, process: Rc<RefCell<dyn Process>>) {
        self.processes.push(process)
    }

    pub fn borrow_server(&self) -> Ref<Server> {
        self.config.server.borrow()
    }

    fn log(&self, action: Print) {
        println!("{}", action.text);
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
                self.label_map
                    .entry(item.label.clone())
                    .or_default()
                    .push(item.clone());
                self.name_map
                    .entry(item.name.clone())
                    .or_default()
                    .push(item.clone());
                x.insert(RefCell::new(ItemInfo {
                    n_stored: 0,
                    n_backup: 0,
                    providers: BinaryHeap::new(),
                }))
                .get_mut()
            }
        }
    }

    pub fn search_item<'a>(
        &'a self,
        filter: &Filter,
    ) -> Option<(&'a Rc<Item>, &'a RefCell<ItemInfo>)> {
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
                if let Some(items) = self.label_map.get(*label) {
                    for item in items {
                        on_candidate(self.items.get_key_value(item).unwrap())
                    }
                }
            }
            Filter::Name(name) => {
                if let Some(items) = self.name_map.get(*name) {
                    for item in items {
                        on_candidate(self.items.get_key_value(item).unwrap())
                    }
                }
            }
            Filter::Both { label, name } => {
                if let Some(items) = self.label_map.get(*label) {
                    for item in items {
                        if item.name == *name {
                            on_candidate(self.items.get_key_value(item).unwrap())
                        }
                    }
                }
            }
            Filter::Fn(filter) => {
                for (item, info) in &self.items {
                    if filter(item) {
                        on_candidate((item, info))
                    }
                }
            }
        }
        best
    }

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

    fn deposit(
        &self,
        bus_slot: usize,
        mut stack: ItemStack,
        tasks: &mut Vec<AbortOnDrop<Result<(), String>>>,
    ) {
        self.log(Print {
            text: format!("{}*{}", stack.item.label, stack.size),
            color: 0xFFA500,
            beep: None,
        });
        while stack.size > 0 {
            let mut best: Option<(&Rc<RefCell<dyn Storage>>, i32)> = None;
            for storage in &self.storages {
                let priority = storage.borrow_mut().deposit_priority(&stack.item);
                if let Some(priority) = priority {
                    if let Some((_, best)) = best {
                        if priority <= best {
                            continue;
                        }
                    }
                    best = Some((storage, priority))
                }
            }
            if let Some((storage, _)) = best {
                let DepositResult { n_deposited, task } =
                    storage.borrow_mut().deposit(&stack, bus_slot);
                stack.size -= n_deposited;
                tasks.push(task)
            } else {
                tasks.push(spawn(async { Err("storage is full".to_owned()) }));
                break;
            }
        }
    }

    pub fn reserve_item(&self, reason: &str, item: &Rc<Item>, size: i32) -> Reservation {
        self.log(Print {
            text: format!("{}: {}*{}", reason, item.label, size),
            color: 0x55ABEC,
            beep: None,
        });
        self.items.get(item).unwrap().borrow_mut().reserve(size)
    }

    fn end_of_cycle(&mut self) {
        for storage in &self.storages {
            storage.borrow_mut().cleanup()
        }
        self.items.clear();
        self.label_map.clear();
        self.name_map.clear();
    }
}

async fn factory_main(factory: Weak<RefCell<Factory>>) -> Result<(), String> {
    let mut cycle_start_last: Option<Instant> = None;
    let mut n_cycles: usize = 0;
    loop {
        let cycle_start_time = Instant::now();
        {
            let this = alive(&factory)?;
            let mut this = this.borrow_mut();
            let mut text = format!("Cycle {}", n_cycles);
            if let Some(last) = cycle_start_last {
                text += &format!(
                    ", nBusUpdates={}, cycleTime={:.03}",
                    this.n_bus_updates,
                    (cycle_start_time - last).as_secs_f32()
                )
            }
            this.log(Print {
                text,
                color: 0xFFFFFF,
                beep: None,
            });
            this.n_bus_updates = 0;
        }
        let result = async {
            update_storages(&factory).await?;
            run_processes(&factory).await
        }
        .await;
        let mut bus_task;
        {
            let this = alive(&factory)?;
            let mut this = this.borrow_mut();
            bus_task = this.bus_task.take();
            if let Err(e) = result {
                this.log(Print {
                    text: format!("cycle failed: {}", e),
                    color: 0xFF0000,
                    beep: Some(NotNan::new(880.0).unwrap()),
                })
            } else {
                n_cycles += 1;
                if bus_task.is_none() && this.n_bus_updates == 0 {
                    bus_task = Some(spawn(bus_main(factory.clone())))
                }
            }
        }
        if let Some(task) = bus_task {
            task.into_future().await?
        }
        let min_cycle_time = {
            let this = alive(&factory)?;
            let mut this = this.borrow_mut();
            this.end_of_cycle();
            this.config.min_cycle_time
        };
        sleep_until(cycle_start_time + min_cycle_time).await;
        cycle_start_last = Some(cycle_start_time)
    }
}

async fn update_storages(factory: &Weak<RefCell<Factory>>) -> Result<(), String> {
    let tasks = alive(factory)?
        .borrow()
        .storages
        .iter()
        .map(|storage| storage.borrow().update())
        .collect();
    join_all(tasks).await?;
    let this = alive(factory)?;
    let this = this.borrow();
    let mut n_total = 0;
    for (_, item) in &this.items {
        n_total += item.borrow().n_stored
    }
    this.log(Print {
        text: format!("storage: {} items, {} types", n_total, this.items.len()),
        color: 0x00FF00,
        beep: None,
    });
    for (filter, n_backup) in &this.backups {
        if let Some((_, info)) = this.search_item(filter) {
            info.borrow_mut().n_backup += n_backup
        }
    }
    Ok(())
}

async fn run_processes(factory: &Weak<RefCell<Factory>>) -> Result<(), String> {
    let tasks = alive(factory)?
        .borrow()
        .processes
        .iter()
        .map(|process| process.borrow().run())
        .collect();
    join_all(tasks).await
}

async fn bus_main(factory: Weak<RefCell<Factory>>) -> Result<(), String> {
    loop {
        let result = bus_update(&factory).await;
        let this = alive(&factory)?;
        let mut this = this.borrow_mut();
        match result {
            Err(e) => {
                let e = format!("bus update failed: {}", e);
                this.log(Print {
                    text: e.clone(),
                    color: 0xFF0000,
                    beep: Some(NotNan::new(880.0).unwrap()),
                });
                for sender in take(&mut this.bus_wait_queue) {
                    sender.send(Err(e.clone()))
                }
            }
            Ok(true) => continue,
            Ok(false) => (),
        }
        this.bus_task = None;
        break Ok(());
    }
}

async fn bus_update(factory: &Weak<RefCell<Factory>>) -> Result<bool, String> {
    let action;
    {
        let this = alive(factory)?;
        let mut this = this.borrow_mut();
        this.n_bus_updates += 1;
        let server = this.borrow_server();
        let access = server.load_balance(&this.config.bus_accesses);
        action = ActionFuture::from(List {
            addr: access.addr,
            side: access.side,
        });
        server.enqueue_request_group(access.client, vec![action.clone().into()]);
    }
    let stacks = action.await?;
    let mut tasks = Vec::new();
    {
        let this = alive(factory)?;
        let mut this = this.borrow_mut();
        let mut free_slots = Vec::new();
        for (slot, stack) in stacks.into_iter().enumerate() {
            if !this.bus_allocations.contains(&slot) {
                if let Some(stack) = stack {
                    this.deposit(slot, stack, &mut tasks);
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
    join_all(tasks).await?;
    let this = alive(factory)?;
    let mut this = this.borrow_mut();
    let mut ever_freed = false;
    for slot in take(&mut this.bus_free_queue) {
        this.bus_allocations.remove(&slot);
        ever_freed = true
    }
    Ok(ever_freed || ever_deposited && !this.bus_wait_queue.is_empty())
}
