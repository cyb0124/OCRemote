#pragma once
#include <functional>
#include <stdexcept>
#include <optional>
#include <variant>
#include <string>
#include <map>
#include "ValuePtr.h"

using SKey = std::variant<double, std::string, bool>;
using STable = std::map<SKey, struct SValue>;
using SValueBase = std::variant<std::monostate, double, std::string, bool, ValuePtr<STable>>;
struct SValue : SValueBase {
  using SValueBase::variant;
  bool isNull() const { return std::holds_alternative<std::monostate>(*this); }
  STable &getTable() { return *std::get<ValuePtr<STable>>(*this); }
  std::string dump() const;
};
STable arrayToSTable(std::vector<SValue>&&);
std::vector<SValue> sTableToArray(STable&&);

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

  class Number : State {
    std::string buffer;
  public:
    using State::State;
    void shift(const char *data, size_t size) override;
  };

  class String : State {
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

  class Table : State {
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
  template<typename T>
  void enter() {
    s = std::make_unique<T>(*this);
    static_cast<T&>(*s).init();
  }
public:
  Deserializer(decltype(cb) cb) :cb(std::move(cb)) { enter<Start>(); }
  void operator()(const char *data, size_t size) { s->shift(data, size); }
};
