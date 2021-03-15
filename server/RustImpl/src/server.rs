use super::lua_value::Parser;
use socket2::{Domain, SockAddr, Socket, Type};
use std::net::{Ipv6Addr, SocketAddr};
use std::{cell::RefCell, fmt::Display, rc::Rc, rc::Weak};
use tokio::io::AsyncReadExt;
use tokio::net::{tcp::OwnedReadHalf, TcpListener};
use tokio::task::{spawn_local, JoinHandle};

pub struct Server {
    clients: Option<Rc<RefCell<Client>>>,
    acceptor: JoinHandle<()>,
}

impl Drop for Server {
    fn drop(&mut self) {
        self.acceptor.abort();
        while let Some(client) = self.clients.take() {
            self.clients = Rc::try_unwrap(client)
                .map_err(|_| "client should be exclusively owned by server")
                .unwrap()
                .into_inner()
                .next
                .take()
        }
    }
}

struct Client {
    addr: SocketAddr,
    next: Option<Rc<RefCell<Client>>>,
    prev: Option<Weak<RefCell<Client>>>,
    server: Weak<RefCell<Server>>,
    reader: JoinHandle<()>,
}

impl Drop for Client {
    fn drop(&mut self) {
        self.log("disconnected");
        self.reader.abort()
    }
}

impl Client {
    fn log(&self, x: &(impl Display + ?Sized)) {
        println!("{}: {}", self.addr, x)
    }

    fn disconnect(&mut self) {
        if let Some(next) = self.next.as_ref() {
            next.borrow_mut().prev = self.prev.clone()
        }
        if let Some(prev) = self.prev.as_ref() {
            prev.upgrade().unwrap().borrow_mut().next = self.next.take()
        } else {
            self.server.upgrade().unwrap().borrow_mut().clients = self.next.take()
        }
    }
}

async fn reader_main(client: Weak<RefCell<Client>>, mut stream: OwnedReadHalf) {
    let mut data = [0; 4096];
    let mut parser = Parser::new();
    loop {
        let n_read = stream.read(&mut data).await;
        match client.upgrade() {
            None => break,
            Some(this) => {
                let mut this = this.borrow_mut();
                match n_read {
                    Err(e) => {
                        this.log(&format!("error reading: {}", e));
                        this.disconnect()
                    }
                    Ok(n_read) => {
                        let result = parser.shift(&data[..n_read], &mut |value| todo!());
                        if let Err(e) = result {
                            this.log(&format!("error decoding packet: {}", e));
                            this.disconnect()
                        }
                    }
                }
            }
        }
    }
}

fn create_listener(port: u16) -> TcpListener {
    let socket = Socket::new(Domain::ipv6(), Type::stream(), None).unwrap();
    socket.set_reuse_address(true).unwrap();
    socket.set_only_v6(false).unwrap();
    socket
        .bind(&SockAddr::from(SocketAddr::from((
            Ipv6Addr::UNSPECIFIED,
            port,
        ))))
        .unwrap();
    socket.set_nonblocking(true).unwrap();
    socket.listen(128).unwrap();
    TcpListener::from_std(socket.into_tcp_listener()).unwrap()
}

async fn acceptor_main(server: Weak<RefCell<Server>>, listener: TcpListener) {
    loop {
        let (stream, addr) = listener.accept().await.unwrap();
        match server.upgrade() {
            None => break,
            Some(this) => {
                let mut this = this.borrow_mut();
                let (r, w) = stream.into_split();
                this.clients = Some(Rc::new_cyclic(|weak| {
                    let client = Client {
                        addr,
                        next: this.clients.take(),
                        prev: None,
                        server: server.clone(),
                        reader: spawn_local(reader_main(weak.clone(), r)),
                    };
                    if let Some(ref next) = client.next {
                        next.borrow_mut().prev = Some(weak.clone())
                    }
                    client.log("connected");
                    RefCell::new(client)
                }))
            }
        }
    }
}

impl Server {
    pub fn new(port: u16) -> Rc<RefCell<Self>> {
        Rc::new_cyclic(|weak| {
            RefCell::new(Server {
                clients: None,
                acceptor: spawn_local(acceptor_main(weak.clone(), create_listener(port))),
            })
        })
    }
}
