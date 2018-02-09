#pragma once
#include "Engine/Core/common.hpp"
#include "Texture.hpp"
#include "Engine/Math/Mat44.hpp"
class vec3;

class Camera {
public:
  Camera() {}
  ~Camera() {}

  // will be implemented later
  void SetColorTarget(Texture* colorTarget) {}
  void SetDepthStencilTarget(Texture* depthTarget) {}

  // model setters
  void LookAt(vec3 position, vec3 target, vec3 up = mat44::up);

  // projection settings
  void SetProjection(mat44 proj);
  void SetProjectionOrtho(float size, float near, float far);

public:
  // default all to identiy
  mat44 m_camera_matrix;  // where is the camera?
  mat44 m_view_matrix;    // inverse of camera (used for shader)
  mat44 m_proj_matrix;    // projection

                          // FrameBuffer m_output; // eventually
};
