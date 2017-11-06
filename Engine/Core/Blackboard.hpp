#pragma once
#include <string>
#include <map>
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/Xml.hpp"

class Xml;
class Blackboard {
public:
  Blackboard(const char* path);
  template<typename T>
  void set(const std::string& key, const T& val) {
    const std::string& str = toString(val);
    m_properties.insert(key, str);
  };

  template<typename T>
  T get(const std::string& key, const T& defaultVal) {
    auto pair = m_properties.find(key);
    if(pair != m_properties.end()) {
      return parse<T>(pair->second);
    } else {
      return defaultVal;
    }
  };

private:
  std::map<std::string, std::string> m_properties;
  Xml m_source;
};
