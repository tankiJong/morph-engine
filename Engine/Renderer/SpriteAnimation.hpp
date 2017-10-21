#pragma once
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/IntRange.hpp"

class Texture;
enum SpriteAnimationMode {
  SPRITE_ANIM_MODE_PLAY_TO_END,	// Play from time=0 to durationSeconds, then finish
  SPRITE_ANIM_MODE_LOOPING,		// Play from time=0 to end then repeat (never finish)
                              //  SPRITE_ANIM_MODE_PINGPONG, 	// optional, play forwards, backwards, forwards...
                              NUM_SPRITE_ANIM_MODES
};

class SpriteSheet;

class SpriteAnimation {
public:
  SpriteAnimation(const SpriteSheet& spriteSheet, float durationSeconds,
                  SpriteAnimationMode playbackMode, int startSpriteIndex, int endSpriteIndex);

  void update(float deltaSeconds);
  AABB2 getCurrentTexCoords() const;	// Based on the current elapsed time
  const Texture& getTexture() const;
  void pause();					// Starts unpaused (playing) by default
  void resume();				// Resume after pausing
  void reset();					// Rewinds to time 0 and starts (re)playing
  bool isFinished() const { return m_isFinished; }
  bool isPlaying() const { return m_isPlaying; }
  float getDurationSeconds() const { return m_durationSeconds; }
  float getSecondsElapsed() const { return m_elapsedSeconds; }
  float getSecondsRemaining() const { return m_durationSeconds - m_elapsedSeconds; }
  float getFractionElapsed() const { return m_elapsedSeconds / m_durationSeconds; }
  float getFractionRemaining() const { return 1.f - m_elapsedSeconds / m_durationSeconds; }
  void setSecondsElapsed(float secondsElapsed);	    // Jump to specific time
  void setFractionElapsed(float fractionElapsed);    // e.g. 0.33f for one-third in
protected:
  const SpriteSheet& m_spriteSheet;
  IntRange m_spriteIndexRange;
  SpriteAnimationMode m_playMode;
  bool m_isFinished = false;
  bool m_isPlaying = false;
  float m_durationSeconds = 0;
  float m_elapsedSeconds = 0;
};
