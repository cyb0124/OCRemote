use num_traits::cast::{AsPrimitive, FromPrimitive};
use ordered_float::NotNan;
use std::{collections::BTreeMap, convert::TryFrom, io::Write, str::from_utf8};

fn try_into_integer<I>(f: f64) -> Result<I, String>
where
    f64: AsPrimitive<I>,
    I: AsPrimitive<f64>,
{
    let i = f.as_();
    if i.as_() == f {
        Ok(i)
    } else {
        Err(format!("non-integer: {}", f))
    }
}

#[derive(PartialEq, Eq, PartialOrd, Ord, Hash, Clone, Debug)]
pub enum Key {
    F(NotNan<f64>),
    S(String),
    B(bool),
}

impl From<usize> for Key {
    fn from(number: usize) -> Key { Key::F(NotNan::from_usize(number).unwrap()) }
}

impl From<&str> for Key {
    fn from(string: &str) -> Key { Key::S(string.to_owned()) }
}

pub type Table = BTreeMap<Key, Value>;

#[derive(PartialEq, Eq, PartialOrd, Ord, Hash, Clone, Debug)]
pub enum Value {
    N,
    F(NotNan<f64>),
    S(String),
    B(bool),
    T(Table),
}

impl From<u8> for Value {
    fn from(number: u8) -> Value { Value::F(NotNan::from_u8(number).unwrap()) }
}

impl From<i16> for Value {
    fn from(number: i16) -> Value { Value::F(NotNan::from_i16(number).unwrap()) }
}

impl From<i32> for Value {
    fn from(number: i32) -> Value { Value::F(NotNan::from_i32(number).unwrap()) }
}

impl From<u32> for Value {
    fn from(number: u32) -> Value { Value::F(NotNan::from_u32(number).unwrap()) }
}

impl From<usize> for Value {
    fn from(number: usize) -> Value { Value::F(NotNan::from_usize(number).unwrap()) }
}

impl From<f64> for Value {
    fn from(number: f64) -> Value { Value::F(NotNan::new(number).unwrap()) }
}

impl From<&str> for Value {
    fn from(string: &str) -> Value { Value::S(string.to_owned()) }
}

impl From<String> for Value {
    fn from(string: String) -> Value { Value::S(string) }
}

impl From<bool> for Value {
    fn from(boolean: bool) -> Value { Value::B(boolean) }
}

impl From<Table> for Value {
    fn from(table: Table) -> Value { Value::T(table) }
}

impl TryFrom<Value> for NotNan<f64> {
    type Error = String;

    fn try_from(value: Value) -> Result<Self, String> {
        if let Value::F(result) = value {
            Ok(result)
        } else {
            Err(format!("non-numeric: {:?}", value))
        }
    }
}

impl TryFrom<Value> for i32 {
    type Error = String;

    fn try_from(value: Value) -> Result<Self, String> {
        try_into_integer(NotNan::try_from(value)?.into_inner())
    }
}

impl TryFrom<Value> for i16 {
    type Error = String;

    fn try_from(value: Value) -> Result<Self, String> {
        try_into_integer(NotNan::try_from(value)?.into_inner())
    }
}

impl TryFrom<Value> for String {
    type Error = String;

    fn try_from(value: Value) -> Result<Self, String> {
        if let Value::S(result) = value {
            Ok(result)
        } else {
            Err(format!("non-string: {:?}", value))
        }
    }
}

impl TryFrom<Value> for bool {
    type Error = String;

    fn try_from(value: Value) -> Result<Self, String> {
        if let Value::B(result) = value {
            Ok(result)
        } else {
            Err(format!("non-boolean: {:?}", value))
        }
    }
}

impl TryFrom<Value> for Table {
    type Error = String;

    fn try_from(value: Value) -> Result<Self, String> {
        if let Value::T(result) = value {
            Ok(result)
        } else {
            Err(format!("non-table: {:?}", value))
        }
    }
}

pub fn vec_to_table(vec: Vec<Value>) -> Table {
    let mut result = Table::new();
    for (i, x) in vec.into_iter().enumerate() {
        result.insert((i + 1).into(), x);
    }
    result
}

pub fn table_to_vec(table: Table) -> Result<Vec<Value>, String> {
    let mut result = Vec::new();
    for (k, v) in table.into_iter() {
        if let Key::F(k) = k {
            let i = try_into_integer(k.into_inner() - 1.0)?;
            if result.len() <= i {
                result.resize_with(i + 1, || Value::N)
            }
            result[i] = v
        } else {
            return Err(format!("non-numeric index: {:?}", k));
        }
    }
    Ok(result)
}

fn serialize_string(x: &str, out: &mut Vec<u8>) {
    out.push(b'@');
    for i in x.as_bytes() {
        out.push(*i);
        if *i == b'@' {
            out.push(b'.')
        }
    }
    out.extend_from_slice(b"@!")
}

