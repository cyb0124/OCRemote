use super::item::{Filter, Item};
use flexstr::LocalStr;
use std::rc::Rc;

pub fn s(x: &'static str) -> LocalStr { LocalStr::from_static(x) }
pub fn label(x: &'static str) -> Filter { Filter::Label(s(x)) }
pub fn name(x: &'static str) -> Filter { Filter::Name(s(x)) }
pub fn both(label: &'static str, name: &'static str) -> Filter { Filter::Both { label: s(label), name: s(name) } }

pub fn custom(desc: &'static str, func: impl Fn(&Item) -> bool + 'static) -> Filter {
    Filter::Custom { desc: s(desc), func: Rc::new(func) }
}
