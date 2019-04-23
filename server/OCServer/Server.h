#pragma once
#include <map>
#include <list>
#include <optional>
#include <boost/asio.hpp>
#include <boost/noncopyable.hpp>
#include "json.hpp"
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
private:
  Server &s;
  Itr itr;
  boost::asio::ip::tcp::socket socket;
  std::optional<std::string> login;
  std::string logHeader;
  std::list<std::vector<SharedAction>> sendQueue;
  size_t sendQueueTotal = 0;
  bool isSending = false;
  std::list<SharedAction> responseQueue;
  void readLength();
  void readContent(size_t size);
  void onPacket(const char *data, size_t size);
  void onPacket(nlohmann::json);
  void send();
public:
  ~Client();
  Client(Server &s, boost::asio::ip::tcp::socket socket);
  const std::optional<std::string> &getLogin() const { return login; }
  const Itr &getItr() const { return itr; }
  void init(const Itr&);
  void log(const std::string &message);
  void enqueueActionGroup(std::vector<SharedAction> group);
  size_t countPending() const;
};

struct Server : std::enable_shared_from_this<Server> {
  IOEnv &io;
private:
  boost::asio::ip::tcp::acceptor acceptor;
  std::list<std::shared_ptr<Client>> clients;
  std::unordered_map<std::string, Client*> logins;
  void accept();
public:
  explicit Server(IOEnv &io);
  void init(uint16_t port);
  void updateLogin(Client &c);
  void removeClient(Client &c);
  void enqueueActionGroup(const std::string &client, std::vector<SharedAction> group);
  void enqueueAction(const std::string &client, const SharedAction &action);
  size_t countPending(const std::string &client) const;
};
