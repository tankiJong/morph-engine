#include "mat44.hpp"
#include "MathUtils.hpp"


const mat44 mat44::up = {1,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,1};

static const mat44 up;

// z,k
static const mat44 forward;

mat44::mat44(float ix, float jx, float kx, float tx, 
                   float iy, float jy, float ky, float ty, 
                   float iz, float jz, float kz, float tz, 
                   float iw, float jw, float kw, float tw)
  : ix(ix), iy(iy), iz(iz), iw(iw),
    jx(jx), jy(jy), jz(jz), jw(jw),
    kx(kx), ky(ky), kz(kz), kw(kw),
    tx(tx), ty(ty), tz(tz), tw(tw) {
}

mat44::mat44(const float* sixteenValuesBasisMajor) {
  set(sixteenValuesBasisMajor);
}
mat44::mat44(const vec2& iBasis, const vec2& jBasis, const vec2& translation)
  : ix(iBasis.x), iy(iBasis.y), iz(0), iw(0),
    jx(jBasis.x), jy(jBasis.y), jz(0), jw(0),
    kx(0), ky(0), kz(1), kw(0),
    tx(translation.x), ty(translation.y), tz(0), tw(1) {
  
}

mat44::mat44(const vec4& x, const vec4& y, const vec4& z, const vec4& w): x(x), y(y), z(z), w(w) {}

vec2 mat44::translateTo(const vec2& position2D) const {
  float x = ix*position2D.x + jx*position2D.y + tx;
  float y = iy*position2D.x + jy*position2D.y + ty;
  return { x,y };
}

vec2 mat44::translate(const vec2& displacement2D) const {
  float x = ix*displacement2D.x + jx*displacement2D.y;
  float y = iy*displacement2D.x + jy*displacement2D.y;
  return { x,y };
}

void mat44::setIdentity() {
  ix = 1, iy = 0, iz = 0, iw = 0,
  jx = 0, jy = 1, jz = 0, jw = 0,
  kx = 0, ky = 0, kz = 1, kw = 0,
  tx = 0, ty = 0, tz = 0, tw = 1;
}
void mat44::set(const float* sixteenValuesBasisMajor) {
  memcpy(this, sixteenValuesBasisMajor, sizeof(float) * 16);
}

mat44& mat44::prepend(const mat44& matrixToPrepend) {
  vec4
    i(ix, iy, iz, iw),
    j(jx, jy, jz, jw),
    k(kx, ky, kz, kw),
    t(tx, ty, tz, tw);

  vec4
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

mat44& mat44::append(const mat44& matrixToAppend) {
  mat44 temp = matrixToAppend;
  temp.prepend(*this);
  *this = temp;
  return *this;
}

mat44& mat44::rotate2D(float rotationDegreesAboutZ) {
  append(makeRotation2D(rotationDegreesAboutZ));
  return *this;
}

mat44& mat44::translate2D(const vec2& translation) {
  append(makeTranslation2D(translation));
  return *this;
}

mat44& mat44::scale2D(float scaleXY) {
  append(makeScale2D(scaleXY));
  return *this;
}

mat44& mat44::scale2D(float scaleX, float scaleY) {
  append(makeScale2D(scaleX, scaleY));
  return *this;
}

mat44 mat44::makeRotation2D(float rotationDegreesAboutZ) {
  float cosdeg = cosDegrees(rotationDegreesAboutZ), sindeg = sinDegrees(rotationDegreesAboutZ);
  return {
    cosdeg, -sindeg, 0, 0,
    sindeg,  cosdeg, 0, 0,
         0,       0, 1, 0,
         0,       0, 0, 1
  };
}

mat44 mat44::makeTranslation2D(const vec2& translation) {
  return {
    1, 0, 0, translation.x,
    0, 1, 0, translation.y,
    0, 0, 1, 0,
    0, 0, 0, 1
  };
}
mat44 mat44::makeScale2D(float scaleXY) {
  return makeScale2D(scaleXY, scaleXY);
}
mat44 mat44::makeScale2D(float scaleX, float scaleY) {
  return {
    scaleX,       0, 0, 0,
         0,  scaleY, 0, 0,
         0,       0, 1, 0,
         0,       0, 0, 1
  };
}

mat44 mat44::makeOrtho2D(const vec2& bottomLeft, const vec2& topRight) {
  return makeOrtho(bottomLeft.x, topRight.x, bottomLeft.y, topRight.y, -1, 1);
}

mat44 mat44::makeOrtho(float l, float r, float b, float t, float nz, float fz) {
  return {
    2/(r-l), 0, 0, 0,
    0, 2/(t-b), 0, 0,
    0, 0, 1/(nz-fz), 0,
    (l+r)/(l-r), (t+b)/(b-t), nz/(nz-fz), 1
  };
}
