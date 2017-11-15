#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Math/IntVector2.hpp"
#include "Engine/Core/Xml.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Renderer/SpriteAnim.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "SpriteAnimSet.hpp"

SpriteAnimSetDefinition::SpriteAnimSetDefinition(const Xml& node, Renderer& render) {
  GUARANTEE_OR_DIE(node.name() == "SpriteAnimSet", "xml node tag name unmatched");

  IntVector2 layout = node.attribute("spriteLayout", IntVector2::zero);

  // TODO: create spriteSheet
  m_spriteSheet = new SpriteSheet(*render.createOrGetTexture(node["spriteSheet"]), layout.x, layout.y);

  node.traverseChilds([&anims = m_animations, &sprite = *m_spriteSheet](const Xml& node) {
    auto name = node["name"];
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
  : m_definition(definition) {
  for(auto& anim: definition.m_animations) {
    m_animations[anim.first] = new SpriteAnim(*anim.second);
  }
}