fn serialize_bool(x: bool, out: &mut Vec<u8>) { out.push(if x { b'+' } else { b'-' }) }

fn serialize_num(x: NotNan<f64>, out: &mut Vec<u8>) {
    out.push(b'#');
    write!(out, "{}", x).unwrap();
    out.push(b'@')
}

fn serialize_key(x: &Key, out: &mut Vec<u8>) {
    match x {
        Key::F(x) => serialize_num(*x, out),
        Key::S(x) => serialize_string(x, out),
        Key::B(x) => serialize_bool(*x, out),
    }
}

pub fn serialize(x: &Value, out: &mut Vec<u8>) {
    match x {
        Value::N => out.push(b'!'),
        Value::F(x) => serialize_num(*x, out),
        Value::S(x) => serialize_string(x, out),
        Value::B(x) => serialize_bool(*x, out),
        Value::T(x) => {
            out.push(b'=');
            for (k, v) in x.iter() {
                serialize_key(k, out);
                serialize(v, out)
            }
            out.push(b'!')
        }
    }
}

enum State {
    V,
    T { result: Table, key: Option<Key> },
    F(Vec<u8>),
    S { result: Vec<u8>, escape: bool },
}

pub struct Parser {
    stack: Vec<State>,
}

impl Parser {
    pub fn new() -> Self {
        Parser {
            stack: vec![State::V],
        }
    }

    fn reduce<T>(&mut self, mut value: Value, handler: &mut T) -> Result<(), String>
    where
        T: FnMut(Value) -> Result<(), String>,
    {
        loop {
            match self.stack.pop() {
                None => {
                    handler(value)?;
                    self.stack.push(State::V)
                }
                Some(State::T { mut result, key }) => {
                    if let Some(key) = key {
                        result.insert(key, value);
                        self.stack.push(State::T { result, key: None });
                        self.stack.push(State::V)
                    } else {
                        match value {
                            Value::N => {
                                value = Value::T(result);
                                continue;
                            }
                            Value::F(x) => self.stack.push(State::T {
                                result,
                                key: Some(Key::F(x)),
                            }),
                            Value::S(x) => self.stack.push(State::T {
                                result,
                                key: Some(Key::S(x)),
                            }),
                            Value::B(x) => self.stack.push(State::T {
                                result,
                                key: Some(Key::B(x)),
                            }),
                            Value::T(x) => break Err(format!("table key: {:?}", x)),
                        }
                    }
                }
                _ => unreachable!(),
            }
            break Ok(());
        }
    }

    pub fn shift<T>(&mut self, mut data: &[u8], handler: &mut T) -> Result<(), String>
    where
        T: FnMut(Value) -> Result<(), String>,
    {
        'outer: while data.len() > 0 {
            match self.stack.pop().unwrap() {
                State::V => {
                    let (x, rem) = data.split_first().unwrap();
                    data = rem;
                    match x {
                        b'!' => self.reduce(Value::N, handler)?,
                        b'#' => self.stack.push(State::F(Vec::new())),
                        b'@' => self.stack.push(State::S {
                            result: Vec::new(),
                            escape: false,
                        }),
                        b'+' => self.reduce(Value::B(true), handler)?,
                        b'-' => self.reduce(Value::B(false), handler)?,
                        b'=' => {
                            self.stack.push(State::T {
                                result: Table::new(),
                                key: None,
                            });
                            self.stack.push(State::V)
                        }
                        x => return Err(format!("invalid tag: {}", x)),
                    }
                }
                State::F(mut result) => {
                    while let Some((x, rem)) = data.split_first() {
                        data = rem;
                        if *x == b'@' {
                            self.reduce(
                                Value::F(
                                    from_utf8(&result)
                                        .map_err(|e| format!("non utf-8 serialized number: {}", e))?
                                        .parse()
                                        .map_err(|e| format!("invalid serialized number: {}", e))?,
                                ),
                                handler,
                            )?;
                            continue 'outer;
                        } else {
                            result.push(*x)
                        }
                    }
                    self.stack.push(State::F(result))
                }
                State::S {
                    mut result,
                    mut escape,
                } => {
                    while let Some((x, rem)) = data.split_first() {
                        data = rem;
                        if escape {
                            match *x {
                                b'.' => {
                                    result.push(b'@');
                                    escape = false
                                }
                                b'~' => {
                                    self.reduce(
                                        Value::S(String::from_utf8(result).map_err(|e| {
                                            format!("non utf-8 serialized string: {}", e)
                                        })?),
                                        handler,
                                    )?;
                                    continue 'outer;
                                }
                                x => return Err(format!("unknown escape: {}", x)),
                            }
                        } else if *x == b'@' {
                            escape = true
                        } else {
                            result.push(*x)
                        }
                    }
                    self.stack.push(State::S { result, escape })
                }
                _ => unreachable!(),
            }
        }
        Ok(())
    }
}
