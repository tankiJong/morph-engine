#pragma once
#include "Engine/Core/common.hpp"
#include "Texture.hpp"
#include "Engine/Math/Primitives/mat44.hpp"
#include "Engine/Math/Primitives/vec3.hpp"
#include "FrameBuffer.hpp"
#include "Engine/Math/Transform.hpp"
#include "Engine/Core/Delegate.hpp"

class aabb2;

enum eCamreraFlag {
  CAM_CLEAR_DEPTH = 0b10,
  CAM_CLEAR_COLOR = 0b01,
};

class Camera {
  friend class Renderer;
public:
  Camera();
  ~Camera();

  int sort;
  // will be implemented later
  void setColorTarget(Texture* colorTarget);
  void setDepthStencilTarget(Texture* depthTarget);

  void handlePrePass(delegate<void(const Camera& cam)> cb);
  void prepass() const;
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

  inline const vec3 right() const { return (mTransform.localToWorld() * vec4(vec3::right, 0)).xyz(); };
  inline const vec3 up() const { return (mTransform.localToWorld() * vec4(vec3::up, 0)).xyz(); };
  inline const vec3 forward() const { return (mTransform.localToWorld() * vec4(vec3::forward, 0)).xyz(); };
  inline uint width() const { return mFrameBuffer->width(); }
  inline uint height() const { return mFrameBuffer->height(); }

  void rotate(const Euler& eular);
  void translate(const vec3& translation);

  inline const Transform& transfrom() const { return mTransform; }
  inline Transform& transfrom() { return mTransform; }
  inline void setFlag(uint flag) { mFlag = mFlag | flag; }
  inline bool queryFlag(eCamreraFlag flag) const { return flag & mFlag; }
protected:
  Transform mTransform;
  // default all to identiy
  std::vector<delegate<void(const Camera& cam)>> mPrePassHandler;
  union {
    struct {
      mat44 mProjMatrix;    // projection
      mat44 mViewMatrix;    // inverse of camera (used for shader)
    };
    camera_t cameraBlock;
  };
  owner<FrameBuffer*> mFrameBuffer;
  uint mFlag = 0;
};
