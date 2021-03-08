use socket2::{Domain, SockAddr, Socket, Type};
use std::net::{Ipv6Addr, SocketAddr};
use std::{cell::RefCell, rc::Rc, rc::Weak};
use tokio::net::{tcp::OwnedReadHalf, TcpListener};
use tokio::task::{spawn_local, JoinHandle};

pub struct Server {
    clients: Option<Rc<RefCell<Client>>>,
    acceptor: JoinHandle<()>,
}

impl Drop for Server {
    fn drop(&mut self) {
        self.acceptor.abort();
    }
}

struct Client {
    addr: SocketAddr,
    next: Option<Rc<RefCell<Client>>>,
    prev: Option<Weak<RefCell<Client>>>,
    reader: JoinHandle<()>,
}

impl Drop for Client {
    fn drop(&mut self) {
        self.reader.abort()
    }
}

async fn reader_main(this: Weak<RefCell<Client>>, stream: OwnedReadHalf) {
    todo!()
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

async fn acceptor_main(this: Weak<RefCell<Server>>, listener: TcpListener) {
    loop {
        let (stream, addr) = listener.accept().await.unwrap();
        match this.upgrade() {
            None => break,
            Some(this) => {
                let mut this = this.borrow_mut();
                let (r, w) = stream.into_split();
                this.clients = Some(Rc::new_cyclic(|weak| {
                    let client = Client {
                        addr,
                        next: this.clients.take(),
                        prev: None,
                        reader: spawn_local(reader_main(weak.clone(), r)),
                    };
                    if let Some(ref next) = client.next {
                        next.borrow_mut().prev = Some(weak.clone())
                    }
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
