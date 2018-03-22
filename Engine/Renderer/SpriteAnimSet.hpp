#pragma once
#include "Engine/Persistence/xml.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Renderer/SpriteAnim.hpp"
#include <map>

class SpriteAnimDefinition;
class Renderer;

class SpriteAnimSetDefinition {
  friend class SpriteAnimSet;
public:
  SpriteAnimSetDefinition(const Xml& node, Renderer& render);
  ~SpriteAnimSetDefinition();
  std::string m_defaultAnimName = "Idle";
protected:
  std::map<std::string, SpriteAnimDefinition*> m_animations;
  SpriteSheet* m_spriteSheet;
};


class SpriteAnimSet {
public:
  SpriteAnimSet(const SpriteAnimSetDefinition& definition);
  void play(const char* name);
  void update(float dSecond);
  aabb2 currentTexCorrds() const;
  const Texture& currentTexture() const;
protected:
  SpriteAnim* spawnAnim(const char* name) const;
  const SpriteAnimSetDefinition& m_definition;
  SpriteAnim* m_currentAnim = nullptr;
  SpriteAnim* m_nextAnim = nullptr;
  SpriteAnim m_defaultAnim;
};
