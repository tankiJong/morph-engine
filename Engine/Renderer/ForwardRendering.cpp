#include "ForwardRendering.hpp"
#include "Engine/Renderer/RenderScene.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/RenderTask.hpp"
#include "Engine/Renderer/Renderable/Renderable.hpp"
#include "Engine/Framework/Light.hpp"
#include "Engine/Renderer/Shader/Material.hpp"
#include "Engine/Renderer/Shader/ShaderPass.hpp"

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

/* pre-pass:
 * 1. do camera pre-pass
 * 3. bake shadow map for each light casting shadow
 * 2. bind bloom texture as second target on camera
*/
void ForwardRendering::prepass(RenderScene& scene, span<RenderTask> tasks, Camera& cam) {
  cam.prepass();
  for(Light* lit: scene.lights()) {
    
  }
}

/* post-pass:
* - bloom
*   - blur sample the bloom texture
*   - bloom the color target
*/
void ForwardRendering::postpass(RenderScene& scene, span<RenderTask> tasks, Camera& cam) {}

/* pass
 * - draw
 */
void ForwardRendering::pass(RenderScene& scene, span<RenderTask> tasks, Camera& cam) {

  // draw
  auto lights = scene.lights();
  for (const RenderTask& task : tasks) {
    mRenderer->setModelMatrix(task.transform->localToWorld());
    for (uint i = 0; i < task.lightCount; ++i) {
      mRenderer->setLight(i, lights[task.lightIndices[i]]->info);
    }

    mRenderer->setMaterial(task.material, task.passIndex);
    mRenderer->drawMesh(*task.mesh);
  }
}
  
