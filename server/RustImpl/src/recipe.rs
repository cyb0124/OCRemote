use super::item::{Filter, Item};
use crate::factory::Factory;
use flexstr::LocalStr;
use fnv::FnvHashMap;
use std::{
    cmp::{max_by, min_by},
    collections::hash_map::Entry,
    rc::Rc,
};

pub trait Outputs {
    fn get_priority(&self, factory: &Factory) -> Option<f64>;
}

impl<T: Fn(&Factory) -> Option<f64>> Outputs for T {
    fn get_priority(&self, factory: &Factory) -> Option<f64> { self(factory) }
}

pub trait BoxedOutputs {
    fn and(self, other: Self) -> Self;
    fn or(self, other: Self) -> Self;
    fn not(self) -> Self;
    fn map_priority(self, f: impl Fn(&Factory, f64) -> f64 + 'static) -> Self;
}

impl BoxedOutputs for Rc<dyn Outputs> {
    fn and(self, other: Self) -> Self {
        Rc::new(move |factory: &_| {
            max_by(self.get_priority(factory), other.get_priority(factory), |x, y| x.partial_cmp(y).unwrap())
        })
    }

    fn or(self, other: Self) -> Self {
        Rc::new(move |factory: &_| {
            min_by(self.get_priority(factory), other.get_priority(factory), |x, y| x.partial_cmp(y).unwrap())
        })
    }

    fn not(self) -> Self {
        Rc::new(move |factory: &_| match self.get_priority(factory) {
            Some(_) => None,
            None => Some(1.),
        })
    }

    fn map_priority(self, f: impl Fn(&Factory, f64) -> f64 + 'static) -> Self {
        Rc::new(move |factory: &_| self.get_priority(factory).map(|x| f(factory, x)))
    }
}

pub fn ignore_outputs(priority: f64) -> Rc<dyn Outputs> { Rc::new(move |_: &_| Some(priority)) }

pub struct Output {
    pub item: Filter,
    pub n_wanted: i32,
}

impl Output {
    pub fn new(item: Filter, n_wanted: i32) -> Rc<dyn Outputs> { Rc::new(Self { item, n_wanted }) }
}

impl Outputs for Output {
    fn get_priority(&self, factory: &Factory) -> Option<f64> {
        let n_stored = factory.search_n_stored(&self.item);
        let n_needed = self.n_wanted - n_stored;
        if n_needed > 0 {
            Some(n_needed as f64 / self.n_wanted as f64)
        } else {
            None
        }
    }
}

pub struct FluidOutput {
    pub fluid: LocalStr,
    pub n_wanted: i64,
}

impl FluidOutput {
    pub fn new(fluid: LocalStr, n_wanted: i64) -> Rc<dyn Outputs> { Rc::new(Self { fluid, n_wanted }) }
}

impl Outputs for FluidOutput {
    fn get_priority(&self, factory: &Factory) -> Option<f64> {
        let n_stored = factory.search_n_fluid(&self.fluid);
        let n_needed = self.n_wanted - n_stored;
        if n_needed > 0 {
            Some(n_needed as f64 / self.n_wanted as f64)
        } else {
            None
        }
    }
}

pub trait Input {
    fn get_item(&self) -> &Filter;
    fn get_size(&self) -> i32;
    fn get_allow_backup(&self) -> bool;
    fn get_extra_backup(&self) -> i32;
    fn allow_backup(self) -> Self;
    fn extra_backup(self, size: i32) -> Self;
}

macro_rules! impl_input {
    ($i:ident) => {
        impl Input for $i {
            fn get_item(&self) -> &Filter { &self.item }
            fn get_size(&self) -> i32 { self.size }
            fn get_allow_backup(&self) -> bool { self.allow_backup }
            fn get_extra_backup(&self) -> i32 { self.extra_backup }

            fn allow_backup(mut self) -> Self {
                self.allow_backup = true;
                self
            }

            fn extra_backup(mut self, size: i32) -> Self {
                self.extra_backup += size;
                self
            }
        }
    };
}

pub trait Recipe: Clone {
    type In: Input;
    fn get_outputs(&self) -> &dyn Outputs;
    fn get_inputs(&self) -> &Vec<Self::In>;
}

macro_rules! impl_recipe {
    ($r:ident, $i:ident) => {
        impl Recipe for $r {
            type In = $i;
            fn get_outputs(&self) -> &dyn Outputs { &*self.outputs }
            fn get_inputs(&self) -> &Vec<$i> { &self.inputs }
        }
    };
}

pub struct ResolvedInputs {
    pub n_sets: i32,
    pub priority: i32,
    pub items: Vec<Rc<Item>>,
}

struct InputInfo {
    n_available: i32,
    n_needed: i32,
}

pub fn resolve_inputs(factory: &Factory, recipe: &impl Recipe) -> Option<ResolvedInputs> {
    let mut items = Vec::new();
    items.reserve(recipe.get_inputs().len());
    let mut infos = FnvHashMap::<&Rc<Item>, InputInfo>::default();
    let mut max_size_bound = i32::MAX;
    for input in recipe.get_inputs() {
        if let Some((item, item_info)) = factory.search_item(input.get_item()) {
            items.push(item.clone());
            match infos.entry(item) {
                Entry::Occupied(input_info) => input_info.into_mut().n_needed += input.get_size(),
                Entry::Vacant(input_info) => {
                    input_info.insert(InputInfo {
                        // Note: backup params are considered for only the first input of the same item.
                        n_available: (item_info.borrow())
                            .get_availability(input.get_allow_backup(), input.get_extra_backup()),
                        n_needed: input.get_size(),
                    });
                }
            }
            max_size_bound = max_size_bound.min(item.max_size / input.get_size());
        } else {
            return None;
        }
    }
    let mut availability_bound = i32::MAX;
    for (_, input_info) in infos.into_iter() {
        let limit = input_info.n_available / input_info.n_needed;
        availability_bound = availability_bound.min(limit)
    }
    let n_sets = max_size_bound.min(availability_bound);
    if n_sets > 0 {
        Some(ResolvedInputs { n_sets, priority: availability_bound, items })
    } else {
        None
    }
}

pub struct Demand {
    pub i_recipe: usize,
    pub inputs: ResolvedInputs,
    pub priority: f64,
}

pub fn compute_demands(factory: &Factory, recipes: &[impl Recipe]) -> Vec<Demand> {
    let mut result = Vec::new();
    for (i_recipe, recipe) in recipes.iter().enumerate() {
        let Some(mut priority) = recipe.get_outputs().get_priority(factory) else { continue };
        let Some(inputs) = resolve_inputs(factory, recipe) else { continue };
        priority *= inputs.priority as f64;
        result.push(Demand { i_recipe, inputs, priority })
    }
    result.sort_by(|x: &Demand, y: &Demand| x.priority.partial_cmp(&y.priority).unwrap().reverse());
    result
}
