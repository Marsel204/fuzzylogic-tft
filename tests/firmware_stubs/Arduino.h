#pragma once
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <cmath>
#include <cstdarg>
#include <string>
#include <algorithm>
#include <type_traits>
#define HIGH 1
#define LOW 0
#define OUTPUT 1
#define constrain(v,lo,hi) ((v)<(lo)?(lo):((v)>(hi)?(hi):(v)))
inline uint32_t stubMillis = 0;
inline uint32_t stubMicros = 0;
inline void delay(int milliseconds) { stubMillis += milliseconds; }
inline void pinMode(int,int) {}
inline void digitalWrite(int,int) {}
inline uint32_t millis(){return stubMillis;}
inline uint32_t micros(){return ++stubMicros;}
struct SerialStub {
 std::string input;
 void begin(int){}
 bool available(){return !input.empty();}
 char read(){char c=input.front();input.erase(0,1);stubMillis += 100;return c;}
 void println(){std::puts("");}
 template<class T> void println(T v){if constexpr(std::is_convertible_v<T,const char*>) std::puts(v); else std::printf("%d\n",int(v));}
 void printf(const char*fmt,...){va_list a;va_start(a,fmt);std::vprintf(fmt,a);va_end(a);}
};
extern SerialStub Serial;
