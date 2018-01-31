#include "mat4.hpp"
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

mat4::mat4(float ix, float jx, float kx, float tx, 
                   float iy, float jy, float ky, float ty, 
                   float iz, float jz, float kz, float tz, 
                   float iw, float jw, float kw, float tw)
  : ix(ix), iy(iy), iz(iz), iw(iw),
    jx(jx), jy(jy), jz(jz), jw(jw),
    kx(kx), ky(ky), kz(kz), kw(kw),
    tx(tx), ty(ty), tz(tz), tw(tw) {
}

mat4::mat4(const float* sixteenValuesBasisMajor) {
  set(sixteenValuesBasisMajor);
}
mat4::mat4(const Vector2& iBasis, const Vector2& jBasis, const Vector2& translation)
  : ix(iBasis.x), iy(iBasis.y), iz(0), iw(0),
    jx(jBasis.x), jy(jBasis.y), jz(0), jw(0),
    kx(0), ky(0), kz(1), kw(0),
    tx(translation.x), ty(translation.y), tz(0), tw(1) {
  
}

Vector2 mat4::translateTo(const Vector2& position2D) const {
  float x = ix*position2D.x + jx*position2D.y + tx;
  float y = iy*position2D.x + jy*position2D.y + ty;
  return { x,y };
}

Vector2 mat4::translate(const Vector2& displacement2D) const {
  float x = ix*displacement2D.x + jx*displacement2D.y;
  float y = iy*displacement2D.x + jy*displacement2D.y;
  return { x,y };
}

void mat4::setIdentity() {
  ix = 1, iy = 0, iz = 0, iw = 0,
  jx = 0, jy = 1, jz = 0, jw = 0,
  kx = 0, ky = 0, kz = 1, kw = 0,
  tx = 0, ty = 0, tz = 0, tw = 1;
}
void mat4::set(const float* sixteenValuesBasisMajor) {
  memcpy(this, sixteenValuesBasisMajor, sizeof(float) * 16);
}

mat4& mat4::prepend(const mat4& matrixToPrepend) {
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

mat4& mat4::append(const mat4& matrixToAppend) {
  mat4 temp = matrixToAppend;
  temp.prepend(*this);
  *this = temp;
  return *this;
}

mat4& mat4::rotate2D(float rotationDegreesAboutZ) {
  append(makeRotation2D(rotationDegreesAboutZ));
  return *this;
}

mat4& mat4::translate2D(const Vector2& translation) {
  append(makeTranslation2D(translation));
  return *this;
}

mat4& mat4::scale2D(float scaleXY) {
  append(makeScale2D(scaleXY));
  return *this;
}

mat4& mat4::scale2D(float scaleX, float scaleY) {
  append(makeScale2D(scaleX, scaleY));
  return *this;
}

mat4 mat4::makeRotation2D(float rotationDegreesAboutZ) {
  float cosdeg = cosDegrees(rotationDegreesAboutZ), sindeg = sinDegrees(rotationDegreesAboutZ);
  return {
    cosdeg, -sindeg, 0, 0,
    sindeg,  cosdeg, 0, 0,
         0,       0, 1, 0,
         0,       0, 0, 1
  };
}

mat4 mat4::makeTranslation2D(const Vector2& translation) {
  return {
    1, 0, 0, translation.x,
    0, 1, 0, translation.y,
    0, 0, 1, 0,
    0, 0, 0, 1
  };
}
mat4 mat4::makeScale2D(float scaleXY) {
  return makeScale2D(scaleXY, scaleXY);
}
mat4 mat4::makeScale2D(float scaleX, float scaleY) {
  return {
    scaleX,       0, 0, 0,
         0,  scaleY, 0, 0,
         0,       0, 1, 0,
         0,       0, 0, 1
  };
}

mat4 mat4::makeOrtho2D(const Vector2& bottomLeft, const Vector2& topRight) {
  return makeOrtho(bottomLeft.x, topRight.x, bottomLeft.y, topRight.y, -1, 1);
}

mat4 mat4::makeOrtho(float l, float r, float b, float t, float nz, float fz) {
  return {
    2/(r-l), 0, 0, 0,
    0, 2/(t-b), 0, 0,
    0, 0, 1/(nz-fz), 0,
    (l+r)/(l-r), (t+b)/(b-t), nz/(nz-fz), 1
  };
}
