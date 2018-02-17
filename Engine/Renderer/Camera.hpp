#pragma once
#include "Engine/Core/common.hpp"
#include "Texture.hpp"
#include "Engine/Math/Primitives/Mat44.hpp"
#include "Engine/Math/Primitives/vec3.hpp"

class Camera {
  friend class Renderer;
public:
  Camera();
  ~Camera();

  // will be implemented later
  void setColorTarget(Texture* colorTarget);
  void setDepthStencilTarget(Texture* depthTarget);

  // model setters
  void lookAt(const vec3& position, const vec3& target, const vec3& up = vec3::up);

  // projection settings
  void setProjection(const mat44& proj);
  void setProjectionOrtho(float width, float height, float near, float far);

  void finalize();

  uint getFrameBufferHandle();

  vec3 screenToWorld(uvec2 pixel, float distanceFromCamera);
  uvec2 worldToScreen(vec3 position);
public:
  // default all to identiy
  mat44 mCameraMatrix;  // where is the camera?
  mat44 mViewMatrix;    // inverse of camera (used for shader)
  mat44 mProjMatrix;    // projection
  owner<FrameBuffer*> mFrameBuffer;
                          // FrameBuffer m_output; // eventually
};
