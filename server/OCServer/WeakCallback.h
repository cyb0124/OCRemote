#pragma once
#include <memory>

template<typename ThisT, typename CallbackFunctionT>
class WeakCallback {
  static_assert(sizeof(ThisT) < 0, "Deduction Failed");
};

template<typename ThisT, typename CallbackObjectT, typename ...Ts>
class WeakCallback<ThisT, void(CallbackObjectT::*)(Ts...) const> {
  std::weak_ptr<ThisT> wkThis;
  CallbackObjectT callback;
public:
  WeakCallback(std::weak_ptr<ThisT> wkThis, CallbackObjectT callback)
    :wkThis(std::move(wkThis)), callback(std::move(callback)) {}

  void operator()(Ts ...xs) const {
    auto pThis(wkThis.lock());
    if (!pThis) return;
    callback(std::forward<Ts>(xs)...);
  }
};

template<typename ThisT, typename CallbackObjectT>
inline WeakCallback<ThisT, decltype(&CallbackObjectT::operator())>
makeWeakCallback(std::shared_ptr<ThisT> pThis, CallbackObjectT callback) {
  return {std::move(pThis), std::move(callback)};
}

template<typename ThisT, typename CallbackObjectT>
inline WeakCallback<ThisT, decltype(&CallbackObjectT::operator())>
makeWeakCallback(std::weak_ptr<ThisT> pThis, CallbackObjectT callback) {
  return {std::move(pThis), std::move(callback)};
}

template<typename ThisT, typename QueueT, typename CallbackFunctionT>
class WeakDeferredCallback {
  static_assert(sizeof(ThisT) < 0, "Deduction Failed");
};

template<typename ThisT, typename CallbackObjectT, typename ...Ts>
class WeakDeferredCallbackTask {
  std::weak_ptr<ThisT> wkThis;
  std::shared_ptr<CallbackObjectT> callback;
  mutable std::tuple<Ts...> arguments;

  template<size_t ...indices>
  void helper(std::integer_sequence<size_t, indices...>) const {
    callback->operator()(std::forward<Ts>(std::get<indices>(arguments))...);
  }
public:
  WeakDeferredCallbackTask(
    std::weak_ptr<ThisT> wkThis,
    std::shared_ptr<CallbackObjectT> callback,
    std::tuple<Ts...> arguments)
    :wkThis(std::move(wkThis)),
    callback(std::move(callback)),
    arguments(std::move(arguments)) {}

  void operator()() const {
    auto pThis(wkThis.lock());
    if (!pThis) return;
    helper(std::index_sequence_for<Ts...>());
  }
};

template<typename ThisT, typename QueueT, typename CallbackObjectT, typename ...Ts>
class WeakDeferredCallback<ThisT, QueueT, void(CallbackObjectT::*)(Ts...) const> {
  std::weak_ptr<ThisT> wkThis;
  QueueT queue;
  std::shared_ptr<CallbackObjectT> callback;
public:
  WeakDeferredCallback(std::weak_ptr<ThisT> wkThis, QueueT queue, CallbackObjectT callback)
    :wkThis(std::move(wkThis)), queue(std::move(queue)),
    callback(std::make_shared<CallbackObjectT>(std::move(callback))) {}

  void operator()(Ts ...xs) const {
    queue(WeakDeferredCallbackTask<ThisT, CallbackObjectT, Ts...>{
      wkThis,
      callback,
      std::tuple<Ts...>(std::forward<Ts>(xs)...)
    });
  }
};

template<typename ThisT, typename QueueT, typename CallbackObjectT>
inline WeakDeferredCallback<ThisT, QueueT, decltype(&CallbackObjectT::operator())>
makeWeakDeferredCallback(std::shared_ptr<ThisT> pThis, QueueT queue, CallbackObjectT callback) {
  return {std::move(pThis), std::move(queue), std::move(callback)};
}

template<typename ThisT, typename QueueT, typename CallbackObjectT>
inline WeakDeferredCallback<ThisT, QueueT, decltype(&CallbackObjectT::operator())>
makeWeakDeferredCallback(std::weak_ptr<ThisT> pThis, QueueT queue, CallbackObjectT callback) {
  return {std::move(pThis), std::move(queue), std::move(callback)};
}

template<typename CallbackObjectT, typename ...Ts>
class DeferredCallbackTask {
  std::shared_ptr<CallbackObjectT> callback;
  mutable std::tuple<Ts...> arguments;

  template<size_t ...indices>
  void helper(std::integer_sequence<size_t, indices...>) const {
    callback->operator()(std::forward<Ts>(std::get<indices>(arguments))...);
  }
public:
  DeferredCallbackTask(
    std::shared_ptr<CallbackObjectT> callback,
    std::tuple<Ts...> arguments)
    :callback(std::move(callback)),
    arguments(std::move(arguments)) {}

  void operator()() const {
    helper(std::index_sequence_for<Ts...>());
  }
};

template<typename QueueT, typename CallbackFunctionT>
class DeferredCallback {
  static_assert(sizeof(QueueT) < 0, "Deduction Failed");
};

template<typename QueueT, typename CallbackObjectT, typename ...Ts>
class DeferredCallback<QueueT, void(CallbackObjectT::*)(Ts...) const> {
  QueueT queue;
  std::shared_ptr<CallbackObjectT> callback;
public:
  DeferredCallback(QueueT queue, CallbackObjectT callback)
    :queue(std::move(queue)),
    callback(std::make_shared<CallbackObjectT>(std::move(callback))) {}

  void operator()(Ts ...xs) const {
    queue(DeferredCallbackTask<CallbackObjectT, Ts...>{
      callback,
      std::tuple<Ts...>(std::forward<Ts>(xs)...)
    });
  }
};

template<typename QueueT, typename CallbackObjectT>
inline DeferredCallback<QueueT, decltype(&CallbackObjectT::operator())>
makeDeferredCallback(QueueT queue, CallbackObjectT callback) {
  return {std::move(queue), std::move(callback)};
}
