#pragma once
#include "Engine/Core/common.hpp"
#include "Engine/Math/Primitives/mat44.hpp"
#include "Engine/Math/Primitives/vec3.hpp"
#include "Engine/Graphics/RHI/FrameBuffer.hpp"
#include "Engine/Math/Transform.hpp"
#include "Engine/Core/Delegate.hpp"

class aabb2;
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

  void handlePrePass(delegate<void(const Camera& cam)> cb);
  void prepass() const;
  // model setters
  void lookAt(const vec3& position, const vec3& target, const vec3& up = vec3::up);

  // projection settings
  void setProjectionOrtho(float width, float height, float near, float far);
  void setProjectionPrespective(float fovDeg, float width, float height, float near, float far);

  inline uint width() const { return mWidth; }
  inline uint height() const { return mHeight; }

  vec3 screenToWorld(uvec2 pixel, float distanceFromCamera);
  uvec2 worldToScreen(vec3 position);

  inline vec3 right() const { return (mTransform.localToWorld() * vec4(vec3::right, 0)).xyz(); };
  inline vec3 up() const { return (mTransform.localToWorld() * vec4(vec3::up, 0)).xyz(); };
  inline vec3 forward() const { return (mTransform.localToWorld() * vec4(vec3::forward, 0)).xyz(); };

  void rotate(const Euler& euler);
  void translate(const vec3& translation);

  inline const Transform& transfrom() const { return mTransform; }
  inline Transform& transfrom() { mIsDirty = true; return mTransform; };
  inline void setFlag(uint flag) { mFlag = mFlag | flag; }
  inline bool queryFlag(eCamreraFlag flag) const { return flag & mFlag; }

  camera_t ubo() const;
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
  float mWidth = 0, mHeight = 0, mDepth = 0;
  bool mIsDirty;
  uint mFlag = 0;
};
