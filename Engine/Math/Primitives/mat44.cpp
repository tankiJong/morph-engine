#include "mat44.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "vec3.hpp"
#include "Engine/Debug/ErrorWarningAssert.hpp"
#include "Engine/Math/Primitives/quaternion.hpp"

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

mat44::mat44(const vec3& _right, const vec3& _up, const vec3& _forward, const vec3& _translation) {
  i = vec4(_right, 0);
  j = vec4(_up, 0);
  k = vec4(_forward, 0);
  t = vec4(_translation, 1.f);
}

mat44::mat44(const vec4& i, const vec4& j, const vec4& k, const vec4& t):
  ix(i.x), jx(j.x), kx(k.x), tx(t.x),
  iy(i.y), jy(j.y), ky(k.y), ty(t.y),
  iz(i.z), jz(j.z), kz(k.z), tz(t.z),
  iw(i.w), jw(j.w), kw(k.w), tw(t.w) {
}

mat44::mat44(const quaternion& q) {
  // imaginary part
  float const x = q.v.x;
  float const y = q.v.y;
  float const z = q.v.z;

  // cache off some squares
  float const x2 = x * x;
  float const y2 = y * y;
  float const z2 = z * z;

  i = vec4(1.0f - 2.0f * y2 - 2.0f * z2,
          2.0f * x * y + 2.0f * q.r * z,
          2.0f * x * z - 2.0f * q.r * y,
          0);

  j = vec4(2 * x * y - 2.0f * q.r * z,
          1.0f - 2.0f * x2 - 2.0f * z2,
          2.0f * y * z + 2.0f * q.r * x,
          0);

  k = vec4(2.0f * x * z + 2.0f * q.r * y,
          2.0f * y * z - 2.0f * q.r * x,
          1.0f - 2.0f * x2 - 2.0f * y2,
          0);

  t = vec4(0, 0, 0, 1);
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

float mat44::operator()(uint x, uint y) const {
  return data[y * 4 + x];
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
  double inv[16];
  double det;
  double m[16];
  uint ii;

  for (ii = 0; ii < 16; ++ii) {
    m[ii] = (double)data[ii];
  }

  inv[0] = m[5] * m[10] * m[15] -
    m[5] * m[11] * m[14] -
    m[9] * m[6] * m[15] +
    m[9] * m[7] * m[14] +
    m[13] * m[6] * m[11] -
    m[13] * m[7] * m[10];

  inv[4] = -m[4] * m[10] * m[15] +
    m[4] * m[11] * m[14] +
    m[8] * m[6] * m[15] -
    m[8] * m[7] * m[14] -
    m[12] * m[6] * m[11] +
    m[12] * m[7] * m[10];

  inv[8] = m[4] * m[9] * m[15] -
    m[4] * m[11] * m[13] -
    m[8] * m[5] * m[15] +
    m[8] * m[7] * m[13] +
    m[12] * m[5] * m[11] -
    m[12] * m[7] * m[9];

  inv[12] = -m[4] * m[9] * m[14] +
    m[4] * m[10] * m[13] +
    m[8] * m[5] * m[14] -
    m[8] * m[6] * m[13] -
    m[12] * m[5] * m[10] +
    m[12] * m[6] * m[9];

  inv[1] = -m[1] * m[10] * m[15] +
    m[1] * m[11] * m[14] +
    m[9] * m[2] * m[15] -
    m[9] * m[3] * m[14] -
    m[13] * m[2] * m[11] +
    m[13] * m[3] * m[10];

  inv[5] = m[0] * m[10] * m[15] -
    m[0] * m[11] * m[14] -
    m[8] * m[2] * m[15] +
    m[8] * m[3] * m[14] +
    m[12] * m[2] * m[11] -
    m[12] * m[3] * m[10];

  inv[9] = -m[0] * m[9] * m[15] +
    m[0] * m[11] * m[13] +
    m[8] * m[1] * m[15] -
    m[8] * m[3] * m[13] -
    m[12] * m[1] * m[11] +
    m[12] * m[3] * m[9];

  inv[13] = m[0] * m[9] * m[14] -
    m[0] * m[10] * m[13] -
    m[8] * m[1] * m[14] +
    m[8] * m[2] * m[13] +
    m[12] * m[1] * m[10] -
    m[12] * m[2] * m[9];

  inv[2] = m[1] * m[6] * m[15] -
    m[1] * m[7] * m[14] -
    m[5] * m[2] * m[15] +
    m[5] * m[3] * m[14] +
    m[13] * m[2] * m[7] -
    m[13] * m[3] * m[6];

  inv[6] = -m[0] * m[6] * m[15] +
    m[0] * m[7] * m[14] +
    m[4] * m[2] * m[15] -
    m[4] * m[3] * m[14] -
    m[12] * m[2] * m[7] +
    m[12] * m[3] * m[6];

  inv[10] = m[0] * m[5] * m[15] -
    m[0] * m[7] * m[13] -
    m[4] * m[1] * m[15] +
    m[4] * m[3] * m[13] +
    m[12] * m[1] * m[7] -
    m[12] * m[3] * m[5];

  inv[14] = -m[0] * m[5] * m[14] +
    m[0] * m[6] * m[13] +
    m[4] * m[1] * m[14] -
    m[4] * m[2] * m[13] -
    m[12] * m[1] * m[6] +
    m[12] * m[2] * m[5];

  inv[3] = -m[1] * m[6] * m[11] +
    m[1] * m[7] * m[10] +
    m[5] * m[2] * m[11] -
    m[5] * m[3] * m[10] -
    m[9] * m[2] * m[7] +
    m[9] * m[3] * m[6];

  inv[7] = m[0] * m[6] * m[11] -
    m[0] * m[7] * m[10] -
    m[4] * m[2] * m[11] +
    m[4] * m[3] * m[10] +
    m[8] * m[2] * m[7] -
    m[8] * m[3] * m[6];

  inv[11] = -m[0] * m[5] * m[11] +
    m[0] * m[7] * m[9] +
    m[4] * m[1] * m[11] -
    m[4] * m[3] * m[9] -
    m[8] * m[1] * m[7] +
    m[8] * m[3] * m[5];

  inv[15] = m[0] * m[5] * m[10] -
    m[0] * m[6] * m[9] -
    m[4] * m[1] * m[10] +
    m[4] * m[2] * m[9] +
    m[8] * m[1] * m[6] -
    m[8] * m[2] * m[5];

  det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];
  det = 1.0 / det;

  mat44 inver;
  for (ii = 0; ii < 16; ii++) {
    inver.data[ii] = (float)(inv[ii] * det);
  }

  return inver;
}

