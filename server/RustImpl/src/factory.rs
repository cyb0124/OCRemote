use super::action::{ActionFuture, Print};
use super::server::{Access, Server};
use std::{
    cell::RefCell,
    rc::{Rc, Weak},
    time::Duration,
};
use tokio::{
    task::{spawn_local, JoinHandle},
    time::{sleep_until, Instant},
};

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

pub struct Factory {
    server: Rc<RefCell<Server>>,
    log_clients: Vec<&'static str>,
    bus_accesses: Vec<BusAccess>,
    task: JoinHandle<()>,
}

impl Drop for Factory {
    fn drop(&mut self) {
        self.task.abort()
    }
}

impl Factory {
    pub fn new(
        server: Rc<RefCell<Server>>,
        min_cycle_time: Duration,
        log_clients: Vec<&'static str>,
        bus_accesses: Vec<BusAccess>,
    ) -> Rc<RefCell<Factory>> {
        Rc::new_cyclic(|weak| {
            RefCell::new(Factory {
                server,
                log_clients,
                bus_accesses,
                task: spawn_local(factory_main(weak.clone(), min_cycle_time)),
            })
        })
    }

    fn log(&self, action: Print) {
        println!("{}", action.text);
        let action = ActionFuture::from(action);
        let server = self.server.borrow();
        for client in &self.log_clients {
            server.enqueue_request_group(client, vec![action.clone().into()]);
        }
    }
}

async fn factory_main(factory: Weak<RefCell<Factory>>, min_cycle_time: Duration) {
    let mut cycle_start_last: Option<Instant> = None;
    let n_cycles = 0usize;
    loop {
        let cycle_start_time = Instant::now();
        let mut text = format!("Cycle {}", n_cycles);
        if let Some(last) = cycle_start_last {
            text += &format!(
                ", lastCycleTime={:.03}",
                (cycle_start_time - last).as_secs_f32()
            )
        }
        if let Some(this) = factory.upgrade() {
            this.borrow().log(Print {
                text,
                color: 0xFFFFFF,
                beep: None,
            })
        } else {
            break;
        }

        // TODO:
        sleep_until(cycle_start_time + min_cycle_time).await;
        cycle_start_last = Some(cycle_start_time)
    }
}
