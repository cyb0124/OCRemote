#pragma once
#include <vector>
#include <variant>
#include <optional>
#include <functional>
#include "Overload.h"
#include "json.hpp"
#include "Item.h"

template<typename T>
struct Listener {
  using Result = T;
  virtual ~Listener() = default;
  virtual void onFail(std::string cause) = 0;
  virtual void onResult(T result) = 0;
};

template<typename T>
using SharedListener = std::shared_ptr<Listener<T>>;

template<typename T>
class Promise : public Listener<T> {
  SharedListener<T> listener;
public:
  void onFail(std::string cause) override {
    if (!listener) throw std::logic_error("broken chain");
    listener->onFail(std::move(cause));
  }

  void onResult(T result) override {
    if (!listener) throw std::logic_error("broken chain");
    listener->onResult(std::move(result));
  }

  void listen(SharedListener<T> x) {
    if (listener) throw std::logic_error("already listened");
    listener = std::move(x);
  }

  template<typename Fn>
  auto map(Fn fn) {
    using To = std::invoke_result_t<Fn, T>;
    auto result(std::make_shared<Promise<To>>());

    struct Impl : Listener<T> {
      SharedListener<To> to;
      Fn fn;
      Impl(SharedListener<To> to, Fn fn) :to(std::move(to)), fn(std::move(fn)) {}

      void onFail(std::string cause) override {
        to->onFail(std::move(cause));
      }

      void onResult(T result) override {
        to->onResult(fn(std::move(result)));
      }
    };

    listen(std::make_shared<Impl>(result, std::move(fn)));
    return result;
  }

  template<typename Fn>
  auto then(Fn fn) {
    using To = typename std::invoke_result_t<Fn, T>::element_type::Result;
    auto result(std::make_shared<Promise<To>>());

    struct Impl : Listener<T> {
      SharedListener<To> to;
      Fn fn;
      Impl(SharedListener<To> to, Fn fn) :to(std::move(to)), fn(std::move(fn)) {}

      void onFail(std::string cause) override {
        to->onFail(std::move(cause));
      }

      void onResult(T result) override {
        fn(std::move(result))->listen(std::move(to));
      }
    };

    listen(std::make_shared<Impl>(result, std::move(fn)));
    return result;
  }

  static std::shared_ptr<Promise<std::vector<T>>> all(const std::vector<std::shared_ptr<Promise<T>>> &xs) {
    if (xs.empty()) throw std::runtime_error("waiting for nothing");

    struct Context {
      std::shared_ptr<Promise<std::vector<T>>> to{std::make_shared<Promise<std::vector<T>>>()};
      std::variant<std::vector<T>, std::string> result;
      size_t nRemaining;

      explicit Context(size_t nTotal)
        :result(std::in_place_type<std::vector<T>>, nTotal), nRemaining(nTotal) {}

      void decreaseRemaining() {
        if (!--nRemaining) {
          std::visit(Overload{
            [this](std::vector<T> &xs) {
              to->onResult(std::move(xs));
            },
            [this](std::string &x) {
              to->onFail(std::move(x));
            }
          }, result);
        }
      }

      void onResult(size_t which, T &x) {
        std::visit(Overload{
          [&](std::vector<T> &xs) {
            xs[which] = std::move(x);
          },
          [](std::string &) {}
        }, result);
        decreaseRemaining();
      }

      void onFail(std::string &reason) {
        std::visit(Overload{
            [&](std::vector<T> &xs) {
              result = std::move(reason);
            },
            [&](std::string &x) {
              x += "; " + reason;
            }
        }, result);
        decreaseRemaining();
      }
    };

    struct Impl : Listener<T> {
      std::shared_ptr<Context> context;
      size_t index;
      Impl(const std::shared_ptr<Context> &context, size_t index)
        :context(context), index(index) {}

      void onFail(std::string cause) override {
        context->onFail(cause);
      }

      void onResult(T result) override {
        context->onResult(index, result);
      }
    };

    auto context(std::make_shared<Context>(xs.size()));
    for (size_t i = 0; i < xs.size(); ++i) {
      xs[i]->listen(std::make_shared<Impl>(context, i));
    }
    return context->to;
  }
};

template<typename T>
using SharedPromise = std::shared_ptr<Promise<T>>;

struct XNetCoord {
  int x, y, z;
  XNetCoord operator-(const XNetCoord &other) const {
    return {x - other.x, y - other.y, z - other.z};
  }
};

namespace Actions {
  enum {
    bottom = 0, down  = 0, yn = 0,
    top    = 1, up    = 1, yp = 1,
    back   = 2, north = 2, zn = 2,
    front  = 3, south = 3, zp = 3,
    right  = 4, west  = 4, xn = 4,
    left   = 5, east  = 5, xp = 5
  };

  struct Base : Listener<nlohmann::json> {
    virtual void dump(nlohmann::json&) = 0;
  };

  template<typename T>
  struct Impl : Base, Promise<T> {
    void onFail(std::string cause) override {
      Promise<T>::onFail(std::move(cause));
    }
  };

  struct ImplUnit : Impl<std::monostate> {
    void onResult(nlohmann::json) override {
      Promise<std::monostate>::onResult(std::monostate());
    }
  };

  struct Print : ImplUnit {
    std::string text;
    uint32_t color = 0xffffff;
    std::optional<float> beep;
    void dump(nlohmann::json&) override;
  };

  struct List : Impl<std::vector<SharedItemStack>> {
    std::string inv;
    int side;
    void dump(nlohmann::json&) override;
    void onResult(nlohmann::json) override;
  };

  struct ListXN : List {
    XNetCoord pos;
    void dump(nlohmann::json&) override;
  };

  struct ListME : Impl<std::vector<SharedItemStack>> {
    std::string inv;
    void dump(nlohmann::json&) override;
    void onResult(nlohmann::json) override;
  };

  struct XferME : ImplUnit {
    std::string me, inv;
    nlohmann::json filter;
    int fromSide, toSide, slot;
    void dump(nlohmann::json&) override;
  };

  struct Call : Impl<nlohmann::json> {
    std::string inv, fn;
    nlohmann::json args = nlohmann::json::array();
    void dump(nlohmann::json&) override;
    void onResult(nlohmann::json) override;
  };
}

using SharedAction = std::shared_ptr<Actions::Base>;
