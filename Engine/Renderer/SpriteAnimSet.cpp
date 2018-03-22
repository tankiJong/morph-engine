#include "Engine/Debug/ErrorWarningAssert.hpp"
#include "Engine/Math/Primitives/ivec2.hpp"
#include "Engine/Persistence/xml.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Renderer/SpriteAnim.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "SpriteAnimSet.hpp"

SpriteAnimSetDefinition::SpriteAnimSetDefinition(const Xml& node, Renderer& render) {
  GUARANTEE_OR_DIE(node.name() == "SpriteAnimSet", "xml node tag name unmatched");

  ivec2 layout = node.attribute("spriteLayout", ivec2::zero);
  m_defaultAnimName = node.attribute("default", m_defaultAnimName);
  m_spriteSheet = new SpriteSheet(*render.createOrGetTexture(node["spriteSheet"]), layout.x, layout.y);

  node.traverseChilds([&anims = m_animations, &sprite = *m_spriteSheet](const Xml& node) {
    std::string name = node["name"];
    auto kv = anims.find(name);
    GUARANTEE_RECOVERABLE(kv == anims.end(), Stringf("anim definition \"%s\" already exists", name.c_str()));
    anims[name] = new SpriteAnimDefinition(sprite, node);
  });
}

SpriteAnimSetDefinition::~SpriteAnimSetDefinition() {
  for (auto& pair : m_animations) {
    delete pair.second;
  }
  m_animations.clear();

  delete m_spriteSheet;
  m_spriteSheet = nullptr;
}

SpriteAnimSet::SpriteAnimSet(const SpriteAnimSetDefinition& definition)
  : m_definition(definition)
  , m_defaultAnim(*m_definition.m_animations.at(m_definition.m_defaultAnimName)){
  m_currentAnim = &m_defaultAnim;
}

void SpriteAnimSet::play(const char* name) {
  if (m_nextAnim != nullptr) {
    if(m_nextAnim->m_definition->m_name == name) return;
    delete m_nextAnim;
  }
  
  m_nextAnim = spawnAnim(name);
}

SpriteAnim* SpriteAnimSet::spawnAnim(const char* name) const {
  auto it = m_definition.m_animations.find(name);

  GUARANTEE_OR_DIE(it != m_definition.m_animations.end(), "undefined animation name");
  return new SpriteAnim(*it->second);
}

void SpriteAnimSet::update(float dSecond) {
  if(m_currentAnim->isFinished()) {
    if (m_nextAnim == nullptr) {
      m_currentAnim = &m_defaultAnim;
    } else {

      if (m_currentAnim != &m_defaultAnim)  delete m_currentAnim;

      m_currentAnim = m_nextAnim;
      m_nextAnim = nullptr;
    }
    m_currentAnim->reset();
  }
  m_currentAnim->update(dSecond);
}

aabb2 SpriteAnimSet::currentTexCorrds() const {
  return m_currentAnim->getCurrentTexCoords();
}

const Texture& SpriteAnimSet::currentTexture() const {
  return m_currentAnim->getTexture();
}
