#include "mat44.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Vec3.hpp"
#include "Engine/Debug/ErrorWarningAssert.hpp"

const mat44 mat44::identity;
const mat44 mat44::right     { vec4::right, vec4::zero, vec4::zero, vec4::zero };
const mat44 mat44::up        { vec4::zero, vec4::up, vec4::zero, vec4::zero    };
const mat44 mat44::forward   { vec4::zero, vec4::zero, vec4::forward, vec4::zero };

static const mat44 up;

// z,k
static const mat44 forward;

mat44::mat44(float ix, float jx, float kx, float tx,
             float iy, float jy, float ky, float ty,
             float iz, float jz, float kz, float tz,
             float iw, float jw, float kw, float tw)
  : ix(ix), jx(jx), kx(kx), tx(tx),
    iy(iy), jy(jy), ky(ky), ty(ty),
    iz(iz), jz(jz), kz(kz), tz(tz),
    iw(iw), jw(jw), kw(kw), tw(tw) {
}

mat44::mat44(const float* sixteenValuesBasisMajor) {
  set(sixteenValuesBasisMajor);
}
mat44::mat44(const vec2& iBasis, const vec2& jBasis, const vec2& translation)
  : ix(iBasis.x), jx(jBasis.x), kx(0), tx(translation.x),
    iy(iBasis.y), jy(jBasis.y), ky(0), ty(translation.y),
    iz(0), jz(0), kz(1), tz(0),
    iw(0), jw(0), kw(0), tw(1) {
  
}

mat44::mat44(const vec4& i, const vec4& j, const vec4& k, const vec4& t):
  ix(i.x), jx(j.x), kx(k.x), tx(t.x),
  iy(i.y), jy(j.y), ky(k.y), ty(t.y),
  iz(i.z), jz(j.z), kz(k.z), tz(t.z),
  iw(i.w), jw(j.w), kw(k.w), tw(t.w) {
}

vec2 mat44::translateTo(const vec2& position2D) const {
  float _x = ix*position2D.x + jx*position2D.y + tx;
  float _y = iy*position2D.x + jy*position2D.y + ty;
  return { _x, _y };
}

vec2 mat44::translate(const vec2& displacement2D) const {
  float _x = ix*displacement2D.x + jx*displacement2D.y;
  float _y = iy*displacement2D.x + jy*displacement2D.y;
  return { _x,_y };
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
  mat44 temp = matrixToPrepend;
  temp.append(*this);
  memcpy(this->data, temp.data, sizeof(float) * 16);
  return *this;
}

