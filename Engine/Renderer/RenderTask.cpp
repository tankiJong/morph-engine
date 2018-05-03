#include "RenderTask.hpp"
#include "Engine/Renderer/Shader/Material.hpp"
#include "Engine/Math/Transform.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/Shader/ShaderPass.hpp"

void RenderTask::sort(const Camera& cam, span<RenderTask> tasks) {
  std::sort(tasks.begin(), tasks.end(), [&cam](RenderTask& a, RenderTask& b) {

    if(a.layer >= SHADER_LAYER_ALPHA && b.layer >= SHADER_LAYER_ALPHA) {
      vec3 camPos = cam.transfrom().position();
      return camPos.distance2(a.transform->position()) < camPos.distance2(b.transform->position());
    } else {
      return ShaderPass::order(a.layer, a.queue) < ShaderPass::order(b.layer, b.queue);
    }
  });
}
