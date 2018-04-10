#include "Transform.hpp"
#include "Primitives/mat44.hpp"

const transform_t transform_t::IDENTITY = transform_t();

transform_t::transform_t()
  : position(0.f)
  , eular(0.f)
  , scale(1.f) {}

mat44 transform_t::localToWorld() const {
  mat44 r = mat44::makeRotation(eular);
  mat44 s = mat44::makeScale(scale.x, scale.y, scale.z);
  mat44 t = mat44::makeTranslation(position);

  return t * r * s;
}

mat44 transform_t::worldToLocal() const {
  mat44 r = mat44::makeRotation(eular).transpose();
  mat44 s = mat44::makeScale(1.f / scale.x, 1.f / scale.y, 1.f / scale.z);
  mat44 t = mat44::makeTranslation(-position);

  return s * r * t;
}

void transform_t::set(const mat44& transform) {
  position = transform.t.xyz();
  eular = transform.eular();
  scale = transform.scale();
}

mat44 Transform::localToWorld() const {
  return mLocalTransform.localToWorld();
}

mat44 Transform::worldToLocal() const {
  return mLocalTransform.worldToLocal();
}

void Transform::localRotate(const Euler& eular) {
  mLocalTransform.rotate(eular);
}

void Transform::localTranslate(const vec3& offset) {
  mLocalTransform.translate(offset);
}

void Transform::setlocalTransform(const mat44& transform) {
  mLocalTransform.set(transform);
}
