use super::access::BusAccess;
use super::action::{ActionFuture, Print};
use super::async_helpers::{join_all, spawn, AbortOnDrop};
use super::item::{Filter, Item};
use super::process::Process;
use super::server::Server;
use super::storage::Storage;
use fnv::{FnvHashMap, FnvHashSet};
use ordered_float::NotNan;
use std::{
    cell::{Cell, RefCell},
    collections::hash_map::Entry,
    collections::VecDeque,
    future::Future,
    pin::Pin,
    rc::{Rc, Weak},
    task::{Context, Poll, Waker},
    time::Duration,
};
use tokio::time::{sleep_until, Instant};

pub struct ItemInfo {
    // TODO: providers
    n_avail: Cell<i32>,
    n_backup: Cell<i32>,
}

struct BusState {
    dirty: bool,
    task: Option<AbortOnDrop<()>>,
}

struct BusWaitState {
    result: Option<Result<usize, String>>,
    waker: Option<Waker>,
}

struct BusWait(Rc<RefCell<BusWaitState>>);

impl Future for BusWait {
    type Output = Result<usize, String>;

    fn poll(self: Pin<&mut Self>, cx: &mut Context<'_>) -> Poll<Self::Output> {
        let this = self.get_mut();
        let mut this = this.0.borrow_mut();
        if let Some(result) = this.result.take() {
            Poll::Ready(result)
        } else {
            this.waker = Some(cx.waker().clone());
            Poll::Pending
        }
    }
}

pub struct Factory {
    task: AbortOnDrop<()>,
    server: Rc<RefCell<Server>>,
    log_clients: Vec<&'static str>,
    bus_accesses: Vec<BusAccess>,
    storages: Vec<Rc<RefCell<dyn Storage>>>,
    backups: Vec<(Filter, i32)>,
    processes: Vec<Rc<RefCell<dyn Process>>>,

    items: FnvHashMap<Rc<Item>, ItemInfo>,
    label_map: FnvHashMap<String, Vec<Rc<Item>>>,
    name_map: FnvHashMap<String, Vec<Rc<Item>>>,

    bus_state: Option<BusState>,
    bus_allocations: FnvHashSet<usize>,
    bus_wait_queue: VecDeque<Weak<RefCell<BusWaitState>>>,
    bus_free_slots: Vec<usize>,
    bus_ever_updated: bool,
}

impl Factory {
    pub fn new(
        server: Rc<RefCell<Server>>,
        min_cycle_time: Duration,
        log_clients: Vec<&'static str>,
        bus_accesses: Vec<BusAccess>,
    ) -> Rc<RefCell<Factory>> {
        Rc::new_cyclic(|weak| {
            RefCell::new(Factory {
                task: spawn(factory_main(weak.clone(), min_cycle_time)),
                server,
                log_clients,
                bus_accesses,
                storages: Vec::new(),
                backups: Vec::new(),
                processes: Vec::new(),

                items: FnvHashMap::default(),
                label_map: FnvHashMap::default(),
                name_map: FnvHashMap::default(),

                bus_state: None,
                bus_allocations: FnvHashSet::default(),
                bus_wait_queue: VecDeque::new(),
                bus_free_slots: Vec::new(),
                bus_ever_updated: false,
            })
        })
    }

    pub fn add_storage(&mut self, storage: Rc<RefCell<dyn Storage>>) {
        self.storages.push(storage)
    }

    pub fn add_backup(&mut self, filter: Filter, size: i32) {
        self.backups.push((filter, size))
    }

    pub fn add_process(&mut self, process: Rc<RefCell<dyn Process>>) {
        self.processes.push(process)
    }

    fn log(&self, action: Print) {
        println!("{}", action.text);
        let action = ActionFuture::from(action);
        let server = self.server.borrow();
        for client in &self.log_clients {
            server.enqueue_request_group(client, vec![action.clone().into()]);
        }
    }

