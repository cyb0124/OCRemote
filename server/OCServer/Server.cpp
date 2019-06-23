#include <iostream>
#include "Server.h"
#include "WeakCallback.h"

Server::Server(IOEnv &io, uint16_t port)
  :io(io), acceptor(io.io, boost::asio::ip::tcp::v6()) {
  acceptor.set_option(boost::asio::ip::tcp::socket::reuse_address(true));
  acceptor.bind({boost::asio::ip::address_v6::any(), port});
  acceptor.listen();
  accept();
}

void Server::accept() {
  auto socket(std::make_shared<boost::asio::ip::tcp::socket>(io.io));
  acceptor.async_accept(*socket, makeWeakCallback(alive, [this, socket](
    const boost::system::error_code &error
  ) {
    if (error)
      throw boost::system::system_error(error, "async_accept failed");
    accept();
    clients.emplace_front(std::make_shared<Client>(*this, std::move(*socket)))->init(clients.begin());
  }));
}

void Server::removeClient(Client &c) {
  if (c.getLogin())
    logins.erase(*c.getLogin());
  clients.erase(c.getItr());
}

Client::Client(Server &s, boost::asio::ip::tcp::socket socket)
  :s(s), socket(std::move(socket)), d(std::bind(&Client::onPacket, this, std::placeholders::_1)) {}

void Client::init(const Itr &x) {
  itr = x;
  const auto &host{socket.remote_endpoint()};
  logHeader = host.address().to_string() + ":" + std::to_string(host.port());
  log("connected");
  read();
}

Client::~Client() {
  log("disconnected");
  std::string failureCause(logHeader + " disconnected");

  if (!sendQueue.empty()) {
    s.io([sendQueue(std::move(sendQueue)), failureCause]() {
      for (auto &i : sendQueue) {
        for (auto &j : i) {
          j->onFail(failureCause);
        }
      }
    });
  }

  if (!responseQueue.empty()) {
    s.io([responseQueue(std::move(responseQueue)), failureCause]() {
      for (auto &i : responseQueue) {
        i->onFail(failureCause);
      }
    });
  }
}

void Client::log(const std::string &message) {
  std::cout << logHeader << ": " << message << std::endl;
}

void Client::read() {
  auto buffer(std::make_shared<std::array<char, BUFSIZ>>());
  socket.async_read_some(boost::asio::buffer(*buffer), makeWeakCallback(weak_from_this(), [this, buffer](
    const boost::system::error_code &ec, size_t nRead
  ) {
    if (ec) {
      log("error reading length: " + ec.message());
      s.removeClient(*this);
    } else {
      read();
      #ifndef NDEBUG
        std::cout << logHeader << " >=> ";
        std::cout.write(buffer->data(), nRead);
        std::cout << std::endl;
      #endif
      try {
        d(buffer->data(), nRead);
      } catch (std::exception &e) {
        log(std::string("error decoding packet: ") + e.what());
        s.removeClient(*this);
      }
    }
  }));
}

void Client::onPacket(SValue p) {
  if (login) {
    if (responseQueue.empty()) {
      log("unexpected packet");
      s.removeClient(*this);
    } else {
      responseQueue.front()->onResult(std::move(p));
      responseQueue.pop_front();
    }
  } else {
    login = std::get<std::string>(p);
    logHeader += "(" + *login + ")";
    log("logged in");
    s.updateLogin(*this);
  }
}

void Server::updateLogin(Client &c) {
  auto result(logins.emplace(*c.getLogin(), &c));
  if (!result.second) {
    Client *old = result.first->second;
    old->log("evicted");
    clients.erase(old->getItr());
    result.first->second = &c;
  }
}

void Server::enqueueActionGroup(const std::string &client, std::vector<SharedAction> actions) {
  auto itr(logins.find(client));
  if (itr == logins.end()) {
    io([actions(std::move(actions)), failureCause(client + " isn't connected")]() mutable {
      for (auto &i : actions) {
        i->onFail(std::move(failureCause));
      }
    });
  } else {
    itr->second->enqueueActionGroup(std::move(actions));
  }
}

void Server::enqueueAction(const std::string &client, SharedAction action) {
  std::vector<SharedAction> actions;
  actions.emplace_back(std::move(action));
  enqueueActionGroup(client, std::move(actions));
}

size_t Server::countPending(const std::string &client) const {
  auto itr(logins.find(client));
  if (itr == logins.end())
    return std::numeric_limits<size_t>::max();
  return itr->second->countPending();
}

void Client::send() {
  if (isSending || sendQueue.empty())
    return;
  auto &head(sendQueue.front());
  auto dumped(std::make_shared<std::string>());
  {
    std::vector<SValue> p;
    for (auto &action : head)
      action->dump(std::get<STable>(p.emplace_back(std::in_place_type<STable>)));
    *dumped = serialize(arrayToSTable(std::move(p)));
  }
  isSending = true;
  for (auto &action : head)
    responseQueue.emplace_back(std::move(action));
  sendQueueTotal -= head.size();
  sendQueue.pop_front();
  #ifndef NDEBUG
    std::cout << logHeader << " <=< " << *dumped << std::endl;
  #endif
  boost::asio::async_write(socket, boost::asio::buffer(*dumped),
    makeWeakCallback(weak_from_this(), [this, dumped](const boost::system::error_code &ec, size_t) {
      if (ec) {
        log("error writing: " + ec.message());
        s.removeClient(*this);
      } else {
        isSending = false;
        send();
      }
    })
  );
}

void Client::enqueueActionGroup(std::vector<SharedAction> actions) {
  sendQueue.emplace_front(std::move(actions));
  sendQueueTotal += actions.size();
  send();
}

size_t Client::countPending() const {
  return sendQueueTotal + responseQueue.size();
}
