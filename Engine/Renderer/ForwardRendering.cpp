#include "ForwardRendering.hpp"
#include "Engine/Renderer/RenderScene.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/RenderTask.hpp"
#include "Engine/Renderer/Renderable/Renderable.hpp"
#include "Engine/Framework/Light.hpp"

void ForwardRendering::render(RenderScene& scene) {
  scene.sortCamera();
  for(Camera* cam: scene.cameras()) {
    renderView(scene, cam);
  }
}

void ForwardRendering::renderView(RenderScene& scene, Camera* cam) {
  static std::vector<RenderTask> tasks;
  tasks.clear();
  mRenderer->setCamera(cam);
  
  TODO("replace with sky box or something");
  if(cam->queryFlag(CAM_CLEAR_COLOR)) {
    mRenderer->cleanColor(Rgba::black);
  }

  if(cam->queryFlag(CAM_CLEAR_DEPTH)) {
    mRenderer->enableDepth(COMPARE_LEQUAL, true);
    mRenderer->clearDepth(1.f);
  }

  for(const Renderable* renderable: scene.Renderables()) {
    tasks.emplace_back();

    RenderTask& rt = tasks.back();

    rt.camera = cam;
    rt.mesh = renderable->mesh();
    rt.transform = &renderable->transform();

    rt.material = renderable->material();

    if(renderable->useLight()) {
      scene.lightContributorsAt(renderable->transform().position(), 
                                 rt.lightIndices, &rt.lightCount);
    }
  }

  RenderTask::sort(*cam, tasks);

  // draw
  for(const RenderTask& task: tasks) {
    mRenderer->setMaterial(task.material);
    mRenderer->setModelMatrix(task.transform->localToWorld());
    auto lights = scene.lights();
    for(uint i = 0; i < task.lightCount; ++i) {
      mRenderer->setLight(i, lights[task.lightIndices[i]]->info);
    }
    mRenderer->drawMesh(*task.mesh);
  }
  
}
