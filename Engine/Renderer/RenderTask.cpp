#include "RenderTask.hpp"
#include "Engine/Renderer/Shader/Material.hpp"
#include "Engine/Renderer/Shader/Shader.hpp"
#include "Engine/Math/Transform.hpp"
#include "Engine/Renderer/Camera.hpp"

void RenderTask::sort(const Camera& cam, span<RenderTask> tasks) {
  std::sort(tasks.begin(), tasks.end(), [&cam](RenderTask& a, RenderTask& b) {
    const Shader* sa = a.material->shader();
    const Shader* sb = b.material->shader();

    if(sa->layer >= SHADER_LAYER_ALPHA && sb->layer >= SHADER_LAYER_ALPHA) {
      vec3 camPos = cam.transfrom().position();
      return camPos.distance2(a.transform->position()) < camPos.distance2(b.transform->position());
    } else {
      return sa->order() < sb->order();
    }
  });
}
