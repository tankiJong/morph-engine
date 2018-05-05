#include "Camera.hpp"
#include "Engine/Debug/ErrorWarningAssert.hpp"
#include "FrameBuffer.hpp"
#include "Engine/Math/Curves.hpp"
#include "RenderTarget.hpp"
Camera::Camera() {
  mFrameBuffer = new FrameBuffer();
  mViewMatrix = mTransform.worldToLocal();
}

Camera::~Camera() {
  delete mFrameBuffer;
  mFrameBuffer = nullptr;
}

void Camera::setColorTarget(RenderTarget* colorTarget) {
  mFrameBuffer->setColorTarget(colorTarget);
}
void Camera::setDepthStencilTarget(RenderTarget* depthTarget) {
  mFrameBuffer->setDepthStencilTarget(depthTarget);
}

void Camera::handlePrePass(delegate<void(const Camera& cam)> cb) {
  mPrePassHandler.push_back(std::move(cb));
}

void Camera::prepass() const {
  for(auto& handle: mPrePassHandler) {
    handle(*this);
  }
}

void Camera::lookAt(const vec3& position, const vec3& target, const vec3& up) {

  mViewMatrix = mat44::lookAt(position, target, up);

  mTransform.setlocalTransform(mViewMatrix.inverse());
}
void Camera::setProjection(const mat44& proj) {
  mProjMatrix = proj;
}

void Camera::setProjectionOrtho(float width, float height, float near, float far) {
  mProjMatrix = mat44::makeOrtho(width, height, near, far);
}

void Camera::setProjectionPrespective(float fovDeg, float width, float height, float near, float far) {
  mProjMatrix = mat44::makePerspective(fovDeg, width, height, near, far);
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

// in screen space, top-right is (0,0), bottom-right is (w,h)
uvec2 Camera::worldToScreen(vec3 position) {
  mat44 vp = mProjMatrix * mViewMatrix;
  vec4 clip = vp * vec4(position, 1);
  vec3 ndc = clip.xyz() / clip.w;
  float w = (float)mFrameBuffer->width();
  float h = (float)mFrameBuffer->height();
  vec2 s = rangeMap(ndc.xy(), -vec2::one, vec2::one, { 0,h }, { w,0 });
  return uvec2(s);
}

void Camera::rotate(const Euler& eular) {
  mTransform.localRotate(eular);

  mViewMatrix = mTransform.worldToLocal();
}

void Camera::translate(const vec3& translation) {
  mTransform.localTranslate(translation);

  mViewMatrix = mTransform.worldToLocal();
}
