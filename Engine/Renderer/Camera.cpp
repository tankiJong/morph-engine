#include "Camera.hpp"
#include "Engine/Debug/ErrorWarningAssert.hpp"
#include "FrameBuffer.hpp"

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
