pub trait Access {
    fn get_client(&self) -> &str;
}

macro_rules! impl_access {
    ($i:ident) => {
        impl Access for $i {
            fn get_client(&self) -> &str { self.client }
        }
    };
}

pub struct BusAccess {
    pub client: &'static str,
    pub addr: &'static str,
    pub side: u8,
}

impl_access!(BusAccess);

pub struct InvAccess {
    pub client: &'static str,
    pub addr: &'static str,
    pub bus_side: u8,
    pub inv_side: u8,
}

impl_access!(InvAccess);

pub struct MEAccess {
    pub client: &'static str,
    pub transposer_addr: &'static str,
    pub me_addr: &'static str,
    pub bus_side: u8,
    pub me_side: u8,
    // 0-7 are valid. 8 is for deposit.
    pub me_slot: usize,
}

impl_access!(MEAccess);

pub struct ComponentAccess {
    pub client: &'static str,
    // typical address for reactor: br_reactor
    pub addr: &'static str,
}

impl_access!(ComponentAccess);

pub struct CraftingRobotAccess {
    pub client: &'static str,
    pub bus_side: u8,
}

impl_access!(CraftingRobotAccess);

pub struct WorkbenchAccess {
    pub client: &'static str,
    pub input_addr: &'static str,
    pub output_addr: &'static str,
    pub input_bus_side: u8,
    pub output_bus_side: u8,
    pub non_consumable_side: u8,
}

impl_access!(WorkbenchAccess);