mat44& mat44::append(const mat44& matrixToAppend) {
  vec4
    pi(matrixToAppend.ix, matrixToAppend.iy, matrixToAppend.iz, matrixToAppend.iw),
    pj(matrixToAppend.jx, matrixToAppend.jy, matrixToAppend.jz, matrixToAppend.jw),
    pk(matrixToAppend.kx, matrixToAppend.ky, matrixToAppend.kz, matrixToAppend.kw),
    pt(matrixToAppend.tx, matrixToAppend.ty, matrixToAppend.tz, matrixToAppend.tw);

  vec4 _x(ix, jx, kx, tx), _y(iy, jy, ky, ty), _z(iz, jz, kz, tz), _w(iw, jw, kw, tw);

  ix = _x.dot(pi); jx = _x.dot(pj); kx = _x.dot(pk); tx = _x.dot(pt);
  iy = _y.dot(pi); jy = _y.dot(pj); ky = _y.dot(pk); ty = _y.dot(pt);
  iz = _z.dot(pi); jz = _z.dot(pj); kz = _z.dot(pk); tz = _z.dot(pt);
  iw = _w.dot(pi); jw = _w.dot(pj); kw = _w.dot(pk); tw = _w.dot(pt);

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

vec4 mat44::x() const {
  return { ix, jx, kx, tx };
}
vec4 mat44::y() const {
  return { iy, jy, ky, ty };
}
vec4 mat44::z() const {
  return { iz, jz, kz, tz };
}
vec4 mat44::w() const {
  return { iw, jw, kw, tw };
}

mat44 mat44::operator*(const mat44& rhs) const {
  return mat44(*this).append(rhs);
}

vec4 mat44::operator*(const vec4& rhs) const {
  return {
    x().dot(rhs),
    y().dot(rhs),
    z().dot(rhs),
    w().dot(rhs)
  };
}

bool mat44::operator==(const mat44& rhs) const {
  return memcmp(data, rhs.data, sizeof(float) * 16) == 0;
}

mat44 mat44::transpose() const {
//    ix, jx, kx, tx,
//    iy, jy, ky, ty,
//    iz, jz, kz, tz,
//    iw, jw, kw, tw
  return {
    ix, iy, iz, iw,
    jx, jy, jz, jw,
    kx, ky, kz, kw,
    tx, ty, tz, tw,
  };
}

mat44 mat44::inverse() const {
  ERROR_AND_DIE("unimplemented");
  return mat44();
}

Eular mat44::eular() const {
  vec3 ii = i.xyz().normalized();
  vec3 jj = j.xyz().normalized();
  vec3 kk = k.xyz().normalized();

  Eular e;

  e.x = asinDegrees(-jj.z);
  // cosx = 0
  if(jj.z == 1.f || jj.z == -1.f) {
    e.y = 0;
    e.z = atan2Degree(kk.y, kk.x);

    return e;
  }

  e.z = atan2Degree(jj.y, jj.x);
  e.y = atan2Degree(kk.z, ii.z);

  return e;
}

vec3 mat44::scale() const {
  return {
    i.xyz().magnitude(),
    j.xyz().magnitude(),
    k.xyz().magnitude(),
  };
}

mat44 mat44::makeRotation(const Eular& ea) {
  return makeRotation(ea.x, ea.y, ea.z);
}

// change to (v)ZXY : Done
mat44 mat44::makeRotation(float x, float y, float z) {
  float cx = cosDegrees(x), cy = cosDegrees(y), cz = cosDegrees(z);
  float sx = sinDegrees(x), sy = sinDegrees(y), sz = sinDegrees(z);

  return {
    cz*cy + sz*sx*sy,  sz*cx, sz*sx*cy - cz*sy, 0,
    -sz*cy + cz*sx*sy, cz*cx, cz*sx*cy + sz*sy, 0,
    cx*sy,               -sx, cx*cy,            0,
    0,                     0, 0,                1
  };
//  return {
//    cx*cy, cx*sy*sz - sx*cz, cx*sy*cz + sx*sz, 0,
//    sx*cy, sx*sy*sz + cx*cz, sx*sy*cz - cx*sz, 0,
//    -sy,   cy*sz,            cy*cz,            0,
//    0,     0,                0,                1
//  };
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

mat44 mat44::makeTranslation(const vec3& translation) {
  return {
    1, 0, 0, translation.x,
    0, 1, 0, translation.y,
    0, 0, 1, translation.z,
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

mat44 mat44::makeScale(float x, float y, float z) {
  return {
    x,  0, 0, 0,
    0,  y, 0, 0,
    0,  0, z, 0,
    0,  0, 0, 1
  };
}

mat44 mat44::makeOrtho2D(const vec2& bottomLeft, const vec2& topRight) {
  return makeOrtho(bottomLeft.x, topRight.x, bottomLeft.y, topRight.y, -1, 1);
}

mat44 mat44::makeOrtho(float l, float r, float b, float t, float nz, float fz) {
  return {
    2/(r-l), 0, 0, (l+r)/(l-r),
    0, 2/(t-b), 0, (t+b)/(b-t),
    0, 0, 2/(fz-nz), (fz+nz)/(nz-fz),
    0,0,0, 1
  };
}

mat44 mat44::makeOrtho(float width, float height, float near, float far) {
  return makeOrtho(-width * .5f, width * .5f, -height * .5f, height * .5f, near, far);
}


// This is for openGL, openGL ndc z is from -1 ~ 1(while directx is from 0,1)
mat44 mat44::makePerspective(float fovDeg, float aspect, float nz, float fz) {
  float d = 1.f / tanDegree(fovDeg * .5f);
  return {
    d / aspect, 0, 0, 0,
    0,          d, 0, 0,
    0,          0, (fz + nz) / (fz-nz), 2*nz*fz/(nz-fz),
    0,          0, 1, 0
  };
}

mat44 mat44::makePerspective(float fovDeg, float width, float height, float nz, float fz) {
  return makePerspective(fovDeg, width / height, nz, fz);
}
