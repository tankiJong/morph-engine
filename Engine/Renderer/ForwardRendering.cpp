#include "ForwardRendering.hpp"
#include "Engine/Renderer/RenderScene.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/RenderTask.hpp"
#include "Engine/Renderer/Renderable/Renderable.hpp"
#include "Engine/Framework/Light.hpp"
#include "Engine/Renderer/Shader/Material.hpp"
#include "Engine/Renderer/Shader/ShaderPass.hpp"
#include "Engine/Application/Window.hpp"
#include "Engine/Renderer/TextureArray.hpp"
#include "Engine/Renderer/Shader/Shader.hpp"
#include "Engine/Renderer/glFunctions.hpp"
#include "Engine/Renderer/AfterEffect/BloomEffect.hpp"
#include "Engine/Renderer/AfterEffect/FogEffect.hpp"

ForwardRendering::ForwardRendering(Renderer* renderer): mRenderer(renderer) {
  mShadowInfo = new RenderTarget(Light::SHADOW_MAP_SIZE * NUM_MAX_LIGHTS, Light::SHADOW_MAP_SIZE, TEXTURE_FORMAT_D24S8);
  vec2 size = Window::Get()->bounds().size();
  mBloomTexture = new RenderTarget((uint)size.x, (uint)size.y);
}

void ForwardRendering::render(RenderScene& scene) {
  scene.sortCamera();
  for(Camera* cam: scene.cameras()) {
    renderView(scene, *cam);
  }
}

/*
 * - create render tasks
 * - sort render task
 * - prepass
 * - pass
 * - postpass
 */
void ForwardRendering::renderView(RenderScene& scene, Camera& cam) {
  static std::vector<RenderTask> tasks;
  tasks.clear();
  mRenderer->setCamera(&cam);

  TODO("replace with sky box or something")
  ;
  if(cam.queryFlag(CAM_CLEAR_COLOR)) {
    mRenderer->cleanColor(Rgba::black);
  }

  if(cam.queryFlag(CAM_CLEAR_DEPTH)) {
    mRenderer->enableDepth(COMPARE_LEQUAL, true);
    mRenderer->clearDepth(1.f);
  }

  for(const Renderable* renderable: scene.Renderables()) {
    renderable->pushRenderTask(tasks, scene, cam);
  }

  RenderTask::sort(cam, tasks);
  
  prepass(scene, tasks, cam);
  pass(scene, tasks, cam);
  postpass(scene, tasks, cam);

}

void ForwardRendering::createShadowMap(Light& light, span<RenderTask> tasks) {
  mRenderer->setCamera(&light.camera());
  mRenderer->enableDepth(COMPARE_LEQUAL, true);
  mRenderer->clearDepth(1.f);
  // render opaque only
  for(RenderTask& task: tasks) {
    static_assert(SHADER_LAYER_ALPHA > SHADER_LAYER_OPAQUE);
    if(task.layer == SHADER_LAYER_ALPHA) break;
    
    mRenderer->setModelMatrix(task.transform->localToWorld());
    mRenderer->drawMesh(*task.mesh);
  }
}

/* pre-pass:
 * 1. do camera pre-pass
 * 2. create shadow info texture
 * 3. bake shadow map for each light casting shadow
 * 4. bind bloom texture as second target on camera
*/
void ForwardRendering::prepass(RenderScene& scene, span<RenderTask> tasks, Camera& cam) {
  cam.prepass();

  mRenderer->setShader(Resource<Shader>::get("shader/default").get(), 0);
  for(Light* lit: scene.lights()) {
    if(lit->castShadow) {
      createShadowMap(*lit, tasks);
    }
  }

  Camera tempCam;
  tempCam.setDepthStencilTarget(mShadowInfo);
  mRenderer->setCamera(&tempCam);
  mRenderer->clearDepth(1.f);

  if (cam.queryFlag(CAM_EFFECT_BLOOM)) {
    cam.setColorTarget(mBloomTexture, 1);
  }
}

/* post-pass:
* - bloom
*   - blur sample the bloom texture
*   - bloom the color target
*/
void ForwardRendering::postpass(RenderScene& scene, span<RenderTask> tasks, Camera& cam) {
  delete effect;
  effect = new BloomEffect(*mRenderer,
                           *cam.colorTarget(0),
                           *Resource<Shader>::get("shader/effect/bloom"),
                           *cam.colorTarget(1));
  // effect->apply();

  FogEffect fog(*mRenderer,
                *cam.colorTarget(0),
                *Resource<Shader>::get("shader/effect/fog"), *cam.depthTarget());

  // fog.apply();
  mRenderer->setCamera(&cam);
}

ForwardRendering::~ForwardRendering() {
  SAFE_DELETE(mBloomTexture);
  SAFE_DELETE(mShadowInfo);
}

/* pass
 * - draw
 */
void ForwardRendering::pass(RenderScene& scene, span<RenderTask> tasks, Camera& cam) {
  
  // draw
  mRenderer->setCamera(&cam);
  auto lights = scene.lights();
  for (const RenderTask& task : tasks) {
    mRenderer->setModelMatrix(task.transform->localToWorld());
    for (uint i = 0; i < task.lightCount; ++i) {
      Light* lit = lights[task.lightIndices[i]];
      if(lit->castShadow) {
        mRenderer->copyTexture(&lit->shadowMap(), 0, 0,
                               mShadowInfo, Light::SHADOW_MAP_SIZE * i, 0, 
                               Light::SHADOW_MAP_SIZE, Light::SHADOW_MAP_SIZE);
      }
      mRenderer->setLight(i, lit->info());
    }

    mRenderer->setMaterial(task.material, task.passIndex);
    const eTextureSlot binding = task.material->shader()->pass(task.passIndex).prog()->info().texture("gTexShadowMap");
    if(binding < NUM_TEXTURE_SLOT) {
      mRenderer->setTexture(binding, mShadowInfo);
    }
    mRenderer->drawMesh(*task.mesh);
  }
}
  
