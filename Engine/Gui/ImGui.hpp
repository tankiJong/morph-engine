#pragma once

#include "ThirdParty/imgui/imgui.h"
#include "ThirdParty/imgui/ImGuizmo.h"

class Transform;
class Camera;

namespace ImGui {
  void startup();
  void shutdown();

  void beginFrame();
  void render();

  void gizmos(Camera& cam, Transform& target, ImGuizmo::OPERATION op);
}