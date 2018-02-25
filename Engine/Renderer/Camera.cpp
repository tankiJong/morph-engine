#include "Camera.hpp"
#include "Engine/Debug/ErrorWarningAssert.hpp"
#include "FrameBuffer.hpp"
#include "Engine/Math/Curves.hpp"

Camera::Camera() {
    mFrameBuffer = new FrameBuffer();
  }

Camera::~Camera() {
  delete mFrameBuffer;
  mFrameBuffer = nullptr;
}

void Camera::setColorTarget(Texture* colorTarget) {
  mFrameBuffer->setColorTarget(colorTarget);
}
void Camera::setDepthStencilTarget(Texture* depthTarget) {
  mFrameBuffer->setDepthStencilTarget(depthTarget);
}

void Camera::lookAt(const vec3& position, const vec3& target, const vec3& up) {
  mat44 t = mat44::makeTranslation(position);

  vec3 forward = (target - position).normalized();

  vec3 right = up.cross(forward).normalized();

  vec3 newUp = forward.cross(right);

  mat44 r(vec4(right, 0), 
          vec4(newUp, 0), 
          vec4(forward, 0));

  mCameraMatrix = t * r;

  mViewMatrix = r.transpose() * mat44::makeTranslation((-1.f*position));
}
void Camera::setProjection(const mat44& proj) {
  mProjMatrix = proj;
}

void Camera::setProjectionOrtho(float width, float height, float near, float far) {
  mProjMatrix = mat44::makeOrtho(width, height, near, far);
}

void Camera::finalize() {
  mFrameBuffer->finalize();
}

uint Camera::getFrameBufferHandle() {
  return mFrameBuffer->mHandle;
}

vec3 Camera::screenToWorld(uvec2 pixel, float distanceFromCamera) {
  vec2 s = vec2(pixel);
  float w = (float)mFrameBuffer->width();
  float h = (float)mFrameBuffer->height();

  vec2 ndcXY = rangeMap(s, vec2(0, h), vec2(w, 0), -vec2::one, vec2::one);

  vec4 clip = mProjMatrix * vec4(0, 0, distanceFromCamera, 1);
  vec3 ndc = clip.xyz() / clip.w;
  ndc = vec3(ndcXY, ndc.z);

  clip = vec4(ndc, 1);

  mat44 vp = mProjMatrix * mViewMatrix;
  DEBUGBREAK;
  // unfinished
  mat44 vpInverse = vp.inverse();

  vec4 world = vpInverse * clip;

  return world.xyz() / world.w;
}

uvec2 Camera::worldToScreen(vec3 position) {
  mat44 vp = mProjMatrix * mViewMatrix;
  vec4 clip = vp * vec4(position, 1);
  vec3 ndc = clip.xyz() / clip.w;
  float w = (float)mFrameBuffer->width();
  float h = (float)mFrameBuffer->height();
  vec2 s = rangeMap(ndc.xy(), -vec2::one, vec2::one, { 0,h }, { w,0 });
  return uvec2(s);
}