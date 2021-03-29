use super::access::{InvAccess, MEAccess};
use super::action::{ActionFuture, Call, List, ListME, XferME};
use super::factory::Factory;
use super::item::{Filter, Item, ItemStack};
use super::util::{alive, spawn, AbortOnDrop};
use fnv::FnvHashMap;
use std::{
    cell::{Cell, RefCell},
    cmp::{min, Ordering},
    rc::{Rc, Weak},
};

pub struct DepositResult {
    pub n_deposited: i32,
    pub task: AbortOnDrop<Result<(), String>>,
}

pub trait Storage {
    fn update(&self, factory: &Factory) -> AbortOnDrop<Result<(), String>>;
    fn cleanup(&mut self);
    fn deposit_priority(&mut self, item: &Rc<Item>) -> Option<i32>;
    fn deposit(&mut self, factory: &Factory, stack: &ItemStack, bus_slot: usize) -> DepositResult;
}

pub trait IntoStorage {
    fn into_storage(self, factory: Weak<RefCell<Factory>>) -> Rc<RefCell<dyn Storage>>;
}

pub trait Extractor {
    fn extract(&self, size: i32, bus_slot: usize) -> AbortOnDrop<Result<(), String>>;
}

pub struct Provider {
    priority: i32,
    pub n_provided: Cell<i32>,
    pub extractor: Rc<dyn Extractor>,
}

impl PartialEq<Provider> for Provider {
    fn eq(&self, other: &Self) -> bool { self.priority == other.priority }
}

impl Eq for Provider {}

impl PartialOrd<Provider> for Provider {
    fn partial_cmp(&self, other: &Self) -> Option<Ordering> { Some(self.cmp(other)) }
}

impl Ord for Provider {
    fn cmp(&self, other: &Self) -> Ordering { self.priority.cmp(&other.priority) }
}

pub struct ChestConfig {
    pub accesses: Vec<InvAccess>,
}

struct ChestStorage {
    weak: Weak<RefCell<ChestStorage>>,
    config: ChestConfig,
    factory: Weak<RefCell<Factory>>,
    stacks: Vec<Option<ItemStack>>,
    inv_slot_to_deposit: usize,
}

struct ChestExtractor {
    weak: Weak<RefCell<ChestStorage>>,
    inv_slot: usize,
}

impl IntoStorage for ChestConfig {
    fn into_storage(self, factory: Weak<RefCell<Factory>>) -> Rc<RefCell<dyn Storage>> {
        Rc::new_cyclic(|weak| {
            RefCell::new(ChestStorage {
                weak: weak.clone(),
                config: self,
                factory,
                stacks: Vec::new(),
                inv_slot_to_deposit: 0,
            })
        })
    }
}

impl Storage for ChestStorage {
    fn update(&self, factory: &Factory) -> AbortOnDrop<Result<(), String>> {
        let server = factory.borrow_server();
        let access = server.load_balance(&self.config.accesses).1;
        let action = ActionFuture::from(List {
            addr: access.addr,
            side: access.inv_side,
        });
        server.enqueue_request_group(access.client, vec![action.clone().into()]);
        let weak = self.weak.clone();
        spawn(async move {
            let stacks = action.await?;
            alive_mut!(weak, this);
            this.stacks = stacks;
            upgrade_mut!(this.factory, factory);
            for (inv_slot, stack) in this.stacks.iter().enumerate() {
                if let Some(stack) = stack {
                    factory
                        .register_stored_item(stack.item.clone())
                        .provide(Provider {
                            priority: -stack.size,
                            n_provided: stack.size.into(),
                            extractor: Rc::new(ChestExtractor {
                                weak: weak.clone(),
                                inv_slot,
                            }),
                        });
                }
            }
            Ok(())
        })
    }

    fn cleanup(&mut self) { self.stacks.clear() }

    fn deposit_priority(&mut self, item: &Rc<Item>) -> Option<i32> {
        let mut empty_slot = None;
        let mut size_of_best_slot = None;
        for (inv_slot, stack) in self.stacks.iter().enumerate() {
            if let Some(stack) = stack {
                if stack.item == *item && stack.size < item.max_size {
                    if let Some(best_size) = size_of_best_slot {
                        if stack.size <= best_size {
                            continue;
                        }
                    }
                    size_of_best_slot = Some(stack.size);
                    self.inv_slot_to_deposit = inv_slot
                }
            } else {
                empty_slot = Some(inv_slot)
            }
        }
        size_of_best_slot.or_else(|| {
            empty_slot.map(|x| {
                self.inv_slot_to_deposit = x;
                i32::MIN + 1
            })
        })
    }

