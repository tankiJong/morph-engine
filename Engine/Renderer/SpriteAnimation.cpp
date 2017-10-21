#include "Engine/Renderer/SpriteAnimation.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Math/MathUtils.hpp"
SpriteAnimation::SpriteAnimation(const SpriteSheet& spriteSheet, 
                                 float durationSeconds, 
                                 SpriteAnimationMode playbackMode,
                                 int startSpriteIndex, 
                                 int endSpriteIndex)
: m_spriteSheet(spriteSheet)
, m_playMode(playbackMode)
, m_durationSeconds(durationSeconds)
, m_spriteIndexRange(startSpriteIndex, endSpriteIndex) {
  
}

void SpriteAnimation::update(float deltaSeconds) {
  m_elapsedSeconds += deltaSeconds;
}
AABB2 SpriteAnimation::getCurrentTexCoords() const {
  float secPerUnit = m_durationSeconds / float(m_spriteIndexRange.numIntIncluded());
  int curremtTexIdx = int(floor(m_elapsedSeconds / secPerUnit)) + m_spriteIndexRange.min;
  
  return m_spriteSheet.getTexCoordsByIndex(curremtTexIdx);
}
const Texture& SpriteAnimation::getTexture() const {
  return m_spriteSheet.getTexture();
}
void SpriteAnimation::pause() {
  m_isPlaying = false;
}
void SpriteAnimation::resume() {
  m_isPlaying = true;
}
void SpriteAnimation::reset() {
  m_isPlaying = true;
}
void SpriteAnimation::setSecondsElapsed(float secondsElapsed) {
  m_elapsedSeconds = secondsElapsed;
}
void SpriteAnimation::setFractionElapsed(float fractionElapsed) {
  
}
