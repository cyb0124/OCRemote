use super::super::access::InvAccess;
use super::super::action::{ActionFuture, List};
use super::super::factory::Factory;
use super::super::item::{jammer, Filter, Item, ItemStack};
use super::super::recipe::{compute_demands, Input, Output, Recipe};
use super::super::util::{alive, join_tasks, spawn, AbortOnDrop};
use super::{extract_output, ExtractFilter, ExtractableProcess, IntoProcess, Process, SlotFilter};
use fnv::FnvHashMap;
use std::{
    cell::RefCell,
    rc::{Rc, Weak},
};

pub struct BufferedInput {
    item: Filter,
    size: i32,
    allow_backup: bool,
    extra_backup: i32,
}

impl BufferedInput {
    pub fn new(item: Filter, size: i32) -> Self {
        BufferedInput {
            item,
            size,
            allow_backup: false,
            extra_backup: 0,
        }
    }
}

impl_input!(BufferedInput);

pub struct BufferedRecipe {
    pub outputs: Vec<Output>,
    pub inputs: Vec<BufferedInput>,
    pub max_sets: i32,
}

impl_recipe!(BufferedRecipe, BufferedInput);

pub struct BufferedConfig {
    pub name: &'static str,
    pub accesses: Vec<InvAccess>,
    pub slot_filter: Option<SlotFilter>,
    pub to_extract: Option<ExtractFilter>,
    pub recipes: Vec<BufferedRecipe>,
    pub max_recipe_inputs: i32,
    pub stocks: Vec<BufferedInput>,
}

struct BufferedProcess {
    weak: Weak<RefCell<BufferedProcess>>,
    config: BufferedConfig,
    factory: Weak<RefCell<Factory>>,
}

impl_extractable_process!(BufferedProcess);

impl IntoProcess for BufferedConfig {
    fn into_process(self, factory: Weak<RefCell<Factory>>) -> Rc<RefCell<dyn Process>> {
        Rc::new_cyclic(|weak| {
            RefCell::new(BufferedProcess {
                weak: weak.clone(),
                config: self,
                factory,
            })
        })
    }
}

impl Process for BufferedProcess {
    fn run(&self, factory: &Factory) -> AbortOnDrop<Result<(), String>> {
        if self.config.to_extract.is_none() && self.config.stocks.is_empty() {
            if compute_demands(factory, &self.config.recipes).is_empty() {
                return spawn(async { Ok(()) });
            }
        }
        let server = factory.borrow_server();
        let access = server.load_balance(&self.config.accesses).1;
        let action = ActionFuture::from(List {
            addr: access.addr,
            side: access.inv_side,
        });
        server.enqueue_request_group(access.client, vec![action.clone().into()]);
        let weak = self.weak.clone();
        spawn(async move {
            let mut stacks = action.await?;
            let mut tasks = Vec::new();
            {
                alive!(weak, this);
                upgrade_mut!(this.factory, factory);
                let mut remaining_size = this.config.max_recipe_inputs;
                let mut existing_size = FnvHashMap::<Rc<Item>, i32>::default();
                'slot: for (slot, stack) in stacks.iter_mut().enumerate() {
                    if let Some(ref slot_filter) = this.config.slot_filter {
                        if !slot_filter(slot) {
                            *stack = Some(ItemStack { item: jammer(), size: 1 });
                            continue 'slot
                        }
                    }
                    if let Some(stack) = stack {
                        *existing_size.entry(stack.item.clone()).or_default() += stack.size;
                        for stock in &this.config.stocks {
                            if stock.item.apply(&stack.item) {
                                continue 'slot
                            }
                        }
                        remaining_size -= stack.size;
                        if let Some(ref to_extract) = this.config.to_extract {
                            for recipe in &this.config.recipes {
                                for input in &recipe.inputs {
                                    if input.item.apply(&stack.item) {
                                        continue 'slot
                                    }
                                }
                            }
                            if to_extract(slot, stack) {
                                tasks.push(extract_output(this, factory, slot, stack.item.max_size))
                            }
                        }
                    }
                }
                todo!()
            }
            join_tasks(tasks).await
        })
    }
}
