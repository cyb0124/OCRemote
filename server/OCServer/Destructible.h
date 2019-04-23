#pragma once
#include <memory>
#include <boost/noncopyable.hpp>

struct Destructible : private boost::noncopyable {
  virtual ~Destructible() = default;
};

using SharedDestructible = std::shared_ptr<Destructible>;

template<typename T>
class LambdaDestructible : Destructible {
  T fn;
public:
  explicit LambdaDestructible(T fn) :fn(std::move(fn)) {}
  ~LambdaDestructible() override { fn(); }
};

template<typename T>
inline SharedDestructible makeDestructible(T fn) {
  return std::make_shared<LambdaDestructible<T>>(std::move(fn));
}
