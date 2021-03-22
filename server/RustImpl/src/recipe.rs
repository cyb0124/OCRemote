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
                        input_info.n_available = item_info
                            .borrow()
                            .get_availability(false, input.get_extra_backup())
                    }
                }
            }
            n_sets = min(n_sets, item.max_size / input.get_size());
        } else {
            return None;
        }
    }
    for (item, input_info) in infos.into_iter() {
        n_sets = min(n_sets, input_info.n_available / input_info.n_needed)
    }
    if n_sets > 0 {
        Some(ResolvedInputs { n_sets, items })
    } else {
        None
    }
}

pub struct Demand {
    i_recipe: usize,
    fullness: f32,
    inputs: ResolvedInputs,
}

pub fn compute_demands(factory: &Factory, recipes: &Vec<impl Recipe>) -> Vec<Demand> {
    let mut result = Vec::new();
    for (i_recipe, recipe) in recipes.iter().enumerate() {
        let mut fullness: f32 = 2.0;
        if !recipe.get_outputs().is_empty() {
            let mut full = true;
            for output in recipe.get_outputs() {
                if let Some((_, info)) = factory.search_item(&output.item) {
                    let n_stored = info.borrow().n_stored;
                    if n_stored >= output.n_wanted {
                        continue;
                    }
                    full = false;
                    fullness = fullness.min(n_stored as f32 / output.n_wanted as f32)
                } else {
                    continue;
                }
            }
            if full {
                continue;
            }
        }
        if let Some(inputs) = resolve_inputs(factory, recipe) {
            result.push(Demand {
                i_recipe,
                fullness,
                inputs,
            })
        }
    }
    result.sort_unstable_by(|x: &Demand, y: &Demand| x.fullness.partial_cmp(&y.fullness).unwrap());
    result
}
