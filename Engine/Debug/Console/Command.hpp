#pragma once
#include <vector>
#include "Engine/Core/common.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Debug/ErrorWarningAssert.hpp"

class ArgumentNotFoundException : public std::exception {

public:
  explicit ArgumentNotFoundException(uint n) :msg(Stringf("Cannot find argument asked at position %u.", n)) {}
  explicit ArgumentNotFoundException(const std::string& name) :msg(Stringf("Cannot find argument \"%s\".", name.c_str())) {}
  inline char const* what() const override { return msg.c_str(); };
  std::string msg;
};

class InvalidArgumentException : public std::exception {

public:
  explicit InvalidArgumentException(uint n) :msg(Stringf("argument at position %u is invalid", n)) {}
  inline char const* what() const override { return msg.c_str(); };
  std::string msg;
};
class Command {
  struct Arg {
    char name[64]{'\0'};
    char value[64]{'\0'};

    Arg(std::string kv);
    Arg(std::string v, std::string k = "");
  };
public:
  Command(const std::string& cmd);
  ~Command() =default;

  inline const std::string& name() const { return mName; };

  template<typename T>
  T arg(const std::string& name) {
    static_assert(!std::is_pointer_v<T>, "T cannot be pointer type");

    const std::string& val = arg<std::string>(name);
    return ::parse<T>(val);
  }

  template<uint N, typename T>
  T arg() {
    static_assert(!std::is_pointer_v<T>, "T cannot be pointer type");
//    EXPECTS(N < mArgs.size());
    if(N >= mArgs.size()) {
      throw ArgumentNotFoundException(N);
    }
    return ::parse<T>(mArgs[N].value);
  }

  template<uint N>
  std::string arg() {
    //    EXPECTS(N < mArgs.size());
    if (N >= mArgs.size()) {
      throw ArgumentNotFoundException(N);
    }
    return mArgs[N].value;
  }

  template<>
  std::string arg(const std::string& name) {
    for(const Arg& a: mArgs) {
      if(strcmp(a.name, name.c_str()) == 0) {
        return a.value;
      }
    }

    throw ArgumentNotFoundException(name);
  }

  bool parse();
  inline bool isLegal() const { return mLegal; }

protected:
  std::string mRawText;
  std::string mName;
  std::vector<Arg> mArgs;
  uint mNextArgIdx = 0;
  bool mLegal = false;
};