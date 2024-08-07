use super::{extract_output, list_inv, IntoProcess, Inventory, Process, SlotFilter};
use crate::access::InvAccess;
use crate::factory::Factory;
use crate::item::Item;
use crate::recipe::Output;
use crate::util::{alive, join_tasks, spawn};
use abort_on_drop::ChildTask;
use flexstr::LocalStr;
use fnv::FnvHashMap;
use std::{
    cell::RefCell,
    cmp::{max, min},
    collections::hash_map::Entry,
    rc::{Rc, Weak},
};

pub struct BlockingOutputConfig {
    pub accesses: Vec<InvAccess>,
    pub slot_filter: Option<SlotFilter>,
    pub outputs: Vec<Output>,
}

pub struct BlockingOutputProcess {
    weak: Weak<RefCell<BlockingOutputProcess>>,
    config: BlockingOutputConfig,
    factory: Weak<RefCell<Factory>>,
}

impl_inventory!(BlockingOutputProcess);

impl IntoProcess for BlockingOutputConfig {
    type Output = BlockingOutputProcess;
    fn into_process(self, factory: &Factory) -> Rc<RefCell<Self::Output>> {
        Rc::new_cyclic(|weak| {
            RefCell::new(Self::Output { weak: weak.clone(), config: self, factory: factory.weak.clone() })
        })
    }
}

struct Info {
    n_stored: i32,
    n_wanted: i32,
}

impl Process for BlockingOutputProcess {
    fn run(&self, factory: &Factory) -> ChildTask<Result<(), LocalStr>> {
        let mut enough = true;
        for output in &self.config.outputs {
            if factory.search_n_stored(&output.item) < output.n_wanted {
                enough = false;
                break;
            }
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
                let mut infos = FnvHashMap::<&Rc<Item>, Info>::default();
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
                                let mut info = Info { n_wanted: 0, n_stored: factory.get_n_stored(&stack.item) };
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