    fn deposit(&mut self, factory: &Factory, stack: &ItemStack, bus_slot: usize) -> DepositResult {
        let inv_slot = self.inv_slot_to_deposit;
        let inv_stack = &mut self.stacks[inv_slot];
        let n_deposited;
        if let Some(inv_stack) = inv_stack {
            n_deposited = min(stack.size, inv_stack.item.max_size - inv_stack.size);
            inv_stack.size += n_deposited
        } else {
            n_deposited = stack.size;
            *inv_stack = Some(stack.clone())
        }
        let server = factory.borrow_server();
        let access = server.load_balance(&self.config.accesses).1;
        let action = ActionFuture::from(Call {
            addr: access.addr,
            func: "transferItem",
            args: vec![
                access.bus_side.into(),
                access.inv_side.into(),
                n_deposited.into(),
                (bus_slot + 1).into(),
                (inv_slot + 1).into(),
            ],
        });
        server.enqueue_request_group(access.client, vec![action.clone().into()]);
        let task = spawn(async move { action.await.map(|_| ()) });
        DepositResult { n_deposited, task }
    }
}

impl Extractor for ChestExtractor {
    fn extract(&self, size: i32, bus_slot: usize) -> AbortOnDrop<Result<(), String>> {
        let weak = self.weak.clone();
        let inv_slot = self.inv_slot;
        spawn(async move {
            let action;
            {
                alive!(weak, this);
                upgrade!(this.factory, factory);
                let server = factory.borrow_server();
                let access = server.load_balance(&this.config.accesses).1;
                action = ActionFuture::from(Call {
                    addr: access.addr,
                    func: "transferItem",
                    args: vec![
                        access.inv_side.into(),
                        access.bus_side.into(),
                        size.into(),
                        (inv_slot + 1).into(),
                        (bus_slot + 1).into(),
                    ],
                });
                server.enqueue_request_group(access.client, vec![action.clone().into()]);
            }
            action.await?;
            alive_mut!(weak, this);
            let inv_stack = &mut this.stacks[inv_slot];
            let inv_size = &mut inv_stack.as_mut().unwrap().size;
            *inv_size -= size;
            if *inv_size <= 0 {
                *inv_stack = None;
            }
            Ok(())
        })
    }
}

pub struct DrawerConfig {
    pub accesses: Vec<InvAccess>,
    pub filters: Vec<Filter>,
}

struct DrawerStorage {
    weak: Weak<RefCell<DrawerStorage>>,
    config: DrawerConfig,
    factory: Weak<RefCell<Factory>>,
}

struct DrawerExtractor {
    weak: Weak<RefCell<DrawerStorage>>,
    inv_slot: usize,
}

impl IntoStorage for DrawerConfig {
    fn into_storage(self, factory: Weak<RefCell<Factory>>) -> Rc<RefCell<dyn Storage>> {
        Rc::new_cyclic(|weak| {
            RefCell::new(DrawerStorage {
                weak: weak.clone(),
                config: self,
                factory,
            })
        })
    }
}

impl Storage for DrawerStorage {
    fn update(&self, factory: &Factory) -> AbortOnDrop<Result<(), String>> {
        let server = factory.borrow_server();
        let access = server.load_balance(&self.config.accesses).1;
        let action = ActionFuture::from(List {
            addr: access.addr,
            side: access.inv_side,
        });
        server.enqueue_request_group(access.client, vec![action.clone().into()]);
        let weak = self.weak.clone();
        spawn(async move {
            let stacks = action.await?;
            alive!(weak, this);
            upgrade_mut!(this.factory, factory);
            for (inv_slot, stack) in stacks.into_iter().enumerate() {
                if let Some(stack) = stack {
                    factory.register_stored_item(stack.item).provide(Provider {
                        priority: i32::MIN,
                        n_provided: stack.size.into(),
                        extractor: Rc::new(DrawerExtractor {
                            weak: weak.clone(),
                            inv_slot,
                        }),
                    });
                }
            }
            Ok(())
        })
    }

    fn cleanup(&mut self) {}

    fn deposit_priority(&mut self, item: &Rc<Item>) -> Option<i32> {
        for filter in &self.config.filters {
            if filter.apply(item) {
                return Some(i32::MAX);
            }
        }
        None
    }

    fn deposit(&mut self, factory: &Factory, stack: &ItemStack, bus_slot: usize) -> DepositResult {
        let n_deposited = stack.size;
        let server = factory.borrow_server();
        let access = server.load_balance(&self.config.accesses).1;
        let action = ActionFuture::from(Call {
            addr: access.addr,
            func: "transferItem",
            args: vec![
                access.bus_side.into(),
                access.inv_side.into(),
                n_deposited.into(),
                (bus_slot + 1).into(),
            ],
        });
        server.enqueue_request_group(access.client, vec![action.clone().into()]);
        let task = spawn(async move { action.await.map(|_| ()) });
        DepositResult { n_deposited, task }
    }
}

