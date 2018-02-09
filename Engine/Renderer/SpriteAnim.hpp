#pragma once
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/IntRange.hpp"
#include "Engine/Core/Xml.hpp"

class Texture;
enum SpriteAnimMode {
  SPRITE_ANIM_MODE_PLAY_TO_END,	// Play from time=0 to durationSeconds, then finish
  SPRITE_ANIM_MODE_LOOPING,		// Play from time=0 to end then repeat (never finish)
//  SPRITE_ANIM_MODE_PINGPONG, 	// optional, play forwards, backwards, forwards...
  NUM_SPRITE_ANIM_MODES
};

class SpriteSheet;

class SpriteAnimDefinition {
  friend class   SpriteAnim;
  friend class SpriteAnimSetDefinition;
  friend class SpriteAnimSet;
public:
  /**
   * \brief input node format: <SpriteAnim name="walkEast" spriteIndexes="1,2,3">
   * \param node 
   */
  explicit SpriteAnimDefinition(const SpriteSheet& spriteSheet, const Xml& node);
protected:
  explicit SpriteAnimDefinition(const SpriteSheet& spriteSheet, float durationSeconds,
                                     SpriteAnimMode playbackMode, std::vector<int> frames);
  std::vector<int> m_frameIndexes;
  float m_durationSeconds;
  const SpriteSheet& m_spriteSheet;
  SpriteAnimMode m_playMode = SPRITE_ANIM_MODE_PLAY_TO_END;
  std::string m_name;
};

class SpriteAnim {
  friend SpriteAnimSet;
public:
  SpriteAnim(const SpriteSheet& spriteSheet, float durationSeconds,
                  SpriteAnimMode playbackMode, int startSpriteIndex, int endSpriteIndex);
  explicit SpriteAnim(const SpriteAnimDefinition& definition);
  ~SpriteAnim();
  void update(float deltaSeconds);
  aabb2 getCurrentTexCoords() const;	// Based on the current elapsed time
  const Texture& getTexture() const;
  void pause();					// Starts unpaused (playing) by default
  void resume();				// Resume after pausing
  void reset();					// Rewinds to time 0 and starts (re)playing
  bool isFinished() const { return m_isFinished; }
  bool isPlaying() const { return m_isPlaying; }
  float getDurationSeconds() const { return m_definition->m_durationSeconds; }
  float getSecondsElapsed() const { return m_elapsedSeconds; }
  float getSecondsRemaining() const { return m_definition->m_durationSeconds - m_elapsedSeconds; }
  float getFractionElapsed() const { return m_elapsedSeconds / m_definition->m_durationSeconds; }
  float getFractionRemaining() const { return 1.f - m_elapsedSeconds / m_definition->m_durationSeconds; }
  void setSecondsElapsed(float secondsElapsed);	    // Jump to specific time
  void setFractionElapsed(float fractionElapsed);    // e.g. 0.33f for one-third in
protected:
  const SpriteAnimDefinition* m_definition = nullptr;
  const bool m_fromDefinition;
  bool m_isFinished = false;
  bool m_isPlaying = false;
  float m_elapsedSeconds = 0;
};
