use super::{IntoProcess, Process};
use crate::access::{EachTank, TankAccess};
use crate::action::{ActionFuture, Call};
use crate::factory::{read_tanks, tanks_to_fluid_map, Factory};
use crate::recipe::FluidOutput;
use crate::util::{alive, join_tasks, spawn};
use abort_on_drop::ChildTask;
use flexstr::{local_str, LocalStr};
use std::{
    cell::RefCell,
    rc::{Rc, Weak},
};

pub struct BlockingFluidOutputConfig {
    pub accesses: Vec<TankAccess>,
    pub outputs: Vec<FluidOutput>,
}

pub struct BlockingFluidOutputProcess {
    weak: Weak<RefCell<BlockingFluidOutputProcess>>,
    config: BlockingFluidOutputConfig,
    factory: Weak<RefCell<Factory>>,
}

impl IntoProcess for BlockingFluidOutputConfig {
    type Output = BlockingFluidOutputProcess;
    fn into_process(self, factory: &Factory) -> Rc<RefCell<Self::Output>> {
        Rc::new_cyclic(|weak| RefCell::new(Self::Output { weak: weak.clone(), config: self, factory: factory.weak.clone() }))
    }
}

impl Process for BlockingFluidOutputProcess {
    fn run(&self, factory: &Factory) -> ChildTask<Result<(), LocalStr>> {
        let mut enough = true;
        for output in &self.config.outputs {
            if factory.search_n_fluid(&output.fluid) < output.n_wanted {
                enough = false;
                break;
            }
        }
        if enough {
            return spawn(async { Ok(()) });
        }
        let tanks = read_tanks(&*factory.borrow_server(), &self.config.accesses, |access| EachTank {
            addr: access.buses[0].addr.clone(),
            side: access.buses[0].tank_side,
        });
        let weak = self.weak.clone();
        spawn(async move {
            let tanks = tanks.await?;
            let mut tasks = Vec::new();
            {
                alive!(weak, this);
                upgrade_mut!(this.factory, factory);
                let tanks = tanks_to_fluid_map(&tanks);
                for output in &this.config.outputs {
                    let Some(&(_, slot)) = tanks.get(&output.fluid) else { continue };
                    let n_stored = factory.search_n_fluid(&output.fluid);
                    let qty = output.n_wanted - n_stored;
                    if qty > 0 {
                        let weak = weak.clone();
                        tasks.push(spawn(async move {
                            let bus = {
                                alive!(weak, this);
                                upgrade_mut!(this.factory, factory);
                                factory.fluid_bus_allocate()
                            };
                            let bus = bus.await?;
                            let task;
                            {
                                alive!(weak, this);
                                upgrade!(this.factory, factory);
                                let server = factory.borrow_server();
                                let access = server.load_balance(&this.config.accesses).1;
                                let bus_of_tank = &access.buses[bus];
                                task = ActionFuture::from(Call {
                                    addr: bus_of_tank.addr.clone(),
                                    func: local_str!("transferFluid"),
                                    args: vec![bus_of_tank.tank_side.into(), bus_of_tank.bus_side.into(), qty.into(), (slot + 1).into()],
                                });
                                server.enqueue_request_group(&access.client, vec![task.clone().into()])
                            }
                            let result = task.await.map(|_| ());
                            alive(&weak)?.borrow().factory.upgrade().unwrap().borrow_mut().fluid_bus_deposit([bus]);
                            result
                        }));
                    }
                }
            }
            join_tasks(tasks).await
        })
    }
}
