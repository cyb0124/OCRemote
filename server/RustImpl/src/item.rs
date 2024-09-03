use super::lua_value::{table_remove, Table, Value};
use flexstr::LocalStr;
use std::{cmp::min, rc::Rc};

#[derive(PartialEq, Eq, Hash)]
pub struct Item {
    pub label: LocalStr,
    pub name: LocalStr,
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
        label: <_>::default(),
        name: <_>::default(),
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
    pub fn parse(value: Value) -> Result<Self, LocalStr> {
        let mut table: Table = value.try_into()?;
        let size = table_remove(&mut table, "size")?;
        let label = table_remove(&mut table, "label")?;
        let name = table_remove(&mut table, "name")?;
        let damage = table_remove(&mut table, "damage")?;
        let max_damage = table_remove(&mut table, "maxDamage")?;
        let max_size = table_remove(&mut table, "maxSize")?;
        let has_tag = table_remove(&mut table, "hasTag")?;
        Ok(ItemStack { item: Rc::new(Item { label, name, damage, max_damage, max_size, has_tag, others: table }), size })
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

#[derive(Clone)]
pub enum Filter {
    Label(LocalStr),
    Name(LocalStr),
    Both { label: LocalStr, name: LocalStr },
    Custom { desc: LocalStr, func: Rc<dyn Fn(&Item) -> bool> },
}

impl Filter {
    pub fn apply(&self, item: &Item) -> bool {
        match self {
            Filter::Label(label) => item.label == *label,
            Filter::Name(name) => item.name == *name,
            Filter::Both { label, name } => item.label == *label && item.name == *name,
            Filter::Custom { func, .. } => func(item),
        }
    }
}
