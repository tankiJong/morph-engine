#include "Matrix44.hpp"
#include "MathUtils.hpp"

struct Vector4 {
public:
  float x = 0, y = 0, z = 0, w = 0;
  Vector4() {};
  Vector4(float x, float y, float z, float w) 
    : x(x)
    , y(y)
    , z(z)
    , w(w) {}
  inline float dot(const Vector4& v) const {
    return x*v.x + y*v.y + z*v.z + w*v.w;
  }
};

Matrix44::Matrix44(float ix, float jx, float kx, float tx, 
                   float iy, float jy, float ky, float ty, 
                   float iz, float jz, float kz, float tz, 
                   float iw, float jw, float kw, float tw)
  : ix(ix), iy(iy), iz(iz), iw(iw),
    jx(jx), jy(jy), jz(jz), jw(jw),
    kx(kx), ky(ky), kz(kz), kw(kw),
    tx(tx), ty(ty), tz(tz), tw(tw) {
}

Matrix44::Matrix44(const float* sixteenValuesBasisMajor) {
  set(sixteenValuesBasisMajor);
}
Matrix44::Matrix44(const Vector2& iBasis, const Vector2& jBasis, const Vector2& translation)
  : ix(iBasis.x), iy(iBasis.y), iz(0), iw(0),
    jx(jBasis.x), jy(jBasis.y), jz(0), jw(0),
    kx(0), ky(0), kz(1), kw(0),
    tx(translation.x), ty(translation.y), tz(0), tw(1) {
  
}

Vector2 Matrix44::translateTo(const Vector2& position2D) const {
  float x = ix*position2D.x + jx*position2D.y + tx;
  float y = iy*position2D.x + jy*position2D.y + ty;
  return { x,y };
}

Vector2 Matrix44::translate(const Vector2& displacement2D) const {
  float x = ix*displacement2D.x + jx*displacement2D.y;
  float y = iy*displacement2D.x + jy*displacement2D.y;
  return { x,y };
}

void Matrix44::setIdentity() {
  ix = 1, iy = 0, iz = 0, iw = 0,
  jx = 0, jy = 1, jz = 0, jw = 0,
  kx = 0, ky = 0, kz = 1, kw = 0,
  tx = 0, ty = 0, tz = 0, tw = 1;
}
void Matrix44::set(const float* sixteenValuesBasisMajor) {
  memcpy(this, sixteenValuesBasisMajor, sizeof(float) * 16);
}

Matrix44& Matrix44::prepend(const Matrix44& matrixToPrepend) {
  Vector4
    i(ix, iy, iz, iw),
    j(jx, jy, jz, jw),
    k(kx, ky, kz, kw),
    t(tx, ty, tz, tw);

  Vector4
    px(matrixToPrepend.ix, matrixToPrepend.jx, matrixToPrepend.kx, matrixToPrepend.tx),
    py(matrixToPrepend.iy, matrixToPrepend.jy, matrixToPrepend.ky, matrixToPrepend.ty),
    pz(matrixToPrepend.iz, matrixToPrepend.jz, matrixToPrepend.kz, matrixToPrepend.tz),
    pw(matrixToPrepend.iw, matrixToPrepend.jw, matrixToPrepend.kw, matrixToPrepend.tw);

  ix = i.dot(px); jx = j.dot(px); kx = k.dot(px); tx = t.dot(px);
  iy = i.dot(py); jy = j.dot(py); ky = k.dot(py); ty = t.dot(py);
  iz = i.dot(pz); jz = j.dot(pz); kz = k.dot(pz); tz = t.dot(pz);
  iw = i.dot(pw); jw = j.dot(pw); kw = k.dot(pw); tw = t.dot(pw);

  return *this;
}

Matrix44& Matrix44::append(const Matrix44& matrixToAppend) {
  Matrix44 temp = matrixToAppend;
  temp.prepend(*this);
  *this = temp;
  return *this;
}

Matrix44& Matrix44::rotate2D(float rotationDegreesAboutZ) {
  append(makeRotation2D(rotationDegreesAboutZ));
  return *this;
}

Matrix44& Matrix44::translate2D(const Vector2& translation) {
  append(makeTranslation2D(translation));
  return *this;
}

Matrix44& Matrix44::scale2D(float scaleXY) {
  append(makeScale2D(scaleXY));
  return *this;
}

Matrix44& Matrix44::scale2D(float scaleX, float scaleY) {
  append(makeScale2D(scaleX, scaleY));
  return *this;
}

Matrix44 Matrix44::makeRotation2D(float rotationDegreesAboutZ) {
  float cosdeg = cosDegrees(rotationDegreesAboutZ), sindeg = sinDegrees(rotationDegreesAboutZ);
  return {
    cosdeg, -sindeg, 0, 0,
    sindeg,  cosdeg, 0, 0,
         0,       0, 1, 0,
         0,       0, 0, 1
  };
}

Matrix44 Matrix44::makeTranslation2D(const Vector2& translation) {
  return {
    1, 0, 0, translation.x,
    0, 1, 0, translation.y,
    0, 0, 1, 0,
    0, 0, 0, 1
  };
}
Matrix44 Matrix44::makeScale2D(float scaleXY) {
  return makeScale2D(scaleXY, scaleXY);
}
Matrix44 Matrix44::makeScale2D(float scaleX, float scaleY) {
  return {
    scaleX,       0, 0, 0,
         0,  scaleY, 0, 0,
         0,       0, 1, 0,
         0,       0, 0, 1
  };
}

Matrix44 Matrix44::makeOrtho2D(const Vector2& bottomLeft, const Vector2& topRight) {
  Vector2 size = topRight - bottomLeft;
  Vector2 center = (bottomLeft + topRight) * -.5f;

  Matrix44 origin;

  origin
    .scale2D(2.f / size.x, 2.f / size.y)
    .translate2D(center);

  origin.kz = -2.f; // assume transform z from (0,1) to (1, -1)
  origin.tz = -1.f;
  origin.tw = 1.f; // keep w good
  return origin;
}
