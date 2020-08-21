#ifndef _OVERLOAD_H_
#define _OVERLOAD_H_

template<typename ...T> struct Overload : T... { using T::operator()...; };
template<typename ...T> Overload(T...) -> Overload<T...>;

#endif
