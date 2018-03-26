#pragma once
#include "Engine/Core/common.hpp"
#include "Engine/File/Path.hpp"
#include "Engine/Debug/ErrorWarningAssert.hpp"
//
//template<typename T>
//class Resource {
//  using ResourceType = Resource<T>;
//public:
//  static void load(fs::path path) {
//    Xml sSrc(path);
//    sSrc.traverseChilds([&defs = sDatabase](Xml& node) {
//      if (node.isEmpty()) return;
//      std::string text = node.value();
//      std::string id = node["id"];
//      auto kv = defs.find(id);
//      GUARANTEE_RECOVERABLE(kv == defs.end(), Stringf("definition \"%s\" already exists", id.c_str()));
//      T* res = fromXml<T>(node);
//      GUARANTEE_OR_DIE(res != nullptr, "fail to create resource");
//      defs[id] = res;
//    });
//  }
//
//  static const T* get(std::string_view name) {
//    if (auto kv = sDatabase.find(name); kv != sDatabase.end()) {
//      return kv->second;
//    } else {
//      ERROR_RECOVERABLE(Stringf("fail to find resource of name: %s", name.data()));
//      return nullptr;;
//    }
//  }
//  //
//  //  static const T* get(const std::string& name) {
//  //    if (auto kv = sDatabase.find(std::ref(name)); kv != sDatabase.end()) {
//  //      return kv->second;
//  //    } else {
//  //      ERROR_RECOVERABLE(Stringf("fail to find resource of name: %s", name.data()));
//  //      return nullptr;;
//  //    }
//  //  }
//
//  static owner<T*> produce(std::string name) {
//    auto kv = sDatabase.find(name);
//    GUARANTEE_RECOVERABLE(kv != sDatabase.end(), Stringf("fail to find resource of name: %s", name.data()));
//    if (kv == sDatabase.end()) return nullptr;
//
//    return new T(*kv->second);
//  }
//
//protected:
//  static std::map<std::string, owner<T*>, std::less<>> sDatabase;
//};
//
