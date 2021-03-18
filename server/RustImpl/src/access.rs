pub trait Access {
    fn get_client(&self) -> &str;
}

pub struct BusAccess {
    pub client: &'static str,
    pub addr: &'static str,
    pub side: u8,
}

impl Access for BusAccess {
    fn get_client(&self) -> &str {
        self.client
    }
}

pub struct InvAccess {
    pub client: &'static str,
    pub addr: &'static str,
    pub bus_side: u8,
    pub inv_side: u8,
}

impl Access for InvAccess {
    fn get_client(&self) -> &str {
        self.client
    }
}
