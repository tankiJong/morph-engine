#pragma once
#include "Engine/Core/common.hpp"
#include "Engine/Math/Primitives/vec3.hpp"

class mat44;
struct transform_t {
  vec3 position;
  Euler eular;
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
  inline void rotate(const Euler& e) { eular += e; };

  static const transform_t IDENTITY;
};

class Transform {
public:
  mat44 localToWorld() const;
  mat44 worldToLocal() const;

  // mutator
  void localRotate(const Euler& eular);
  void localTranslate(const vec3& offset);
  void setlocalTransform(const mat44& transform);
  

  // accessor
  vec3 forward() const;
  vec3 up() const;
  vec3 right() const;
  
  inline const vec3& localPosition() const { return mLocalTransform.position; }
  inline vec3& localPosition() { return mLocalTransform.position; };

  inline const Euler& localRotation() const { return mLocalTransform.eular; };
  inline Euler& localRotation() { return mLocalTransform.eular; };

  inline const vec3& localScale() const { return mLocalTransform.scale; };
  inline vec3& localScale() { return mLocalTransform.scale; };

  inline const vec3 position() const { return mLocalTransform.position; };
  inline const Euler rotation() const { return mLocalTransform.eular; };
  inline const vec3 scale() const { return mLocalTransform.scale; };\

private:
  transform_t mLocalTransform;
};
