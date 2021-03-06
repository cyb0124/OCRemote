use super::lua_value::{Table, Value};
use std::{cmp::min, convert::TryInto, rc::Rc};

#[derive(PartialEq, Eq, Hash)]
pub struct Item {
    pub label: String,
    pub name: String,
    pub damage: i16,
    pub max_damage: i16,
    pub max_size: i32,
    pub has_tag: bool,
    pub others: Table,
}

impl Item {
    pub fn serialize(&self) -> Value {
        let mut result = self.others.clone();
        result.insert("label".into(), self.label.clone().into());
        result.insert("name".into(), self.name.clone().into());
        result.insert("damage".into(), self.damage.into());
        result.insert("maxDamage".into(), self.max_damage.into());
        result.insert("maxSize".into(), self.max_size.into());
        result.insert("hasTag".into(), self.has_tag.into());
        result.into()
    }
}

pub fn jammer() -> Rc<Item> {
    thread_local!(static ITEM: Rc<Item> = Rc::new(Item {
        label: String::new(),
        name: String::new(),
        damage: 0,
        max_damage: 0,
        max_size: 1,
        has_tag: false,
        others: Table::new()
    }));
    ITEM.with(|item| item.clone())
}

#[derive(Clone)]
pub struct ItemStack {
    pub item: Rc<Item>,
    pub size: i32,
}

impl ItemStack {
    pub fn parse(value: Value) -> Result<Self, String> {
        let mut table: Table = value.try_into()?;
        let mut get = |key: &'static str| table.remove(&key.into()).ok_or_else(|| format!("key not found: {}", key));
        let size = get("size")?.try_into()?;
        let label = get("label")?.try_into()?;
        let name = get("name")?.try_into()?;
        let damage = get("damage")?.try_into()?;
        let max_damage = get("maxDamage")?.try_into()?;
        let max_size = get("maxSize")?.try_into()?;
        let has_tag = get("hasTag")?.try_into()?;
        Ok(ItemStack {
            item: Rc::new(Item { label, name, damage, max_damage, max_size, has_tag, others: table }),
            size,
        })
    }
}

pub struct InsertPlan {
    pub n_inserted: i32,
    pub insertions: Vec<(usize, i32)>,
}

pub fn insert_into_inventory(inventory: &mut Vec<Option<ItemStack>>, item: &Rc<Item>, to_insert: i32) -> InsertPlan {
    let mut result = InsertPlan { n_inserted: 0, insertions: Vec::new() };
    let mut remaining = min(to_insert, item.max_size);
    let mut first_empty_slot = None;
    for (slot, stack) in inventory.iter_mut().enumerate() {
        if remaining <= 0 {
            return result;
        }
        if let Some(stack) = stack {
            if stack.item == *item {
                let to_insert = min(remaining, item.max_size - stack.size);
                if to_insert > 0 {
                    stack.size += to_insert;
                    result.n_inserted += to_insert;
                    result.insertions.push((slot, to_insert));
                    remaining -= to_insert
                }
            }
        } else if first_empty_slot.is_none() {
            first_empty_slot = Some(slot)
        }
    }
    if remaining > 0 {
        if let Some(slot) = first_empty_slot {
            inventory[slot] = Some(ItemStack { item: item.clone(), size: remaining });
            result.n_inserted += remaining;
            result.insertions.push((slot, remaining))
        }
    }
    result
}

pub enum Filter {
    Label(&'static str),
    Name(&'static str),
    Both { label: &'static str, name: &'static str },
    Fn(Box<dyn Fn(&Item) -> bool>),
}

impl Filter {
    pub fn apply(&self, item: &Item) -> bool {
        match self {
            Filter::Label(label) => item.label == *label,
            Filter::Name(name) => item.name == *name,
            Filter::Both { label, name } => item.label == *label && item.name == *name,
            Filter::Fn(filter) => filter(item),
        }
    }
}
