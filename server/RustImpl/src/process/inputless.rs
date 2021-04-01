use super::super::access::InvAccess;
use super::super::factory::Factory;
use super::super::item::Item;
use super::super::recipe::Output;
use super::super::util::{alive, join_tasks, spawn, AbortOnDrop};
use super::{extract_output, list_inv, IntoProcess, InvProcess, Process, SlotFilter};
use fnv::FnvHashMap;
use std::{
    cell::RefCell,
    cmp::{max, min},
    collections::hash_map::Entry,
    rc::{Rc, Weak},
};

pub struct InputlessConfig {
    pub accesses: Vec<InvAccess>,
    pub slot_filter: Option<SlotFilter>,
    pub outputs: Vec<Output>,
}

pub struct InputlessProcess {
    weak: Weak<RefCell<InputlessProcess>>,
    config: InputlessConfig,
    factory: Weak<RefCell<Factory>>,
}

impl_inv_process!(InputlessProcess);

impl IntoProcess for InputlessConfig {
    type Output = InputlessProcess;
    fn into_process(self, factory: &Weak<RefCell<Factory>>) -> Rc<RefCell<Self::Output>> {
        Rc::new_cyclic(|weak| RefCell::new(Self::Output { weak: weak.clone(), config: self, factory: factory.clone() }))
    }
}

struct InputlessInfo {
    n_stored: i32,
    n_wanted: i32,
}

impl Process for InputlessProcess {
    fn run(&self, factory: &Factory) -> AbortOnDrop<Result<(), String>> {
        let mut enough = true;
        for output in &self.config.outputs {
            if output.n_wanted <= 0 {
                continue;
            }
            if let Some((_, info)) = factory.search_item(&output.item) {
                if info.borrow().n_stored >= output.n_wanted {
                    continue;
                }
            }
            enough = false;
            break;
        }
        if enough {
            return spawn(async { Ok(()) });
        }
        let stacks = list_inv(self, factory);
        let weak = self.weak.clone();
        spawn(async move {
            let stacks = stacks.await?;
            let mut tasks = Vec::new();
            {
                alive!(weak, this);
                upgrade_mut!(this.factory, factory);
                let mut infos = FnvHashMap::<&Rc<Item>, InputlessInfo>::default();
                for (slot, stack) in stacks.iter().enumerate() {
                    if let Some(ref slot_filter) = this.config.slot_filter {
                        if !slot_filter(slot) {
                            continue;
                        }
                    }
                    if let Some(stack) = stack {
                        let info = match infos.entry(&stack.item) {
                            Entry::Occupied(entry) => entry.into_mut(),
                            Entry::Vacant(entry) => {
                                let mut info = InputlessInfo {
                                    n_wanted: 0,
                                    n_stored: factory.items.get(&stack.item).map_or(0, |info| info.borrow().n_stored),
                                };
                                for output in &this.config.outputs {
                                    if output.item.apply(&stack.item) {
                                        info.n_wanted = max(info.n_wanted, output.n_wanted)
                                    }
                                }
                                entry.insert(info)
                            }
                        };
                        let to_extract = min(info.n_wanted - info.n_stored, stack.size);
                        if to_extract <= 0 {
                            continue;
                        }
                        info.n_stored += to_extract;
                        tasks.push(extract_output(this, factory, slot, to_extract))
                    }
                }
            }
            join_tasks(tasks).await
        })
    }
}
