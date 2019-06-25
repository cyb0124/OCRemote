#pragma once
#include <functional>
#include <stdexcept>
#include <optional>
#include <variant>
#include <memory>
#include <string>
#include <map>

using SKeyBase = std::variant<double, std::string, bool>;
struct SKey : SKeyBase {
  using SKeyBase::variant;
  SKey(const char *x) :SKeyBase(std::in_place_type<std::string>, x) {}
};
using STable = std::map<SKey, struct SValue>;
using SValueBase = std::variant<std::monostate, double, std::string, bool, STable>;
struct SValue : SValueBase {
  using SValueBase::variant;
  SValue(const char *x) :SValueBase(std::in_place_type<std::string>, x) {}
};
STable arrayToSTable(std::vector<SValue>&&);
std::vector<SValue> sTableToArray(STable&&);
std::string serialize(const SValue&);
std::string serialize(const STable&);

class Deserializer {
  struct State {
    Deserializer &env;
    std::unique_ptr<State> p;
    State(Deserializer &env) :env(env), p(std::move(env.s)) {}
    virtual ~State() = default;
    virtual void reduce(SValue x, const char *data, size_t size) { throw std::logic_error("invalid reduce"); }
    virtual void shift(const char *data, size_t size) { throw std::logic_error("invalid shift"); }
    std::unique_ptr<State> yield();
    void init() {}
  };

  class Number : public State {
    std::string buffer;
  public:
    using State::State;
    void shift(const char *data, size_t size) override;
  };

  class String : public State {
    std::string buffer;
    bool escape{};
  public:
    using State::State;
    void shift(const char *data, size_t size) override;
  };

  struct Root : State {
    using State::State;
    void shift(const char *data, size_t size) override;
  };

  class Table : public State {
    STable result;
    std::optional<SKey> key;
  public:
    using State::State;
    void reduce(SValue x, const char *data, size_t size) override;
    void init();
  };

  struct Start : State {
    using State::State;
    void reduce(SValue x, const char *data, size_t size) override;
    void init();
  };

  std::function<void(SValue)> cb;
  std::unique_ptr<State> s;
  template<typename T> void enter() {
    s = std::make_unique<T>(*this);
    static_cast<T&>(*s).init();
  }
public:
  Deserializer(decltype(cb) cb) :cb(std::move(cb)) { enter<Start>(); }
  void operator()(const char *data, size_t size) { s->shift(data, size); }
};
