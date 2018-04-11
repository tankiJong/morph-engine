#pragma once
#include "Engine/Core/common.hpp"
#include "Engine/Debug/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include <map>
#include "Engine/File/Path.hpp"
template<typename T>
using ResDef = std::tuple<std::string, T*>;

template<typename T>
class Resource {
  using ResourceType = Resource<T>;

public:
  static bool define(std::string_view name, T* res) {
    if (auto kv = sDatabase.find(name); kv != sDatabase.end()) {
      ERROR_RECOVERABLE("resource already exists");
      return false;
    }
    sDatabase[std::string(name)].reset(res);
    return true;
  }

  static S<const T> get(std::string_view name) {
    if (auto kv = sDatabase.find(name); kv != sDatabase.end()) {
      return { kv->second };
    } else {
      ERROR_RECOVERABLE(Stringf("fail to find resource of name: %s", name.data()));
      return S<const T>{};
    }
  }

  static bool define(const fs::path& file) {
    auto [name, res] = load(file);
    return define(name, res);
  }
protected:
  static std::map<std::string, S<T>, std::less<>> sDatabase;
  static ResDef<T> load(const fs::path& file);
};

template<typename T>
std::map<std::string, S<T>, std::less<>> Resource<T>::sDatabase = {};
