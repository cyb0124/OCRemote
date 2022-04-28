use flexstr::LocalStr;

pub trait Access {
    fn get_client(&self) -> &str;
}

macro_rules! impl_access {
    ($i:ident) => {
        impl Access for $i {
            fn get_client(&self) -> &str { &*self.client }
        }
    };
}

pub struct SidedAccess {
    pub client: LocalStr,
    pub addr: LocalStr,
    pub side: u8,
}

impl_access!(SidedAccess);

pub struct InvAccess {
    pub client: LocalStr,
    pub addr: LocalStr,
    pub bus_side: u8,
    pub inv_side: u8,
}

impl_access!(InvAccess);

pub struct MEAccess {
    pub client: LocalStr,
    pub transposer_addr: LocalStr,
    pub me_addr: LocalStr,
    pub bus_side: u8,
    pub me_side: u8,
    // 0-7 are valid. 8 is for deposit.
    pub me_slot: usize,
}

impl_access!(MEAccess);

pub struct ComponentAccess {
    pub client: LocalStr,
    // typical address for reactor: br_reactor
    // typical address for plastic mixer: plastic_mixer
    // typical address for flux controller: flux_controller
    pub addr: LocalStr,
}

impl_access!(ComponentAccess);

pub struct CraftingRobotAccess {
    pub client: LocalStr,
    pub bus_side: u8,
}

impl_access!(CraftingRobotAccess);

pub struct WorkbenchAccess {
    pub client: LocalStr,
    pub input_addr: LocalStr,
    pub output_addr: LocalStr,
    pub input_bus_side: u8,
    pub output_bus_side: u8,
    pub non_consumable_side: u8,
}

impl_access!(WorkbenchAccess);
