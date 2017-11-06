#pragma once
//-----------------------------------------------------------------------------------------------
#include <string>
#include <vector>

//-----------------------------------------------------------------------------------------------
const std::string Stringf( const char* format, ... );
const std::string Stringf( const int maxLength, const char* format, ... );
std::vector<std::string> split(const char* data, const char* tokens);

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
inline int parse(const char* str) {
  return atoi(str);
}

template<>
inline unsigned char parse(const char* str) {
  int val = atoi(str);

  return unsigned char(val);
}

template<>
inline float parse(const char* str) {
  return float(atof(str));
}

template<>
inline bool parse(const char* str) {
  return strcmp(str, "true")==0 ? true : false;
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