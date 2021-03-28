use super::super::access::InvAccess;
use super::super::item::Filter;
use super::super::recipe::{Input, Output};
use super::ExtractFilter;

pub struct SlottedInput {
    item: Filter,
    size: i32,
    slots: Vec<usize>,
    allow_backup: bool,
    extra_backup: i32,
}

impl SlottedInput {
    pub fn new(item: Filter, size: i32, slots: Vec<usize>) -> Self {
        SlottedInput {
            item,
            size,
            slots,
            allow_backup: false,
            extra_backup: 0,
        }
    }
}

impl_input!(SlottedInput);

pub struct SlottedRecipe {
    pub outputs: Vec<Output>,
    pub inputs: Vec<SlottedInput>,
    pub max_per_slot: i32,
}

pub struct SlottedConfig {
    pub name: &'static str,
    pub accesses: Vec<InvAccess>,
    pub input_slots: Vec<usize>,
    pub to_extract: Option<ExtractFilter>,
    pub recipes: Vec<SlottedRecipe>,
}
