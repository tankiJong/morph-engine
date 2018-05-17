#include "ParticleEmitter.hpp"
#include "Engine/Debug/Draw.hpp"
#include "Engine/Renderer/Geometry/Mesher.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/Shader/Material.hpp"
#include "Engine/Renderer/Renderable/Renderable.hpp"
#include "Engine/Renderer/Camera.hpp"

particle_t::particle_t(float spawnTime, float lifeTime)
  : timeSpawnSec(spawnTime)
  , timeDecaySec(spawnTime + lifeTime) {}

void particle_t::update(float dsec) {
  if(mass != 0 ) {
    vec3 acceleration = force / mass;
    velocity += dsec * acceleration;
  }
  position += dsec * velocity;
  force = vec3::zero;
  age += dsec;
}

ParticleEmitter::ParticleEmitter()
  : mRenderable(new Renderable()) {
  mRenderable->material(Resource<Material>::get("material/particle"));
  mRenderable->transform() = &transform;
  Mesher ms;
  mMesh = ms.createMesh<vertex_pcu_t>();
  mRenderable->mesh() = mMesh;
}

void ParticleEmitter::update() {
  float totalSec = (float)mSpawnInterval.clock().total.second;
  float dsec = (float)mSpawnInterval.clock().frame.second;

  uint toSpawn = mSpawnInterval.flush();
  emit(toSpawn);


  for(size_t i = mParticles.size() - 1, count = mParticles.size(); i < count; --i) {
    particle_t& p = mParticles[i];
    updateFunc(p, dsec);
    p.update(dsec);
    if(p.isDead(totalSec)) {
      mParticles[i] = mParticles.back();
      mParticles.pop_back();
    }
  }
}

void ParticleEmitter::setup(const Camera& cam) {
  Mesher ms;

  vec3 right = (transform.worldToLocal() * vec4(cam.right(), 0.f)).xyz();
  vec3 up = (transform.worldToLocal() * vec4(cam.up(), 0.f)).xyz();

  ms.begin(DRAW_TRIANGES);
  for(const particle_t& p: mParticles) {
    ms.color(p.tint);
    ms.quad(p.position, right, up, p.size);
  }
  ms.end();

  ms.resetMesh<vertex_pcu_t>(*mMesh);
}

void ParticleEmitter::emit(uint count) {
  for(uint i = 0; i < count; i++) {
    float currentTime = (float)mSpawnInterval.clock().total.second;
    mParticles.emplace_back(currentTime, mLifeTime.getRandomInRange());
    particle_t& p = mParticles.back();

    p.position = transform.localPosition();
    p.velocity = vec3(getRandomf01(), getRandomf01(), getRandomf01()) * getRandomf(.1f, .2f);
    setupFunc(p);
    // set up params
  }
}