Euler mat44::euler() const {
  vec3 ii = i.xyz().normalized();
  vec3 jj = j.xyz().normalized();
  vec3 kk = k.xyz().normalized();

  Euler e;

  e.x = asinDegrees(kk.y);
  // cosx = 0
  if(kk.y == 1.f || kk.y == -1.f) {
    e.y = 0;
    e.z = atan2Degree(jj.x, ii.x);

    return e;
  }

  e.z = atan2Degree(-ii.y, jj.y);
  e.y = atan2Degree(-kk.x, kk.z);

  return e;
}

quaternion mat44::quat() const {
  UNIMPLEMENTED_RETURN(quaternion());
}

vec3 mat44::scale() const {
  return {
    i.xyz().magnitude(),
    j.xyz().magnitude(),
    k.xyz().magnitude(),
  };
}

mat44 mat44::makeRotation(const Euler& ea) {
  return makeRotation(ea.x, ea.y, ea.z);
}

// change to (v)ZXY : Done
mat44 mat44::makeRotation(float x, float y, float z) {
  float cx = cosDegrees(x), cy = cosDegrees(y), cz = cosDegrees(z);
  float sx = sinDegrees(x), sy = sinDegrees(y), sz = sinDegrees(z);

  mat44 re{
    cz*cy - sz*sx*sy,      sz*cy + cz * sx*sy,    -cx * sy,    0,
    -sz * cx,                           cz*cx,         sx,    0,
    sz*sx*cy + cz * sy, -cz * sx*cy + sz * sy,      cx*cy,    0,
    0,                                      0,          0,    1
  };

  return re;

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
  return makeOrtho(0, width, 0, height, near, far);
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

mat44 mat44::lookAt(const vec3& position, const vec3& target, const vec3& _up) {
  mat44 t = mat44::makeTranslation(position);

  vec3 _forward = (target - position).normalized();

  float dot = _forward.dot(_up);
  vec3 _right;

  vec3 newUp;
  if(equal(dot, -1.f) || equal(dot, 1.f)) {
    newUp = _forward.cross(vec3::right).normalized();
    _right = newUp.cross(_forward).normalized();
  } else {
    _right = _up.cross(_forward).normalized();
    newUp = _forward.cross(_right);
  }

  mat44 r(vec4(_right, 0),
          vec4(newUp, 0),
          vec4(_forward, 0));

  return t * r;
}
