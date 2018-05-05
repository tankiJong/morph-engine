#pragma once

#include "Engine/Core/common.hpp"
#include "Engine/Math/Transform.hpp"
#include "Engine/Renderer/type.h"
#include "Engine/Core/Time/Clock.hpp"
#include <vector>
#include "Engine/Math/Primitives/FloatRange.hpp"
#include "Engine/Core/Delegate.hpp"
#include "Engine/Math/Curves.hpp"

class Camera;
class Renderable;
class Mesh;

struct particle_t {
  particle_t(float spawnTime, float lifeTime);
  vec3 position = vec3::zero;
  float mass = 1.f;
  vec3 velocity = vec3::zero;
  Rgba tint = Rgba::white;
  vec3 force = vec3::zero;
  vec2 size = vec2::one * .05f;
  
  float age= 0;
  float timeSpawnSec;
  float timeDecaySec;
  bool isDead(float worldTime) const { return worldTime > timeDecaySec; };
  float normalizedAge() const { return clampf01(age / (timeDecaySec - timeSpawnSec)); }
  void update(float dsec);
};

class ParticleEmitter {
public:
  ParticleEmitter();
  void update();
  void setup(const Camera& cam);
  void emit(uint count = 1);

  delegate<void(particle_t&)> setupFunc = [](particle_t&) {};
  delegate<void(particle_t&, float)> updateFunc = [](particle_t&, float) {};
  
  inline void frequency(float numPerSecond) { mSpawnInterval.duration = 1.f / numPerSecond; };
  inline Renderable& renderable() { return *mRenderable; }
  inline void setLifeTime(float min, float max) { mLifeTime.min = min; mLifeTime.max = max; }

  bool isActive = true;
  Transform transform;
protected:
  Renderable* mRenderable = nullptr;
  std::vector<particle_t> mParticles;
  Interval mSpawnInterval;
  FloatRange mLifeTime;
  Mesh* mMesh;
};