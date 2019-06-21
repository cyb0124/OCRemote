#pragma once
#include <memory>

template<typename T>
class ValuePtr {
  std::unique_ptr<T> data;
public:
  template<typename ...Ts> ValuePtr(Ts &&...xs) :data(std::make_unique<T>(std::forward<Ts>(xs)...)) {}
  ValuePtr(const ValuePtr<T> &o) :ValuePtr(*o) {}
  ValuePtr(ValuePtr<T> &&o) :data(std::move(o.data)) {}
  ValuePtr<T> &operator=(const ValuePtr<T> &o) { auto copy(o); swap(*this, o); }
  ValuePtr<T> &operator=(ValuePtr<T> &&o) { data = std::move(o.data); }
  friend void swap(ValuePtr<T> &x, ValuePtr<T> &y) noexcept { std::swap(x.data, y.data); }
  T &operator*() { return *data; }
  const T &operator*() const { return *data; }
  T *operator->() { return data.get(); }
  const T *operator->() const { return data.get(); }
};
template<typename T> inline bool operator==(const ValuePtr<T> &x, const ValuePtr<T> &y) { return *x == *y; }
template<typename T> inline bool operator!=(const ValuePtr<T> &x, const ValuePtr<T> &y) { return *x != *y; }
template<typename T> inline bool operator>=(const ValuePtr<T> &x, const ValuePtr<T> &y) { return *x >= *y; }
template<typename T> inline bool operator<=(const ValuePtr<T> &x, const ValuePtr<T> &y) { return *x <= *y; }
template<typename T> inline bool operator>(const ValuePtr<T> &x, const ValuePtr<T> &y) { return *x > *y; }
template<typename T> inline bool operator<(const ValuePtr<T> &x, const ValuePtr<T> &y) { return *x < *y; }
namespace std { template<typename T> struct hash<ValuePtr<T>> { size_t operator()(const ValuePtr<T> &x) const { return hash<T>{}(*x); } }; }

template<template<typename> typename F>
struct InitialAlgebra : ValuePtr<F<InitialAlgebra<F>>> {
  using ValuePtr<F<InitialAlgebra<F>>::ValuePtr;
};
