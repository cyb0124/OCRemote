use super::factory::Factory;
use super::item::{Filter, Item};
use fnv::FnvHashMap;
use std::rc::Rc;

pub struct Output {
    pub item: Filter,
    pub target: i32,
}

pub trait Input {
    fn get_filter(&self) -> &Filter;
    fn get_size(&self) -> i32;
    fn get_allow_backup(&self) -> bool;
    fn get_extra_backup(&self) -> i32;
}

pub trait Recipe {
    type In: Input;
    fn get_outputs(&self) -> &Vec<Output>;
    fn get_inputs(&self) -> &Vec<Self::In>;
}

pub struct ResolvedInputs {
    pub n_sets: i32,
    pub items: Vec<Rc<Item>>,
}

struct InputAvailInfo {
    n_avail: i32,
    n_needed: i32,
    allow_backup: bool,
}

pub struct Demand {
    i_recipe: usize,
    priority: f32,
    inputs: ResolvedInputs,
}
