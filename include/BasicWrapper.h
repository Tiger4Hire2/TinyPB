#pragma once
#include <type_traits>

template<class T>
class BasicWrapper {
    static_assert(std::is_integral_v<T>);
    T value;
public:
    using value_type = T;
    BasicWrapper() :value() {}
    BasicWrapper(T v) :value(v) {}
    operator T() const {return value;}
    //modifiers
    BasicWrapper& operator=(T v) {value=v; return *this;}
    BasicWrapper& operator+=(T v) {value+=v; return *this;}
    BasicWrapper& operator-=(T v) {value-=v; return *this;}
    BasicWrapper& operator*=(T v) {value*=v; return *this;}
    BasicWrapper& operator/=(T v) {value/=v; return *this;}
    BasicWrapper& operator%=(T v) {value%=v; return *this;}
    BasicWrapper& operator++() {++value; return *this;}
    BasicWrapper& operator--() {--value; return *this;}
    BasicWrapper operator++(int) {return BasicWrapper(value++);}
    BasicWrapper operator--(int) {return BasicWrapper(value--);}
    BasicWrapper& operator&=(T v) {value&=v; return *this;}
    BasicWrapper& operator|=(T v) {value|=v; return *this;}
    BasicWrapper& operator^=(T v) {value^=v; return *this;}
    BasicWrapper& operator<<=(T v) {value<<=v; return *this;}
    BasicWrapper& operator>>=(T v) {value>>=v; return *this;}
    // cast
    operator T() { return value; } 
    //accessors
    BasicWrapper operator+() const {return BasicWrapper(+value);}
    BasicWrapper operator-() const {return BasicWrapper(-value);}
    BasicWrapper operator!() const {return BasicWrapper(!value);}
    BasicWrapper operator~() const {return BasicWrapper(~value);}
    //friends
    friend BasicWrapper operator+(BasicWrapper iw, BasicWrapper v) {return iw+=v;}
    friend BasicWrapper operator+(BasicWrapper iw, T v) {return iw+=v;}
    friend BasicWrapper operator+(T v, BasicWrapper iw) {return BasicWrapper(v)+=iw;}
    friend BasicWrapper operator-(BasicWrapper iw, BasicWrapper v) {return iw-=v;}
    friend BasicWrapper operator-(BasicWrapper iw, T v) {return iw-=v;}
    friend BasicWrapper operator-(T v, BasicWrapper iw) {return BasicWrapper(v)-=iw;}
    friend BasicWrapper operator*(BasicWrapper iw, BasicWrapper v) {return iw*=v;}
    friend BasicWrapper operator*(BasicWrapper iw, T v) {return iw*=v;}
    friend BasicWrapper operator*(T v, BasicWrapper iw) {return BasicWrapper(v)*=iw;}
    friend BasicWrapper operator/(BasicWrapper iw, BasicWrapper v) {return iw/=v;}
    friend BasicWrapper operator/(BasicWrapper iw, T v) {return iw/=v;}
    friend BasicWrapper operator/(T v, BasicWrapper iw) {return BasicWrapper(v)/=iw;}
    friend BasicWrapper operator%(BasicWrapper iw, BasicWrapper v) {return iw%=v;}
    friend BasicWrapper operator%(BasicWrapper iw, T v) {return iw%=v;}
    friend BasicWrapper operator%(T v, BasicWrapper iw) {return BasicWrapper(v)%=iw;}
    friend BasicWrapper operator&(BasicWrapper iw, BasicWrapper v) {return iw&=v;}
    friend BasicWrapper operator&(BasicWrapper iw, T v) {return iw&=v;}
    friend BasicWrapper operator&(T v, BasicWrapper iw) {return BasicWrapper(v)&=iw;}
    friend BasicWrapper operator|(BasicWrapper iw, BasicWrapper v) {return iw|=v;}
    friend BasicWrapper operator|(BasicWrapper iw, T v) {return iw|=v;}
    friend BasicWrapper operator|(T v, BasicWrapper iw) {return BasicWrapper(v)|=iw;}
    friend BasicWrapper operator^(BasicWrapper iw, BasicWrapper v) {return iw^=v;}
    friend BasicWrapper operator^(BasicWrapper iw, T v) {return iw^=v;}
    friend BasicWrapper operator^(T v, BasicWrapper iw) {return BasicWrapper(v)^=iw;}
    friend BasicWrapper operator<<(BasicWrapper iw, BasicWrapper v) {return iw<<=v;}
    friend BasicWrapper operator<<(BasicWrapper iw, T v) {return iw<<=v;}
    friend BasicWrapper operator<<(T v, BasicWrapper iw) {return BasicWrapper(v)<<=iw;}
    friend BasicWrapper operator>>(BasicWrapper iw, BasicWrapper v) {return iw>>=v;}
    friend BasicWrapper operator>>(BasicWrapper iw, T v) {return iw>>=v;}
    friend BasicWrapper operator>>(T v, BasicWrapper iw) {return BasicWrapper(v)>>=iw;}
};
