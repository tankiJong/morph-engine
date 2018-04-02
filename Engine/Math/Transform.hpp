#pragma once
#include "Engine/Core/common.hpp"
#include "Engine/Math/Primitives/vec3.hpp"

class mat44;
struct transform_t {
  vec3 position;
  Eular eular;
  vec3 scale;

  transform_t();

  /* provide an SRT matrix
   * The rotation is ZXY rotation
   */
  mat44 localToWorld() const;
  mat44 worldToLocal() const;
  void set(const mat44& transform);

  inline void translate(const vec3& offset) { position += offset; };
  inline void rotate(float x, float y, float z) { eular += vec3(x, y, z); };
  inline void rotate(const Eular& e) { eular += e; };

  static const transform_t IDENTITY;
};

class Transform {
public:
  mat44 localToWorld() const;
  mat44 worldToLocal() const;

  // mutator
  void localRotate(const Eular& eular);
  void localTranslate(const vec3& offset);
  void setlocalTransform(const mat44& transform);
  

  // accessor
  inline const vec3& localPosition() const { return mLocalTransform.position; };
  inline vec3& localPosition() { return mLocalTransform.position; };

  inline const Eular& localEular() const { return mLocalTransform.eular; };
  inline Eular& localEular() { return mLocalTransform.eular; };

  inline const vec3& localScale() const { return mLocalTransform.scale; };
  inline vec3& localScale() { return mLocalTransform.scale; };

private:
  transform_t mLocalTransform;
};
