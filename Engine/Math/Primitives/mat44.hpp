#pragma once
#include "Engine//Math/Primitives/vec2.hpp"
#include "Engine//Math/Primitives/vec4.hpp"
class mat44 {
public:
  union {
    struct {
      float
        ix, iy, iz, iw,
        jx, jy, jz, jw,
        kx, ky, kz, kw,
        tx, ty, tz, tw;
    };
    struct {
      vec4 i,j,k,t;
    };
    float data[16];
  };

public:
  // default-construct to Identity matrix (via variable initialization)
  mat44() {
    ix = 1, iy = 0, iz = 0, iw = 0,
    jx = 0, jy = 1, jz = 0, jw = 0,
    kx = 0, ky = 0, kz = 1, kw = 0,
    tx = 0, ty = 0, tz = 0, tw = 1;
  } 
  
  mat44(float ix, float jx, float kx, float tx,
        float iy, float jy, float ky, float ty,
        float iz, float jz, float kz, float tz,
        float iw, float jw, float kw, float tw);
  explicit mat44(const float* sixteenValuesBasisMajor); // float[16] array in order Ix, Iy...
  explicit mat44(const vec2& iBasis, const vec2& jBasis, const vec2& translation = vec2::zero);
  explicit mat44(const vec4& right, const vec4& up, const vec4& forward, const vec4& w = vec4(0,0,0,1));

  // Accessors
  vec2 translateTo(const vec2& position2D) const; // Written assuming z=0, w=1
  vec2 translate(const vec2& displacement2D) const; // Written assuming z=0, w=0

                                                                  // Mutators
  void setIdentity();
  void set(const float* sixteenValuesBasisMajor); // float[16] array in order Ix, Iy...
  mat44& append(const mat44& matrixToAppend); // a.k.a. Concatenate (right-multiply), this * matrixToAppend
  mat44& prepend(const mat44& matrixToPrepend); // matrixToPrepend * this 
  mat44& rotate2D(float rotationDegreesAboutZ); // 
  mat44& translate2D(const vec2& translation);
  mat44& scale2D(float scaleXY);
  mat44& scale2D(float scaleX, float scaleY);
  vec4 x() const;
  vec4 y() const;
  vec4 z() const;
  vec4 w() const;

  mat44 operator*(const mat44& rhs) const;
  vec4 operator*(const vec4& rhs) const;
  bool operator==(const mat44& rhs) const;

  mat44 transpose() const;
  mat44 inverse() const;
  Euler eular() const;
  vec3  scale() const;
  // Producers
  static mat44 makeRotation(const Euler& ea);
  static mat44 makeRotation(float x, float y, float z);
  static mat44 makeRotation2D(float rotationDegreesAboutZ);
  static mat44 makeTranslation2D(const vec2& translation);
  static mat44 makeTranslation(const vec3& translation);
  static mat44 makeScale2D(float scaleXY);
  static mat44 makeScale2D(float scaleX, float scaleY);
  static mat44 makeScale(float x, float y, float z);
  static mat44 makeOrtho2D(const vec2& bottomLeft, const vec2& topRight);
  static mat44 makeOrtho(float l, float r, float b, float t, float nz, float fz);
  static mat44 makeOrtho(float width, float height, float near, float far);
  static mat44 makePerspective(float fovDeg, float aspect, float nz, float fz);
  static mat44 makePerspective(float fovDeg, float width, float height, float nz, float fz);
  static mat44 lookAt(const vec3& position, const vec3& target, const vec3& up = vec3::up);
public:
  // x,i
  static const mat44 right;
  
  // y,j
  static const mat44 up;

  // z,k
  static const mat44 forward;

  static const mat44 identity;
};

