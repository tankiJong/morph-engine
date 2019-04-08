#include "Transform.hpp"
#include "Primitives/mat44.hpp"

const transform_t transform_t::IDENTITY = transform_t();

transform_t::transform_t()
  : position(0.f)
  , euler(0.f)
  , scale(1.f) {}

mat44 transform_t::localToWorld(eRotationOrder rotationOrder) const {
  mat44 r = mat44::rotation(euler, rotationOrder);
  mat44 s = mat44::scale(scale.x, scale.y, scale.z);
  mat44 t = mat44::translation(position);

  return t * r * s;
}

mat44 transform_t::worldToLocal(eRotationOrder rotationOrder) const {
  mat44 r = mat44::rotation(euler, rotationOrder).transpose();
  mat44 s = mat44::scale(1.f / scale.x, 1.f / scale.y, 1.f / scale.z);
  mat44 t = mat44::translation(-position);

  return s * r * t;
}

void transform_t::set(const mat44& transform, eRotationOrder rotationOrder) {
  position = transform.t.xyz();
  euler = transform.euler(rotationOrder);
  scale = transform.scale();
}

mat44 Transform::localToWorld() const {
  return worldMat() * mLocalTransform.localToWorld(mRotationOrder);
}

mat44 Transform::worldToLocal() const {
  return mLocalTransform.worldToLocal(mRotationOrder) * localMat();
}

void Transform::localRotate(const Euler& euler) {
  mLocalTransform.rotate(euler);
}

void Transform::localTranslate(const vec3& offset) {
  mLocalTransform.translate(offset);
}

void Transform::setlocalTransform(const mat44& transform) {
  mLocalTransform.set(transform, mRotationOrder);
}

void Transform::setWorldTransform(const mat44& transform) {
  mat44 mat =
    (mParent == nullptr)
    ? transform
    : parent()->worldToLocal() * transform;
  setlocalTransform(mat);
}

void Transform::setRotationOrder(eRotationOrder order) {
  mRotationOrder = order;
}

vec3 Transform::forward() const {
  return (localToWorld() * vec4(vec3::forward, 0)).xyz();
}

vec3 Transform::up() const {
  return (localToWorld() * vec4(vec3::up, 0)).xyz();
}

vec3 Transform::right() const {
  return (localToWorld() * vec4(vec3::right, 0)).xyz();
}

vec3 Transform::transform(const vec3& pointOrDisp, bool isDisp) const {
  vec4 from(pointOrDisp, isDisp ? 0.f : 1.f);
  vec4 result = localToWorld() * from;
  return result.xyz();
}

mat44 Transform::lookAt(const vec3& position, const vec3& target) {
  vec3 _forward = (target - position).normalized();

  vec3 _right = vec3::up.cross(_forward).normalized();

  vec3 _up = _forward.cross(_right);

  mat44 r(vec4(_right, 0),
          vec4(_up, 0),
          vec4(_forward, 0));

  return mat44::translation(position) * r;
}

mat44 Transform::worldMat() const {
  return (mParent == nullptr ? mat44::identity : mParent->localToWorld());
}

mat44 Transform::localMat() const {
  return (mParent == nullptr ? mat44::identity : mParent->worldToLocal());
}
