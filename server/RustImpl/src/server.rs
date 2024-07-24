use super::access::Access;
use super::action::ActionRequest;
use super::lua_value::{serialize, vec_to_table, Parser, Value};
use super::util::spawn;
use abort_on_drop::ChildTask;
use flexstr::{local_fmt, LocalStr};
use fnv::FnvHashMap;
use socket2::{Domain, SockAddr, Socket, Type};
use std::{
    cell::RefCell,
    collections::VecDeque,
    fmt::Write,
    mem::replace,
    net::{Ipv6Addr, SocketAddr},
    rc::{Rc, Weak},
    time::Duration,
};
use tokio::{
    io::{AsyncReadExt, AsyncWriteExt},
    net::{tcp::OwnedReadHalf, tcp::OwnedWriteHalf, TcpListener},
    time::sleep,
};

pub struct Server {
    clients: Option<Rc<RefCell<Client>>>,
    logins: FnvHashMap<LocalStr, Weak<RefCell<Client>>>,
    _acceptor: ChildTask<()>,
}

impl Drop for Server {
    fn drop(&mut self) {
        while let Some(client) = self.clients.take() {
            self.clients = Rc::into_inner(client).unwrap().into_inner().next.take()
        }
    }
}

enum WriterState {
    NotWriting(OwnedWriteHalf),
    Writing { _task: ChildTask<()> },
    Invalid,
}

struct Client {
    weak: Weak<RefCell<Client>>,
    log_prefix: String,
    next: Option<Rc<RefCell<Client>>>,
    prev: Option<Weak<RefCell<Client>>>,
    server: Weak<RefCell<Server>>,
    login: Option<LocalStr>,
    _reader: ChildTask<()>,
    request_queue: VecDeque<Vec<Rc<RefCell<dyn ActionRequest>>>>,
    request_queue_size: usize,
    response_queue: VecDeque<Rc<RefCell<dyn ActionRequest>>>,
    writer: WriterState,
    timeout: Option<ChildTask<()>>,
}

impl Drop for Client {
    fn drop(&mut self) {
        self.log(format_args!("disconnected"));
        let message: LocalStr = [&self.log_prefix, " disconnected"].into_iter().collect();
        for x in self.request_queue.iter().flatten().chain(&self.response_queue) {
            x.borrow_mut().on_fail(message.clone())
        }
    }
}

impl Client {
    fn log(&self, args: std::fmt::Arguments) { println!("{}: {}", self.log_prefix, args) }
    fn disconnect(&mut self) { self.disconnect_by_server(&mut self.server.upgrade().unwrap().borrow_mut()); }
    fn disconnect_by_server(&mut self, server: &mut Server) {
        if let Some(login) = &self.login {
            server.logins.remove(login);
        }
        if let Some(next) = self.next.as_ref() {
            next.borrow_mut().prev = self.prev.clone()
        }
        if let Some(prev) = self.prev.as_ref() {
            prev.upgrade().unwrap().borrow_mut().next = self.next.take()
        } else {
            server.clients = self.next.take()
        }
    }

    fn log_and_disconnect(&mut self, args: std::fmt::Arguments) {
        self.log(args);
        self.disconnect()
    }

    fn enqueue_request_group(&mut self, group: Vec<Rc<RefCell<dyn ActionRequest>>>) {
        self.request_queue_size += group.len();
        self.request_queue.push_back(group);
        let writer = replace(&mut self.writer, WriterState::Invalid);
        if let WriterState::NotWriting(stream) = writer {
            self.update_timeout(false);
            self.writer = WriterState::Writing { _task: spawn(writer_main(self.weak.clone(), stream)) }
        } else {
            self.writer = writer
        }
    }

    fn update_timeout(&mut self, restart: bool) {
        if self.request_queue_size == 0 && self.response_queue.is_empty() {
            self.timeout = None
        } else if restart || self.timeout.is_none() {
            self.timeout = Some(spawn(timeout_main(self.weak.clone())))
        }
    }

    fn estimate_load(&self) -> usize { self.request_queue_size + self.response_queue.len() }
}

async fn timeout_main(client: Weak<RefCell<Client>>) {
    sleep(Duration::from_secs(30)).await;
    if let Some(this) = client.upgrade() {
        this.borrow_mut().log_and_disconnect(format_args!("request timeout"))
    }
}

async fn writer_main(client: Weak<RefCell<Client>>, mut stream: OwnedWriteHalf) {
    loop {
        let mut data = Vec::new();
        let Some(this) = client.upgrade() else { break };
        let mut this = this.borrow_mut();
        if let Some(group) = this.request_queue.pop_front() {
            this.request_queue_size -= group.len();
            let mut value = Vec::new();
            for x in group {
                value.push(x.borrow_mut().build_request());
                this.response_queue.push_back(x)
            }
            serialize(&vec_to_table(value).into(), &mut data);
            #[cfg(feature = "dump_traffic")]
            this.log(format_args!("out: {}", data.iter().map(|x| char::from(*x)).collect::<String>()));
        } else {
            this.writer = WriterState::NotWriting(stream);
            break;
        }
        if let Err(e) = stream.write_all(&data).await {
            if let Some(this) = client.upgrade() {
                this.borrow_mut().log_and_disconnect(format_args!("error writing: {}", e))
            }
            break;
        }
    }
}