impl Extractor for DrawerExtractor {
    fn extract(&self, size: i32, bus_slot: usize) -> AbortOnDrop<Result<(), String>> {
        let weak = self.weak.clone();
        let inv_slot = self.inv_slot;
        spawn(async move {
            let action;
            {
                alive!(weak, this);
                upgrade!(this.factory, factory);
                let server = factory.borrow_server();
                let access = server.load_balance(&this.config.accesses).1;
                action = ActionFuture::from(Call {
                    addr: access.addr,
                    func: "transferItem",
                    args: vec![
                        access.inv_side.into(),
                        access.bus_side.into(),
                        size.into(),
                        (inv_slot + 1).into(),
                        (bus_slot + 1).into(),
                    ],
                });
                server.enqueue_request_group(access.client, vec![action.clone().into()]);
            }
            action.await.map(|_| ())
        })
    }
}

pub struct MEConfig {
    accesses: Vec<MEAccess>,
}

struct MEStorage {
    weak: Weak<RefCell<MEStorage>>,
    config: MEConfig,
    factory: Weak<RefCell<Factory>>,
    access_for_item: FnvHashMap<Rc<Item>, usize>,
}

struct MEExtractor {
    weak: Weak<RefCell<MEStorage>>,
    item: Rc<Item>,
}

impl IntoStorage for MEConfig {
    fn into_storage(self, factory: Weak<RefCell<Factory>>) -> Rc<RefCell<dyn Storage>> {
        Rc::new_cyclic(|weak| {
            RefCell::new(MEStorage {
                weak: weak.clone(),
                config: self,
                factory,
                access_for_item: FnvHashMap::default(),
            })
        })
    }
}

impl Storage for MEStorage {
    fn update(&self, factory: &Factory) -> AbortOnDrop<Result<(), String>> {
        let server = factory.borrow_server();
        let access = server.load_balance(&self.config.accesses).1;
        let action = ActionFuture::from(ListME {
            addr: access.me_addr,
        });
        server.enqueue_request_group(access.client, vec![action.clone().into()]);
        let weak = self.weak.clone();
        spawn(async move {
            let stacks = action.await?;
            alive!(weak, this);
            upgrade_mut!(this.factory, factory);
            for mut stack in stacks.into_iter() {
                Rc::get_mut(&mut stack.item)
                    .unwrap()
                    .others
                    .remove(&"isCraftable".into());
                factory
                    .register_stored_item(stack.item.clone())
                    .provide(Provider {
                        priority: i32::MAX,
                        n_provided: stack.size.into(),
                        extractor: Rc::new(MEExtractor {
                            weak: weak.clone(),
                            item: stack.item,
                        }),
                    })
            }
            Ok(())
        })
    }

    fn cleanup(&mut self) { self.access_for_item.clear() }

    fn deposit_priority(&mut self, _item: &Rc<Item>) -> Option<i32> { Some(i32::MIN) }

    fn deposit(&mut self, factory: &Factory, stack: &ItemStack, bus_slot: usize) -> DepositResult {
        let n_deposited = stack.size;
        let server = factory.borrow_server();
        let access = server.load_balance(&self.config.accesses).1;
        let action = ActionFuture::from(Call {
            addr: access.transposer_addr,
            func: "transferItem",
            args: vec![
                access.bus_side.into(),
                access.me_side.into(),
                n_deposited.into(),
                (bus_slot + 1).into(),
                9.into(),
            ],
        });
        server.enqueue_request_group(access.client, vec![action.clone().into()]);
        let task = spawn(async move { action.await.map(|_| ()) });
        DepositResult { n_deposited, task }
    }
}

impl Extractor for MEExtractor {
    fn extract(&self, size: i32, bus_slot: usize) -> AbortOnDrop<Result<(), String>> {
        let weak = self.weak.clone();
        let item = self.item.clone();
        spawn(async move {
            let action;
            {
                alive_mut!(weak, this);
                upgrade!(this.factory, factory);
                let server = factory.borrow_server();
                let accesses = &this.config.accesses;
                let access = *this
                    .access_for_item
                    .entry(item.clone())
                    .or_insert_with(|| server.load_balance(accesses).0);
                let access = &this.config.accesses[access];
                action = ActionFuture::from(XferME {
                    me_addr: access.me_addr,
                    me_slot: access.me_slot,
                    filter: item.serialize(),
                    size,
                    transposer_addr: access.transposer_addr,
                    transposer_args: vec![
                        access.me_side.into(),
                        access.bus_side.into(),
                        size.into(),
                        (access.me_slot + 1).into(),
                        (bus_slot + 1).into(),
                    ],
                });
                server.enqueue_request_group(access.client, vec![action.clone().into()]);
            }
            action.await.map(|_| ())
        })
    }
}
