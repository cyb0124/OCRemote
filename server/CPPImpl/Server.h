#ifndef _SERVER_H_
#define _SERVER_H_
#include <list>
#include <boost/asio.hpp>
#include <boost/noncopyable.hpp>
#include "Actions.h"

struct Server;
struct Client;

struct IOEnv {
  boost::asio::io_context io;
  boost::asio::io_context::strand strand;
  IOEnv() :io(), strand(io) {}
  template<typename T>
  void operator()(T &&fn) {
    boost::asio::post(strand, std::forward<T>(fn));
  }
};

struct Client : std::enable_shared_from_this<Client> {
  using Itr = std::list<std::shared_ptr<Client>>::iterator;
  static constexpr auto timeout() { return std::chrono::seconds{120}; }
private:
  Server &s;
  Itr itr;
  boost::asio::ip::tcp::socket socket;
  std::optional<std::string> login;
  std::string logHeader;
  std::list<std::vector<SharedAction>> sendQueue;
  std::list<SharedAction> responseQueue;
  std::shared_ptr<boost::asio::steady_timer> responseTimer;
  bool isSending = false;
  size_t sendQueueTotal{};
  Deserializer d;
  void onPacket(SValue);
  void read();
  void send();
  void updateTimer();
public:
  ~Client();
  Client(Server &s, boost::asio::ip::tcp::socket socket);
  const auto &getLogin() const { return login; }
  const auto &getItr() const { return itr; }
  void init(const Itr&);
  void log(const std::string &message);
  void enqueueActionGroup(std::vector<SharedAction> actions);
  size_t countPending() const;
};

struct Access {
  std::string client;
  Access(std::string client)
    :client(std::move(client)) {}
};

struct Server {
  IOEnv &io;
private:
  std::shared_ptr<std::monostate> alive{std::make_shared<std::monostate>()};
  boost::asio::ip::tcp::acceptor acceptor;
  std::list<std::shared_ptr<Client>> clients;
  std::unordered_map<std::string, Client*> logins;
  void accept();
public:
  Server(IOEnv &io, uint16_t port);
  void updateLogin(Client &c);
  void removeClient(Client &c);
  void enqueueActionGroup(const std::string &client, std::vector<SharedAction> actions);
  void enqueueAction(const std::string &client, SharedAction action);
  size_t countPending(const std::string &client) const;

  template<typename T>
  const T &getBestAccess(const std::vector<T> &accesses) {
    const T *bestAccess(&accesses.front());
    size_t bestCount(std::numeric_limits<size_t>::max());
    for (auto &access : accesses) {
      size_t count(countPending(access.client));
      if (count < bestCount) {
        bestCount = count;
        bestAccess = &access;
      }
    }
    return *bestAccess;
  }
};

#endif
