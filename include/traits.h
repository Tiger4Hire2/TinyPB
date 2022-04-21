#pragma once
#include <vector>
template <class v> struct is_vector{ static constexpr bool value{false}; };
template <class T> struct is_vector<std::vector<T>>{ static constexpr bool value{true}; };
template <class T> static constexpr  bool is_vector_v() { return is_vector<T>::value; };

template <class T> concept Numerical = std::is_arithmetic_v<T>;
