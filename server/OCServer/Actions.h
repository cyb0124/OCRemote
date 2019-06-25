#pragma once
#include <vector>
#include "Overload.h"
#include "Item.h"

template<typename T>
struct Listener {
  using Result = T;
  virtual ~Listener() = default;
  virtual void onFail(std::string cause) = 0;
  virtual void onResult(T result) = 0;
};

template<typename T>
struct DummyListener : Listener<T> {
  void onFail(std::string cause) override {}
  void onResult(T result) override {}
};

template<typename T>
using SharedListener = std::shared_ptr<Listener<T>>;

template<typename T>
class Promise : public Listener<T> {
  SharedListener<T> next;
public:
  void onFail(std::string cause) override {
    if (!next) throw std::logic_error("broken chain");
    next->onFail(std::move(cause));
  }

  void onResult(T result) override {
    if (!next) throw std::logic_error("broken chain");
    next->onResult(std::move(result));
  }

  void listen(SharedListener<T> x) {
    if (next) throw std::logic_error("already listened");
    next = std::move(x);
  }

  template<typename Fn>
  auto map(std::weak_ptr<std::monostate> alive, Fn fn) {
    using To = std::invoke_result_t<Fn, T>;
    auto result(std::make_shared<Promise<To>>());

    struct Impl : Listener<T> {
      SharedListener<To> to;
      std::weak_ptr<std::monostate> alive;
      Fn fn;

      Impl(SharedListener<To> to, std::weak_ptr<std::monostate> alive, Fn fn)
        :to(std::move(to)), alive(std::move(alive)), fn(std::move(fn)) {}

      void onFail(std::string cause) override {
        to->onFail(std::move(cause));
      }

      void onResult(T result) override {
        if (alive.expired())
          to->onFail("node died");
        else
          to->onResult(fn(std::move(result)));
      }
    };

    listen(std::make_shared<Impl>(result, std::move(alive), std::move(fn)));
    return result;
  }

  template<typename To>
  auto mapTo(To value) {
    auto result(std::make_shared<Promise<To>>());

    struct Impl : Listener<T> {
      SharedListener<To> to;
      To value;

      Impl(SharedListener<To> to, To value)
        :to(std::move(to)), value(std::move(value)) {}

      void onFail(std::string cause) override {
        to->onFail(std::move(cause));
      }

      void onResult(T) override {
        to->onResult(std::move(value));
      }
    };

    listen(std::make_shared<Impl>(result, std::move(value)));
    return result;
  }

  template<typename Fn>
  auto then(std::weak_ptr<std::monostate> alive, Fn fn) {
    using To = typename std::invoke_result_t<Fn, T>::element_type::Result;
    auto result(std::make_shared<Promise<To>>());

    struct Impl : Listener<T> {
      SharedListener<To> to;
      std::weak_ptr<std::monostate> alive;
      Fn fn;

      Impl(SharedListener<To> to, std::weak_ptr<std::monostate> alive, Fn fn)
        :to(std::move(to)), alive(std::move(alive)), fn(std::move(fn)) {}

      void onFail(std::string cause) override {
        to->onFail(std::move(cause));
      }

      void onResult(T result) override {
        if (alive.expired())
          to->onFail("node died");
        else
          fn(std::move(result))->listen(std::move(to));
      }
    };

    listen(std::make_shared<Impl>(result, std::move(alive), std::move(fn)));
    return result;
  }

  template<typename Fn>
  std::shared_ptr<Promise<T>> finally(std::weak_ptr<std::monostate> alive, Fn fn) {
    auto result(std::make_shared<Promise<T>>());

    struct Impl : Listener<T> {
      SharedListener<T> to;
      std::weak_ptr<std::monostate> alive;
      Fn fn;

      Impl(SharedListener<T> to, std::weak_ptr<std::monostate> alive, Fn fn)
        :to(std::move(to)), alive(std::move(alive)), fn(std::move(fn)) {}

      void onFail(std::string cause) override {
        if (!alive.expired())
          fn();
        to->onFail(std::move(cause));
      }

      void onResult(T result) override {
        if (!alive.expired())
          fn();
        to->onResult(std::move(result));
      }
    };

    listen(std::make_shared<Impl>(result, std::move(alive), std::move(fn)));
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

template<typename F>
inline SharedPromise<std::monostate> scheduleTrivialPromise(F &dispatcher) {
  auto result(std::make_shared<Promise<std::monostate>>());
  dispatcher([result]() { result->onResult({}); });
  return result;
}

template<typename T, typename F>
inline SharedPromise<T> scheduleFailingPromise(F &dispatcher, std::string reason) {
  auto result(std::make_shared<Promise<T>>());
  dispatcher([result, reason(std::move(reason))]() { result->onFail(std::move(reason)); });
  return result;
}

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

  struct Base : Listener<SValue> {
    virtual void dump(STable&) = 0;
  };

  template<typename T>
  struct Impl : Base, Promise<T> {
    void onFail(std::string cause) override {
      Promise<T>::onFail(std::move(cause));
    }
  };

  struct ImplUnit : Impl<std::monostate> {
    void onResult(SValue) override {
      Promise<std::monostate>::onResult(std::monostate());
    }
  };

  struct Print : ImplUnit {
    std::string text;
    uint32_t color{0xffffffu};
    double beep{-1.};
    void dump(STable&) override;
  };

  struct List : Impl<std::vector<SharedItemStack>> {
    std::string inv;
    int side;
    void dump(STable&) override;
    void onResult(SValue) override;
  };

  struct ListXN : List {
    XNetCoord pos;
    void dump(STable&) override;
  };

  struct ListME : Impl<std::vector<SharedItemStack>> {
    std::string inv;
    void dump(STable&) override;
    void onResult(SValue) override;
  };

  struct XferME : ImplUnit {
    std::string me, inv;
    SValue filter;
    std::vector<SValue> args;
    int size;
    void dump(STable&) override;
  };

  struct Call : Impl<SValue> {
    std::string inv, fn;
    std::vector<SValue> args;
    void dump(STable&) override;
    void onResult(SValue) override;
  };
}

using SharedAction = std::shared_ptr<Actions::Base>;
