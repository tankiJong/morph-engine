#pragma once
#include "Engine/Math/Vector2.hpp"


class Matrix44 {
public:
  // default-construct to Identity matrix (via variable initialization)
  Matrix44() {
    ix = 1, iy = 0, iz = 0, iw = 0,
    jx = 0, jy = 1, jz = 0, jw = 0,
    kx = 0, ky = 0, kz = 1, kw = 0,
    tx = 0, ty = 0, tz = 0, tw = 1;
  } 
  
  Matrix44(float ix, float iy, float iz, float iw,
           float jx, float jy, float jz, float jw,
           float kx, float ky, float kz, float kw,
           float tx, float ty, float tz, float tw);
  explicit Matrix44(const float* sixteenValuesBasisMajor); // float[16] array in order Ix, Iy...
  explicit Matrix44(const Vector2& iBasis, const Vector2& jBasis, const Vector2& translation = Vector2::zero);

  // Accessors
  Vector2 translateTo(const Vector2& position2D) const; // Written assuming z=0, w=1
  Vector2 translate(const Vector2& displacement2D) const; // Written assuming z=0, w=0

                                                                  // Mutators
  void setIdentity();
  void set(const float* sixteenValuesBasisMajor); // float[16] array in order Ix, Iy...
  Matrix44& append(const Matrix44& matrixToAppend); // a.k.a. Concatenate (right-multiply), this * matrixToAppend
  Matrix44& prepend(const Matrix44& matrixToPrepend); // matrixToPrepend * this 
  Matrix44& rotate2D(float rotationDegreesAboutZ); // 
  Matrix44& translate2D(const Vector2& translation);
  Matrix44& scale2D(float scaleXY);
  Matrix44& scale2D(float scaleX, float scaleY);

  // Producers
  static Matrix44 makeRotation2D(float rotationDegreesAboutZ);
  static Matrix44 makeTranslation2D(const Vector2& translation);
  static Matrix44 makeScale2D(float scaleXY);
  static Matrix44 makeScale2D(float scaleX, float scaleY);
  static Matrix44 makeOrtho2D(const Vector2& bottomLeft, const Vector2& topRight);
protected:
  float
    ix, iy, iz, iw, 
    jx, jy, jz, jw, 
    kx, ky, kz, kw, 
    tx, ty, tz, tw;
};
