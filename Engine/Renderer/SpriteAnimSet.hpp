#pragma once
#include "Engine/Core/Xml.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include <map>

class SpriteAnim;
class SpriteAnimDefinition;
class Renderer;

class SpriteAnimSetDefinition {
  friend class SpriteAnimSet;
public:
  SpriteAnimSetDefinition(const Xml& node, Renderer& render);
  ~SpriteAnimSetDefinition();
protected:
  std::map<std::string, SpriteAnimDefinition*> m_animations;
  std::string m_defaultAnimName = "Idle";
  SpriteSheet* m_spriteSheet;
};


class SpriteAnimSet {
public:
  SpriteAnimSet(const SpriteAnimSetDefinition& definition);
protected:
  const SpriteAnimSetDefinition& m_definition;
  std::map<std::string, SpriteAnim*> m_animations;
  SpriteAnim* m_currentAnim = nullptr;
  SpriteAnim* m_nextAnim = nullptr;
};