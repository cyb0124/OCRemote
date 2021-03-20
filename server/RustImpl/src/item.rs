use super::lua_value::{Key, Table, Value};
use num_traits::cast::FromPrimitive;
use ordered_float::NotNan;
use std::{cmp::min, convert::TryInto, rc::Rc};

#[derive(PartialEq, Eq, Hash)]
pub struct Item {
    pub label: String,
    pub name: String,
    damage: i16,
    max_damage: i16,
    max_size: i32,
    has_tag: bool,
    others: Table,
}

impl Item {
    fn serialize(&self) -> Value {
        let mut result = self.others.clone();
        result.insert(Key::S("label".to_owned()), Value::S(self.label.clone()));
        result.insert(Key::S("name".to_owned()), Value::S(self.name.clone()));
        result.insert(
            Key::S("damage".to_owned()),
            Value::F(NotNan::from_i16(self.damage).unwrap()),
        );
        result.insert(
            Key::S("maxDamage".to_owned()),
            Value::F(NotNan::from_i16(self.max_damage).unwrap()),
        );
        result.insert(
            Key::S("maxSize".to_owned()),
            Value::F(NotNan::from_i32(self.max_size).unwrap()),
        );
        result.insert(Key::S("hasTag".to_owned()), Value::B(self.has_tag));
        Value::T(result)
    }
}

pub struct ItemStack {
    item: Rc<Item>,
    size: i32,
}

impl ItemStack {
    pub fn parse(value: Value) -> Result<Self, String> {
        if let Value::T(mut table) = value {
            let mut get = |key: &'static str| {
                table
                    .remove(&Key::S(key.to_owned()))
                    .ok_or_else(|| format!("key not found: {}", key))
            };
            let size = get("size")?.try_into()?;
            let label = get("label")?.try_into()?;
            let name = get("name")?.try_into()?;
            let damage = get("damage")?.try_into()?;
            let max_damage = get("maxDamage")?.try_into()?;
            let max_size = get("maxSize")?.try_into()?;
            let has_tag = get("hasTag")?.try_into()?;
            Ok(ItemStack {
                item: Rc::new(Item {
                    label,
                    name,
                    damage,
                    max_damage,
                    max_size,
                    has_tag,
                    others: table,
                }),
                size,
            })
        } else {
            Err(format!("non-table ItemStack: {:?}", value))
        }
    }
}

struct InsertResult {
    n_inserted: i32,
    actions: Vec<(usize, i32)>,
}

fn insert_into_inventory(
    inventory: &mut Vec<Option<ItemStack>>,
    item: &Rc<Item>,
    to_insert: i32,
) -> InsertResult {
    let mut result = InsertResult {
        n_inserted: 0,
        actions: Vec::new(),
    };
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
                    result.actions.push((slot, to_insert));
                    remaining -= to_insert
                }
            }
        } else if first_empty_slot.is_none() {
            first_empty_slot = Some(slot)
        }
    }
    if remaining > 0 {
        if let Some(slot) = first_empty_slot {
            inventory[slot] = Some(ItemStack {
                item: item.clone(),
                size: remaining,
            });
            result.n_inserted += remaining;
            result.actions.push((slot, remaining))
        }
    }
    result
}

pub enum Filter {
    Label(&'static str),
    Name(&'static str),
    Both {
        label: &'static str,
        name: &'static str,
    },
    Fn(Box<dyn Fn(&Item) -> bool>),
}

impl Filter {
    fn apply(&self, item: &Item) -> bool {
        match self {
            Filter::Label(label) => item.label == *label,
            Filter::Name(name) => item.name == *name,
            Filter::Both { label, name } => item.label == *label && item.name == *name,
            Filter::Fn(filter) => filter(item),
        }
    }
}
