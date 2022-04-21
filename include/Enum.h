#pragma once

template<class T>
class EnumIteratorImp
{
    int i;
public:
    EnumIteratorImp(int start): i(start) {}
    EnumIteratorImp operator++() { const auto tmp = *this; i++; return tmp;}
    EnumIteratorImp operator++(int) {return EnumIteratorImp{i+1};}
    T operator*() { return static_cast<T>(i); }
    bool operator==(const EnumIteratorImp other) const { return other.i == i;}
    bool operator!=(const EnumIteratorImp other) const { return other.i != i;}
};

template<class T> concept EnumeratedType = std::is_enum_v<T> && 
                        requires(T t){T::NUM_ENUMS;} &&
                        requires(T t){to_string(t);};
template<EnumeratedType T>
class EnumIterator
{
public:
    EnumIteratorImp<T> begin() const { return EnumIteratorImp<T>{0}; }
    EnumIteratorImp<T> end() const { return EnumIteratorImp<T>{static_cast<int>(T::NUM_ENUMS)}; }
};