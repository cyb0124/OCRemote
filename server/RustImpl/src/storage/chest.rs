use super::super::access::InvAccess;
use super::super::action::{ActionFuture, Call, List};
use super::super::factory::Factory;
use super::super::item::{Item, ItemStack};
use super::super::util::{alive, spawn};
use super::{DepositResult, Extractor, IntoStorage, Provider, Storage};
use abort_on_drop::ChildTask;
use flexstr::{local_str, LocalStr};
use std::{
    cell::RefCell,
    cmp::min,
    rc::{Rc, Weak},
};

pub struct ChestConfig {
    pub accesses: Vec<InvAccess>,
}

pub struct ChestStorage {
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
    type Output = ChestStorage;
    fn into_storage(self, factory: &Weak<RefCell<Factory>>) -> Rc<RefCell<Self::Output>> {
        Rc::new_cyclic(|weak| {
            RefCell::new(Self::Output {
                weak: weak.clone(),
                config: self,
                factory: factory.clone(),
                stacks: Vec::new(),
                inv_slot_to_deposit: 0,
            })
        })
    }
}

impl Storage for ChestStorage {
    fn update(&self, factory: &Factory) -> ChildTask<Result<(), LocalStr>> {
        let server = factory.borrow_server();
        let access = server.load_balance(&self.config.accesses).1;
        let action = ActionFuture::from(List { addr: access.addr.clone(), side: access.inv_side });
        server.enqueue_request_group(&access.client, vec![action.clone().into()]);
        let weak = self.weak.clone();
        spawn(async move {
            let stacks = action.await?;
            alive_mut!(weak, this);
            this.stacks = stacks;
            upgrade_mut!(this.factory, factory);
            for (inv_slot, stack) in this.stacks.iter().enumerate() {
                if let Some(stack) = stack {
                    factory.register_stored_item(stack.item.clone()).provide(Provider {
                        priority: -stack.size,
                        n_provided: stack.size.into(),
                        extractor: Rc::new(ChestExtractor { weak: weak.clone(), inv_slot }),
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
            addr: access.addr.clone(),
            func: local_str!("transferItem"),
            args: vec![
                access.bus_side.into(),
                access.inv_side.into(),
                n_deposited.into(),
                (bus_slot + 1).into(),
                (inv_slot + 1).into(),
            ],
        });
        server.enqueue_request_group(&access.client, vec![action.clone().into()]);
        let task = spawn(async move { action.await.map(|_| ()) });
        DepositResult { n_deposited, task }
    }
}

impl Extractor for ChestExtractor {
    fn extract(&self, factory: &Factory, size: i32, bus_slot: usize) -> ChildTask<Result<(), LocalStr>> {
        let inv_slot = self.inv_slot;
        let server = factory.borrow_server();
        upgrade!(self.weak, this);
        let access = server.load_balance(&this.config.accesses).1;
        let action = ActionFuture::from(Call {
            addr: access.addr.clone(),
            func: local_str!("transferItem"),
            args: vec![
                access.inv_side.into(),
                access.bus_side.into(),
                size.into(),
                (inv_slot + 1).into(),
                (bus_slot + 1).into(),
            ],
        });
        server.enqueue_request_group(&access.client, vec![action.clone().into()]);
        let weak = self.weak.clone();
        spawn(async move {
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
