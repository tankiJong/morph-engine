#pragma once
//-----------------------------------------------------------------------------------------------
#include <string>
#include <string_view>
#include <vector>
#include <algorithm>
#include "Engine/Core/type.h"
#include "Engine/Debug/ErrorWarningAssert.hpp"
//-----------------------------------------------------------------------------------------------
const std::string Stringf( const char* format, ... );
const std::string Stringf( const int maxLength, const char* format, ... );
std::vector<std::string> split(const char* data, const char* tokens);
unsigned char castHex(char hex);

//---------------------Parse---------------------------------------------------------------------
template<typename T>
inline T parse(const std::string& str) { return parse<T>(str.c_str()); };

template<typename T>
inline T parse(const char* str) {
  T result{};
  result.fromString(str);
  return result;
};

template<>
inline std::string parse(const char* str) {
  return std::string(str);
}

template<>
inline int parse(const char* str) {
  return atoi(str);
}

template<>
inline unsigned char parse(const char* str) {
  int val = atoi(str);

  return unsigned char(val);
}

template<>
inline uint parse(const char* str) {
  char* end;
  uint val = strtol(str, &end, 10);
  ENSURES(*end == '\0');

  return val;
}

template<>
inline float parse(const char* str) {
  return float(atof(str));
}

template<>
inline bool parse(const char* str) {
  return strcmp(str, "true")==0 ? true : false;
}

template<typename T, typename A = std::allocator<T>>
inline std::vector<T, A> parse(const char* str, const char* tokens) {
  auto datas = split(str, tokens);
  std::vector<T, A> results(datas.size());
  std::transform(datas.begin(), datas.end(), results.begin(), [](auto& str) { return parse<T>(str); });

  return results;
}

template<typename T>
inline void operator >>(const char* lhs, T& rhs) {
  rhs = std::move<T>(parse<T>(lhs));
}

//--------------toString-------------------------------------------------------------------------
template<typename T>
inline std::string toString(T val) {
  return val.toString();
}

template<>
inline std::string toString(int val) {
  return Stringf("%d", val);
}

template<>
inline std::string toString(float val) {
  return Stringf("%f", val);
}

template<>
inline std::string toString(char val) {
  return Stringf("%c", val);
}

template<>
inline std::string toString(bool val) {
  return val ? "true" : "false";
}

template<typename T>
inline void operator >>(const T& lhs, std::string& rhs) {
  rhs = std::move(toString(lhs));
}

using namespace std::literals::string_literals;
using namespace std::literals::string_view_literals;
