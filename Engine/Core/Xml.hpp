#pragma once
#include "ThirdParty/pugixml/pugixml.hpp"
#include <functional>
#include "StringUtils.hpp"
#include <algorithm>

namespace pugi {
  class xml_node;
  class xml_attribute;
  class xml_document;
}

class Xml {

public:
  Xml(const char* path);
  Xml(const char* content, size_t size);
  ~Xml();
  std::string name() const;
  std::string value() const;
  Xml parent() const;
  Xml firstChild(const char* name = nullptr) const;
  Xml lastChild() const;
  Xml nextSibling() const;
  Xml previousSibling() const;
  bool isEmpty() const;
  
  template<typename T>
  inline T attribute(const char* attributeName, T defaultValue) const {
    return parseXmlAttribute(*this, attributeName, defaultValue);
  }

  template<typename Functor>
  inline Functor traverseChilds(const Functor& fn) const {
    for(auto& node: m_node) {
      Xml child(node, m_document);
      fn(child);
    }

    return (fn);
  }


  template<typename Functor>
  inline Functor traverseChilds(const std::string& name, const Functor& fn) const {
    for (auto& node : m_node) {
      if (node.name() != name) continue;
      Xml child(node, m_document);
      fn(child);
    }

    return (fn);
  }

  template<typename Functor>
  inline Functor traverseAttributes(const Functor& fn) const {
    for (auto& attr : m_node.attributes()) {
      fn(attr.name(), attr.value());
    }
    return (fn);
  }

  std::string operator[](const char* attribute) const;

protected:
  Xml(const pugi::xml_node& xmlNode, pugi::xml_document* doc, bool isRoot = false);
  pugi::xml_node m_node;
  pugi::xml_document* m_document = nullptr;
  bool m_isRoot = false;

};

template<typename T>
inline T parseXmlAttribute(const Xml& ele, const char* attributeName, T defaultValue) {
  auto raw = ele[attributeName];
  if (raw.length() == 0) return defaultValue;
  T result = parse<T>(raw);
  return result;
}

template<>
inline std::string parseXmlAttribute(const Xml& ele, const char* attributeName, std::string defaultValue) {
  auto raw = ele[attributeName];
  return (raw.length() == 0) ? defaultValue : raw;
}

template<typename T, typename A>
inline std::vector<T, A> parseXmlAttribute(const Xml& ele, const char* attributeName, std::vector<T, A> defaultValue) {
  auto raw = ele[attributeName];

  return (raw.length() == 0) ? defaultValue : parse<T, A>(raw.c_str(), " ,");
}