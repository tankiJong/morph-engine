#pragma once
#include "Engine/Core/common.hpp"
#include "Engine/Math/Primitives/mat44.hpp"
#include "Engine/Math/Primitives/vec3.hpp"
#include "Engine/Math/Transform.hpp"
#include "Engine/Core/Delegate.hpp"
#include "Engine/Graphics/RHI/RHI.hpp"
#include "Engine/Math/Primitives/vec4.hpp"

class uvec2;
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
  void setProjectionOrtho(float width, float height, float nearZ, float farZ);
  void setProjectionPrespective(float fovDeg, float width, float height, float nearZ, float farZ);

  inline float width() const { return mWidth; }
  inline float height() const { return mHeight; }

  vec3 screenToWorld(uvec2 pixel, float distanceFromCamera);
  uvec2 worldToScreen(vec3 position);

  inline vec3 right() const { return (mCoordinateTransform.inverse() * mTransform.localToWorld() * vec4(vec3::right, 0)).xyz(); };
  inline vec3 up() const { return (mCoordinateTransform.inverse() * mTransform.localToWorld() * vec4(vec3::up, 0)).xyz(); };
  inline vec3 forward() const { return (mCoordinateTransform.inverse() * mTransform.localToWorld() * vec4(vec3::forward, 0)).xyz(); };

  void rotate(const Euler& euler);
  void translate(const vec3& translation);

  inline const Transform& transform() const { return mTransform; }
  inline Transform& transform() { mIsDirty = true; return mTransform; };
  inline void setFlag(uint flag) { mFlag = mFlag | flag; }
  inline bool queryFlag(eCamreraFlag flag) const { return flag & mFlag; }

  // an optional mat44 to transform from game world space to view space, useful for game to have different coordinate system
  void setCoordinateTransform(const mat44& t);

  camera_t ubo() const;
  inline mat44 view() const { return mCoordinateTransform * mTransform.worldToLocal(); }
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
  mat44 mCoordinateTransform;
  bool mIsDirty;
  uint mFlag = 0;
};
