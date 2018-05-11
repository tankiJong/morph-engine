#pragma once
#include "Engine/Core/common.hpp"
#include "Texture.hpp"
#include "Engine/Math/Primitives/mat44.hpp"
#include "Engine/Math/Primitives/vec3.hpp"
#include "FrameBuffer.hpp"
#include "Engine/Math/Transform.hpp"
#include "Engine/Core/Delegate.hpp"

class aabb2;
class RenderTarget;
enum eCamreraFlag {
  CAM_CLEAR_DEPTH = 0b10,
  CAM_CLEAR_COLOR = 0b01,
  CAM_EFFECT_BLOOM = 0b100,
};

class Camera {
public:
  Camera();
  ~Camera();

  int sort;
  // will be implemented later
  void setColorTarget(RenderTarget* colorTarget, uint slot = 0);
  void setDepthStencilTarget(RenderTarget* depthTarget);

  inline RenderTarget* depthTarget() { return (RenderTarget*)mFrameBuffer->mDepthTarget; }
  inline RenderTarget* colorTarget(uint index = 0) { return (RenderTarget*)mFrameBuffer->mColorTarget[index]; }
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

  void rotate(const Euler& euler);
  void translate(const vec3& translation);

  inline const Transform& transfrom() const { return mTransform; }
  inline Transform& transfrom() { mIsDirty = true; return mTransform; };
  inline void setFlag(uint flag) { mFlag = mFlag | flag; }
  inline bool queryFlag(eCamreraFlag flag) const { return flag & mFlag; }

  camera_t ubo() const;
  inline FrameBuffer* fbo() const { return mFrameBuffer; }
  inline mat44 view() const { return mTransform.worldToLocal(); }
  inline mat44 projection() const { return mProjMatrix; };
protected:
  Transform mTransform;
  // default all to identiy
  std::vector<delegate<void(const Camera& cam)>> mPrePassHandler;
  union {
    struct {
      mat44 mProjMatrix;    // projection
      mutable mat44 mViewMatrix;    // inverse of camera (used for shader)
    };
    camera_t cameraBlock;
  };
  owner<FrameBuffer*> mFrameBuffer;
  bool mIsDirty;
  uint mFlag = 0;
};
