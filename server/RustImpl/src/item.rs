use super::lua_value::{Key, Table, Value};
use num_traits::cast::FromPrimitive;
use ordered_float::NotNan;
use std::{convert::TryInto, rc::Rc};

#[derive(PartialEq, Eq, Hash)]
struct Item {
    name: String,
    label: String,
    damage: i16,
    max_damage: i16,
    max_size: i32,
    has_tag: bool,
    others: Table,
}

impl Item {
    fn serialize(&self) -> Value {
        let mut result = self.others.clone();
        result.insert(Key::S("name".to_owned()), Value::S(self.name.clone()));
        result.insert(Key::S("label".to_owned()), Value::S(self.label.clone()));
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

struct ItemStack {
    item: Rc<Item>,
    size: i32,
}

impl ItemStack {
    fn parse(value: Value) -> Result<Self, String> {
        if let Value::T(mut table) = value {
            let mut get = |key: &'static str| {
                table
                    .remove(&Key::S(key.to_owned()))
                    .ok_or_else(|| format!("key not found: {}", key))
            };
            let size = get("size")?.try_into()?;
            let name = get("name")?.try_into()?;
            let label = get("label")?.try_into()?;
            let damage = get("damage")?.try_into()?;
            let max_damage = get("maxDamage")?.try_into()?;
            let max_size = get("maxSize")?.try_into()?;
            let has_tag = get("hasTag")?.try_into()?;
            Ok(ItemStack {
                item: Rc::new(Item {
                    name,
                    label,
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
