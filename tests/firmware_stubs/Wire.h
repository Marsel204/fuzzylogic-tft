#pragma once
#include <Arduino.h>
#include <array>
struct WireStub {
 bool connected = false;
 std::array<uint8_t, 5> data = {};
 int bytesToReturn = 5;
 int readIndex = 0;
 template<class...T> void begin(T...){}
 template<class...T> void setTimeOut(T...){}
 template<class...T> void beginTransmission(T...){}
 template<class...T> void write(T...){}
 template<class...T> int endTransmission(T...){return connected ? 0 : 1;}
 template<class...T> int requestFrom(T...){readIndex=0; return connected ? bytesToReturn : 0;}
 int read(){return readIndex < bytesToReturn ? data[readIndex++] : -1;}
};
inline WireStub Wire;
