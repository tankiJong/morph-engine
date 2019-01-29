#pragma once
#include "Engine/Core/common.hpp"
#include "Engine/Debug/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include <map>
#include "Engine/File/Path.hpp"
template<typename T>
using ResDef = std::tuple<std::string, T*>;


template<typename T>
struct ResourceItem {
  S<T> res;
  fs::path path;
};

class ResourceDecl {
public:
  virtual void init() = 0;
};

template<typename T>
class Resource {
  using ResourceType = Resource<T>;

public:

  /* default template expension will always return nullptr, which means, by default, resource is not cloneable.
   * If you want to make some resource cloneable, you should create specialization of this function.
   */
  static owner<T*> clone(S<const T> res);

  /*
   * This is an convenient way to call the upper function. In most case DO NOT need to implement this.
   */
  static owner<T*> clone(std::string_view name) {
    S<const T> res = get(name);

    return clone(res);
  }
  
  /**
   * \brief 
   * \param name it's actual res id, eg, for shader "shader/default"
   * \param res  resource
   * \param path the file path when load in, used for reloading
   * \return whether the action is successful
   */
  static bool define(std::string_view name, T* res, const fs::path& path = "") {
    if (sDatabase == nullptr) sDatabase = new std::map<std::string, ResourceItem<T>, std::less<>>();
    if (auto kv = sDatabase->find(name); kv != sDatabase->end()) {
      ERROR_RECOVERABLE("resource already exists");
      return false;
    }
    if (res == nullptr) {
      ERROR_RECOVERABLE("fail to define resource(nullptr)");
      return false;
    }
    auto& item = (*sDatabase)[std::string(name)];
    item.res.reset(res);
    item.path = path;
    return true;
  }

  static bool define(S<T> res, std::string_view name, const fs::path& path = "") {
    if (sDatabase == nullptr) sDatabase = new std::map<std::string, ResourceItem<T>, std::less<>>();
    if (auto kv = sDatabase->find(name); kv != sDatabase->end()) {
      ERROR_RECOVERABLE(Stringf("'%s' already exists", name.data()));
      return false;
    }
    if (res == nullptr) {
      ERROR_RECOVERABLE("fail to define resource(nullptr)");
      return false;
    }
    auto& item = (*sDatabase)[std::string(name)];
    item.res = std::move(res);
    item.path = path;
    return true;
  }

  static S<const T> get(std::string_view name) {
    const ResourceItem<T>* resource = res(name);
    return resource == nullptr ? S<const T>{} : resource->res;
  }

  static const ResourceItem<T>* res(std::string_view name) {
    deferDefine();

    if (auto kv = sDatabase->find(name); kv != sDatabase->end()) {
      return &kv->second;
    } else {
      ERROR_RECOVERABLE(Stringf("fail to find resource of name: %s", name.data()));
      return nullptr;
    }
  }

  static bool reload() {
    
    bool success = true;
    for(auto& [k,res]: *sDatabase) {
      success = success && reload(res.path, *res.res);
    }

    return success;
  }
  static bool define(const std::string& file) {
    auto [name, res] = load(file);
    return define(name, res, file);
  }

  static void declare(ResourceDecl* decl) {
    if (sDecls == nullptr) sDecls = new std::vector<ResourceDecl*>();
    sDecls->push_back(decl);
  }

  static void deferDefine() {
    if (sDecls == nullptr) return;
    for(ResourceDecl* decl: *sDecls) {
      decl->init();
    }
    sDecls->clear();
  }
protected:
  static std::map<std::string, ResourceItem<T>, std::less<>>* sDatabase;
  static std::vector<ResourceDecl*>* sDecls;


  /*
   * This function is required for the resource system to work.
   */
  static ResDef<T> load(const std::string& file);
  static bool reload(const fs::path&, T&);
};

template<typename T>
std::map<std::string, ResourceItem<T>, std::less<>>* Resource<T>::sDatabase = nullptr;

template<typename T>
std::vector<ResourceDecl*>* Resource<T>::sDecls = nullptr;

#define DEF_RESOURCE(Type, name) \
class APPEND(__RES_DEF, __LINE__): public ResourceDecl { \
public: \
  using ResType = Type; \
  static S<ResType> _Reg(); \
  APPEND(__RES_DEF, __LINE__)() { \
    Resource<ResType>::declare(this); \
  }; \
  virtual void init () override { \
    auto res = _Reg(); \
    Resource<ResType>::define(res, name); \
  }; \
}; \
static APPEND(__RES_DEF, __LINE__) APPEND(__RES_DEF_, __LINE__); \
inline S<Type> APPEND(__RES_DEF, __LINE__)::_Reg()