#pragma once
#include "Engine/Core/common.hpp"
#include "Texture.hpp"
#include "Engine/Math/Primitives/Mat44.hpp"
#include "Engine/Math/Primitives/vec3.hpp"
#include "FrameBuffer.hpp"

class aabb2;

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
  void setProjectionPrespective(float fovDeg, float width, float height, float near, float far);

  void finalize();

  uint getFrameBufferHandle();

  vec3 screenToWorld(uvec2 pixel, float distanceFromCamera);
  uvec2 worldToScreen(vec3 position);

  inline vec3 right() const { return mCameraMatrix.i.xyz(); };
  vec3 up() const { return mCameraMatrix.j.xyz(); };
  vec3 forward() const { return mCameraMatrix.k.xyz(); };
  inline uint width() const { return mFrameBuffer->width(); }
  inline uint height() const { return mFrameBuffer->height(); }
public:
  // default all to identiy
  mat44 mCameraMatrix;  // where is the camera?
  union {
    struct {
      mat44 mProjMatrix;    // projection
      mat44 mViewMatrix;    // inverse of camera (used for shader)
    };
    camera_t cameraBlock;
  };
  owner<FrameBuffer*> mFrameBuffer;
                          // FrameBuffer m_output; // eventually
};
