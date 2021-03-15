use fnv::FnvHashMap;
use std::{fmt::Display, io::Write, str::from_utf8};

#[derive(PartialEq, Eq, Hash, Debug)]
pub enum Key {
    I(isize),
    S(String),
    B(bool),
}

pub type Table = FnvHashMap<Key, Value>;

#[derive(Debug)]
pub enum Value {
    N,
    F(f64),
    S(String),
    B(bool),
    T(Table),
}

pub fn vec_to_table(vec: Vec<Value>) -> Table {
    let mut result = Table::default();
    for (i, x) in vec.into_iter().enumerate() {
        result.insert(Key::I((i + 1) as isize), x);
    }
    result
}

pub fn table_to_vec(table: Table) -> Result<Vec<Value>, String> {
    let mut result = Vec::new();
    for (k, v) in table.into_iter() {
        if let Key::I(k) = k {
            let k = k - 1;
            if k < 0 {
                return Err(format!("negative index: {}", k));
            }
            let k = k as usize;
            if result.len() <= k {
                result.resize_with(k + 1, || Value::N)
            }
            result[k] = v
        } else {
            return Err(format!("non-integer index: {:?}", k));
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

fn serialize_bool(x: bool, out: &mut Vec<u8>) {
    out.push(if x { b'+' } else { b'-' })
}

fn serialize_num(x: &impl Display, out: &mut Vec<u8>) {
    out.push(b'#');
    write!(out, "{}", x).unwrap();
    out.push(b'@')
}

fn serialize_key(x: &Key, out: &mut Vec<u8>) {
    match x {
        Key::I(x) => serialize_num(x, out),
        Key::S(x) => serialize_string(x, out),
        Key::B(x) => serialize_bool(*x, out),
    }
}

pub fn serialize(x: &Value, out: &mut Vec<u8>) {
    match x {
        Value::N => out.push(b'!'),
        Value::F(x) => serialize_num(x, out),
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

pub struct Parser(Vec<State>);

impl Parser {
    pub fn new() -> Self {
        Parser(vec![State::V])
    }

    fn reduce<T>(&mut self, mut value: Value, handler: &mut T) -> Result<(), String>
    where
        T: FnMut(Value) -> Result<(), String>,
    {
        loop {
            match self.0.pop() {
                None => {
                    handler(value)?;
                    self.0.push(State::V)
                }
                Some(State::T { mut result, key }) => {
                    if let Some(key) = key {
                        result.insert(key, value);
                        self.0.push(State::T { result, key: None });
                        self.0.push(State::V)
                    } else {
                        match value {
                            Value::N => {
                                value = Value::T(result);
                                continue;
                            }
                            Value::F(x) => {
                                let i = x as isize;
                                if i as f64 != x {
                                    break Err(format!("non-integer key: {}", x));
                                }
                                self.0.push(State::T {
                                    result,
                                    key: Some(Key::I(i)),
                                })
                            }
                            Value::S(x) => self.0.push(State::T {
                                result,
                                key: Some(Key::S(x)),
                            }),
                            Value::B(x) => self.0.push(State::T {
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
            match self.0.pop().unwrap() {
                State::V => {
                    let (x, rem) = data.split_first().unwrap();
                    data = rem;
                    match x {
                        b'!' => self.reduce(Value::N, handler)?,
                        b'#' => self.0.push(State::F(Vec::new())),
                        b'@' => self.0.push(State::S {
                            result: Vec::new(),
                            escape: false,
                        }),
                        b'+' => self.reduce(Value::B(true), handler)?,
                        b'-' => self.reduce(Value::B(false), handler)?,
                        b'=' => {
                            self.0.push(State::T {
                                result: Table::default(),
                                key: None,
                            });
                            self.0.push(State::V)
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
                    self.0.push(State::F(result))
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
                    self.0.push(State::S { result, escape })
                }
                _ => unreachable!(),
            }
        }
        Ok(())
    }
}
