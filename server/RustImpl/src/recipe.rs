use super::factory::Factory;
use super::item::{Filter, Item};
use fnv::FnvHashMap;
use std::{cmp::min, collections::hash_map::Entry, rc::Rc};

pub struct Output {
    pub item: Filter,
    pub n_wanted: i32,
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

pub trait Recipe {
    type In: Input;
    fn get_outputs(&self) -> &Vec<Output>;
    fn get_inputs(&self) -> &Vec<Self::In>;
}

macro_rules! impl_recipe {
    ($r:ident, $i:ident) => {
        impl Recipe for $r {
            type In = $i;
            fn get_outputs(&self) -> &Vec<Output> { &self.outputs }
            fn get_inputs(&self) -> &Vec<$i> { &self.inputs }
        }
    };
}

pub struct ResolvedInputs {
    pub n_sets: i32,
    pub items: Vec<Rc<Item>>,
}

struct InputInfo {
    n_available: i32,
    n_needed: i32,
    allow_backup: bool,
}

pub fn resolve_inputs(factory: &Factory, recipe: &impl Recipe) -> Option<ResolvedInputs> {
    let mut n_sets = i32::MAX;
    let mut items = Vec::new();
    items.reserve(recipe.get_inputs().len());
    let mut infos = FnvHashMap::<&Rc<Item>, InputInfo>::default();
    for input in recipe.get_inputs() {
        if let Some((item, item_info)) = factory.search_item(input.get_item()) {
            items.push(item.clone());
            match infos.entry(item) {
                Entry::Vacant(input_info) => {
                    input_info.insert(InputInfo {
                        n_available: item_info
                            .borrow()
                            .get_availability(input.get_allow_backup(), input.get_extra_backup()),
                        n_needed: input.get_size(),
                        allow_backup: input.get_allow_backup(),
                    });
                }
                Entry::Occupied(input_info) => {
                    let input_info = input_info.into_mut();
                    input_info.n_needed += input.get_size();
                    if input_info.allow_backup && !input.get_allow_backup() {
                        input_info.allow_backup = false;
                        input_info.n_available = item_info.borrow().get_availability(false, input.get_extra_backup())
                    }
                }
            }
            n_sets = min(n_sets, item.max_size / input.get_size());
        } else {
            return None;
        }
    }
    for (_, input_info) in infos.into_iter() {
        n_sets = min(n_sets, input_info.n_available / input_info.n_needed)
    }
    if n_sets > 0 {
        Some(ResolvedInputs { n_sets, items })
    } else {
        None
    }
}

pub struct Demand {
    pub i_recipe: usize,
    pub inputs: ResolvedInputs,
    fullness: f64,
}

pub fn compute_demands(factory: &Factory, recipes: &Vec<impl Recipe>) -> Vec<Demand> {
    let mut result = Vec::new();
    for (i_recipe, recipe) in recipes.iter().enumerate() {
        let mut fullness: f64 = 2.0;
        if !recipe.get_outputs().is_empty() {
            let mut full = true;
            for output in recipe.get_outputs() {
                if output.n_wanted <= 0 {
                    continue;
                }
                if let Some((_, info)) = factory.search_item(&output.item) {
                    let n_stored = info.borrow().n_stored;
                    if n_stored >= output.n_wanted {
                        continue;
                    }
                    full = false;
                    fullness = fullness.min(n_stored as f64 / output.n_wanted as f64)
                } else {
                    fullness = 0.0;
                    break;
                }
            }
            if full {
                continue;
            }
        }
        if let Some(inputs) = resolve_inputs(factory, recipe) {
            result.push(Demand { i_recipe, inputs, fullness })
        }
    }
    result.sort_unstable_by(|x: &Demand, y: &Demand| x.fullness.partial_cmp(&y.fullness).unwrap());
    result
}

pub struct SlottedInput {
    item: Filter,
    pub size: i32,
    pub slots: Vec<usize>,
    allow_backup: bool,
    extra_backup: i32,
}

impl SlottedInput {
    pub fn new(item: Filter, size: i32, slots: Vec<usize>) -> Self {
        SlottedInput { item, size, slots, allow_backup: false, extra_backup: 0 }
    }
}

impl_input!(SlottedInput);

pub struct NonConsumable {
    // for crafting robot:
    //   3, 7, 11, 12, 13, 14
    //   with extra inventory: 16, 17, ..
    pub storage_slot: usize,
    pub crafting_grid_slot: usize,
}

pub struct CraftingGridRecipe {
    pub outputs: Vec<Output>,
    // slots:
    //   0, 1, 2
    //   3, 4, 5
    //   6, 7, 8
    pub inputs: Vec<SlottedInput>,
    // can't craft more than one stack at a time.
    pub max_sets: i32,
    pub non_consumables: Vec<NonConsumable>,
}

impl_recipe!(CraftingGridRecipe, SlottedInput);
