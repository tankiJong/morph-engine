#include "Engine/Core/Xml.hpp"
#include "ThirdParty/pugixml/pugixml.hpp"

using namespace pugi;

Xml::Xml(const char* path) {
  xml_document* doc = new xml_document();
  doc->load_file(path);
  m_node = doc->first_child();
  m_document = doc;
}

Xml::Xml(const char* content, size_t size) {
  xml_document* doc = new xml_document();
  doc->load_buffer(content, size);
  m_node = doc->first_child();
  m_document = doc;
}
  
Xml::~Xml() {
  if(m_isRoot) {
    delete m_document;
  }

  m_document = nullptr;
}

std::string Xml::name() const {
  return m_node.name();
}
std::string Xml::value() const {
  return m_node.value();
}
Xml Xml::parent() const {
  return { m_node.parent(), m_document };
}

Xml Xml::firstChild(const char* name) const {
  if(name == nullptr) {
    return { m_node.first_child(), m_document };
  }

  return { m_node.child(name), m_document };
}
Xml Xml::lastChild() const {
  return { m_node.last_child(), m_document };
}
Xml Xml::nextSibling() const {
  return { m_node.next_sibling(), m_document };
}
Xml Xml::previousSibling() const {
  return { m_node.previous_sibling(), m_document };
}

bool Xml::isEmpty() const {
  return m_node.empty();
}

bool Xml::hasAttribute(const char* name) const {
  return *m_node.attribute(name).name() != '\0';
}

xml::Attribute Xml::appendAttribute(const char* name, const char* value) {
  if(hasAttribute(name)) {
    (*this)[name] = value;
  } else {
    m_node.append_attribute(name) = value;
  }
  return (*this)[name];
}

Xml Xml::selectNode(const char* xpath) const {
  xpath_node node = m_node.select_node(xpath);
  GUARANTEE_OR_DIE(node, "query return empty");
  return { node.node(), m_document };
}

bool Xml::save(const char* path) const {
  return m_document->save_file(path);
}

void Xml::save(std::ostream& stream) const {
  m_document->save(stream);
}

void Xml::print(std::ostream& stream) const {
  m_node.print(stream);
}

const xml::Attribute Xml::operator[](const char* attribute) const {
  return m_node.attribute(attribute);
}

xml::Attribute Xml::operator[](const char* attribute) {
  return m_node.attribute(attribute);
}

Xml::Xml(const pugi::xml_node& xmlNode, pugi::xml_document* doc, bool isRoot)
  : m_node(xmlNode) 
  , m_document(doc)
  , m_isRoot(isRoot) {
}