    pub fn register_stored_item(&mut self, item: Rc<Item>) -> &mut ItemInfo {
        match self.items.entry(item) {
            Entry::Occupied(x) => x.into_mut(),
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
                x.insert(ItemInfo {
                    n_avail: 0.into(),
                    n_backup: 0.into(),
                })
            }
        }
    }

    pub fn get_item<'a>(&'a self, filter: &Filter) -> Option<(&'a Rc<Item>, &'a ItemInfo)> {
        let mut best: Option<(&'a Rc<Item>, &'a ItemInfo)> = None;
        let mut on_candidate = |(new_item, new_info): (&'a Rc<Item>, &'a ItemInfo)| {
            if let Some((_, old_info)) = best {
                if new_info.n_avail <= old_info.n_avail {
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

    fn cleanup(&mut self) {
        for storage in &self.storages {
            storage.borrow_mut().cleanup()
        }
        self.items.clear();
        self.label_map.clear();
        self.name_map.clear();
        self.bus_ever_updated = false
    }
}

async fn factory_main(factory: Weak<RefCell<Factory>>, min_cycle_time: Duration) {
    let mut cycle_start_last: Option<Instant> = None;
    let mut n_cycles = 0usize;
    async {
        loop {
            let cycle_start_time = Instant::now();
            let mut text = format!("Cycle {}", n_cycles);
            if let Some(last) = cycle_start_last {
                text += &format!(
                    ", lastCycleTime={:.03}",
                    (cycle_start_time - last).as_secs_f32()
                )
            }
            factory.upgrade()?.borrow().log(Print {
                text,
                color: 0xFFFFFF,
                beep: None,
            });
            let cycle = async {
                update_storages(&factory).await?;
                run_processes(&factory).await
            };
            let bus_task = if let Err(e) = cycle.await {
                if let Some(e) = e {
                    let this = factory.upgrade()?;
                    let mut this = this.borrow_mut();
                    this.log(Print {
                        text: format!("Cycle failed: {}", e),
                        color: 0xFF0000,
                        beep: Some(NotNan::new(880.0).unwrap()),
                    });
                    this.bus_state
                        .as_mut()
                        .map(|state| state.task.take().unwrap())
                } else {
                    return Option::<!>::None;
                }
            } else {
                n_cycles += 1;
                let this = factory.upgrade()?;
                let mut this = this.borrow_mut();
                if let Some(ref mut state) = this.bus_state {
                    Some(state.task.take().unwrap())
                } else if this.bus_ever_updated {
                    None
                } else {
                    this.bus_state = Some(BusState {
                        dirty: true,
                        task: None,
                    });
                    Some(spawn(bus_main(factory.clone())))
                }
            };
            if let Some(task) = bus_task {
                task.into_future().await
            }
            factory.upgrade()?.borrow_mut().cleanup();
            sleep_until(cycle_start_time + min_cycle_time).await;
            cycle_start_last = Some(cycle_start_time)
        }
    }
    .await;
}

async fn update_storages(factory: &Weak<RefCell<Factory>>) -> Result<(), Option<String>> {
    let tasks = factory
        .upgrade()
        .ok_or(None)?
        .borrow()
        .storages
        .iter()
        .map(|storage| storage.borrow().update())
        .collect();
    join_all(tasks).await?;
    let this = factory.upgrade().ok_or(None)?;
    let this = this.borrow();
    let mut total = 0;
    for (_, item) in &this.items {
        total += item.n_avail.get();
    }
    this.log(Print {
        text: format!("storage: {} items, {} types", total, this.items.len()),
        color: 0x00FF00,
        beep: None,
    });
    for (filter, size) in &this.backups {
        if let Some((_, info)) = this.get_item(filter) {
            info.n_backup.set(info.n_backup.get() + size)
        }
    }
    Ok(())
}

async fn run_processes(factory: &Weak<RefCell<Factory>>) -> Result<(), Option<String>> {
    let tasks = factory
        .upgrade()
        .ok_or(None)?
        .borrow()
        .processes
        .iter()
        .map(|process| process.borrow().run())
        .collect();
    join_all(tasks).await
}

async fn bus_main(factory: Weak<RefCell<Factory>>) {
    todo!()
}