fn on_packet(client: &Rc<RefCell<Client>>, value: Value) -> Result<(), LocalStr> {
    let mut this = client.borrow_mut();
    if this.login.is_some() {
        if let Some(x) = this.response_queue.pop_front() {
            this.update_timeout(true);
            x.borrow_mut().on_response(value)
        } else {
            Err(local_fmt!("unexpected packet: {:?}", value))
        }
    } else if let Value::S(login) = value {
        upgrade_mut!(this.server, server);
        write!(this.log_prefix, "[{}]", login).unwrap();
        this.log(format_args!("logged in"));
        this.login = Some(login.clone());
        drop(this);
        server.login(login, Rc::downgrade(client));
        Ok(())
    } else {
        Err(local_fmt!("invalid login packet: {:?}", value))
    }
}

async fn reader_main(client: Weak<RefCell<Client>>, mut stream: OwnedReadHalf) {
    let mut data = [0; 4096];
    let mut parser = Parser::new();
    loop {
        let n_read = stream.read(&mut data).await;
        let Some(this) = client.upgrade() else { break };
        match n_read {
            Err(e) => break this.borrow_mut().log_and_disconnect(format_args!("error reading: {}", e)),
            Ok(n_read) => {
                if n_read > 0 {
                    let data = &data[..n_read];
                    #[cfg(feature = "dump_traffic")]
                    this.borrow().log(format_args!("in: {}", data.iter().map(|x| char::from(*x)).collect::<String>()));
                    if let Err(e) = parser.shift(data, &mut |x| on_packet(&this, x)) {
                        this.borrow_mut().log_and_disconnect(format_args!("error decoding packet: {}", e));
                        break;
                    }
                } else {
                    this.borrow_mut().log_and_disconnect(format_args!("client disconnected"));
                    break;
                }
            }
        }
    }
}

fn create_listener(port: u16) -> TcpListener {
    let socket = Socket::new(Domain::IPV6, Type::STREAM, None).unwrap();
    socket.set_reuse_address(true).unwrap();
    socket.set_only_v6(false).unwrap();
    socket.bind(&SockAddr::from(SocketAddr::from((Ipv6Addr::UNSPECIFIED, port)))).unwrap();
    socket.set_nonblocking(true).unwrap();
    socket.listen(128).unwrap();
    TcpListener::from_std(socket.into()).unwrap()
}

async fn acceptor_main(server: Weak<RefCell<Server>>, listener: TcpListener) {
    loop {
        let (stream, addr) = listener.accept().await.unwrap();
        let Some(this) = server.upgrade() else { break };
        let mut this = this.borrow_mut();
        let (r, w) = stream.into_split();
        this.clients = Some(Rc::new_cyclic(|weak| {
            let client = Client {
                weak: weak.clone(),
                log_prefix: addr.to_string(),
                next: this.clients.take(),
                prev: None,
                server: server.clone(),
                login: None,
                _reader: spawn(reader_main(weak.clone(), r)),
                request_queue: VecDeque::new(),
                request_queue_size: 0,
                response_queue: VecDeque::new(),
                writer: WriterState::NotWriting(w),
                timeout: None,
            };
            if let Some(ref next) = client.next {
                next.borrow_mut().prev = Some(weak.clone())
            }
            client.log(format_args!("connected"));
            RefCell::new(client)
        }))
    }
}

impl Server {
    pub fn new(port: u16) -> Rc<RefCell<Self>> {
        Rc::new_cyclic(|weak| {
            RefCell::new(Server {
                clients: None,
                logins: FnvHashMap::default(),
                _acceptor: spawn(acceptor_main(weak.clone(), create_listener(port))),
            })
        })
    }

    fn login(&mut self, name: LocalStr, client: Weak<RefCell<Client>>) {
        if let Some(old) = self.logins.insert(name, client) {
            upgrade_mut!(old, old);
            old.log(format_args!("logged in from another address"));
            old.login = None;
            old.disconnect_by_server(self)
        }
    }

    pub fn enqueue_request_group(&self, client: &str, group: Vec<Rc<RefCell<dyn ActionRequest>>>) {
        if let Some(client) = self.logins.get(client) {
            client.upgrade().unwrap().borrow_mut().enqueue_request_group(group)
        } else {
            let reason = local_fmt!("{} isn't connected", client);
            for x in group {
                x.borrow_mut().on_fail(reason.clone())
            }
        }
    }

    fn estimate_load(&self, client: &str) -> usize {
        if let Some(client) = self.logins.get(client) {
            client.upgrade().unwrap().borrow().estimate_load()
        } else {
            usize::MAX
        }
    }

    pub fn load_balance<'a, T: Access>(&self, iter: impl IntoIterator<Item = &'a T>) -> (usize, &'a T) {
        let mut iter = iter.into_iter().enumerate();
        let mut best_access = iter.next().unwrap();
        let mut best_load = self.estimate_load(best_access.1.get_client());
        for access in iter {
            let load = self.estimate_load(access.1.get_client());
            if load < best_load {
                best_load = load;
                best_access = access
            }
        }
        best_access
    }
}
