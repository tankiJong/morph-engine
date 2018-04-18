#include "Engine/Core/common.hpp"
#include "Mesher.hpp"
#include "Mesh.hpp"
#include "Engine/Renderer/Font.hpp"
#include "ThirdParty/mikktspace/mikktspace.h"
#include "Engine/Math/MathUtils.hpp"

class MikktBinding : public SMikkTSpaceContext {
public:
  MikktBinding(Mesher* mesher);
  void genTangent();
  static int mikkGetNumFace(const SMikkTSpaceContext* pContext);

  static int mikktGetNumVerticesOfFace(const SMikkTSpaceContext* pContext, const int iFace);

  static void mikktGetPosition(const SMikkTSpaceContext* pContext, float out[], const int iFace, const int iVert);

  static void mikktGetNormal(const SMikkTSpaceContext* pContext, float out[], const int iFace, const int iVert);

  static void mikktGetTexCoords(const SMikkTSpaceContext* pContext, float out[], const int iFace, const int iVert);

  static void mikktSetTSpaceBasic(const SMikkTSpaceContext* pContext,
                                  const float               fvTangent[],
                                  const float               fSign,
                                  const int                 iFace,
                                  const int                 iVert);;

  static void mikktSetTSpace(const SMikkTSpaceContext* pContext,
                             const float               fvTangent[],
                             const float               fvBiTangent[],
                             const float               fMagS,
                             const float               fMagT,
                             const tbool               bIsOrientationPreserving,
                             const int                 iFace,
                             const int                 iVert);

};

MikktBinding::MikktBinding(class Mesher* mesher) {
  m_pUserData = mesher;
  m_pInterface = new SMikkTSpaceInterface{
    MikktBinding::mikkGetNumFace,
    MikktBinding::mikktGetNumVerticesOfFace,
    MikktBinding::mikktGetPosition,
    MikktBinding::mikktGetNormal,
    MikktBinding::mikktGetTexCoords,
    MikktBinding::mikktSetTSpaceBasic,
    MikktBinding::mikktSetTSpace,
  };
}

void MikktBinding::genTangent() {
  bool result = genTangSpaceDefault(this);

  ENSURES(result);
}

int MikktBinding::mikkGetNumFace(const SMikkTSpaceContext* pContext) {
  Mesher& mesher = *(Mesher*)pContext->m_pUserData;
  if (mesher.mIns.prim != DRAW_TRIANGES) return 0;

  return mesher.mIns.startIndex + mesher.currentElementCount() / 3;
}

int MikktBinding::mikktGetNumVerticesOfFace(const SMikkTSpaceContext* /*pContext*/, const int /*iFace*/) {
  return 3;
}

void MikktBinding::mikktGetPosition(const SMikkTSpaceContext* pContext, float out[], const int iFace, const int iVert) {
  Mesher& mesher = *(Mesher*)pContext->m_pUserData;

  int index = mesher.mIns.startIndex + 3 * iFace + iVert;

  index = mesher.mIns.useIndices ? mesher.mIndices[index] : index;

  vec3 pos = mesher.mVertices.vertices().position[index];

  out[0] = pos.x;
  out[1] = pos.y;
  out[2] = pos.z;
}

void MikktBinding::mikktGetNormal(const SMikkTSpaceContext* pContext, float out[], const int iFace, const int iVert) {
  Mesher& mesher = *(Mesher*)pContext->m_pUserData;

  int index = mesher.mIns.startIndex + 3 * iFace + iVert;

  index = mesher.mIns.useIndices ? mesher.mIndices[index] : index;

  vec3 norm = mesher.mVertices.vertices().normal[index];

  out[0] = norm.x;
  out[1] = norm.y;
  out[2] = norm.z;
}

void MikktBinding::mikktGetTexCoords(const SMikkTSpaceContext* pContext, float out[], const int iFace, const int iVert) {
  Mesher& mesher = *(Mesher*)pContext->m_pUserData;

  int index = mesher.mIns.startIndex + 3 * iFace + iVert;

  index = mesher.mIns.useIndices ? mesher.mIndices[index] : index;

  vec2 uv = mesher.mVertices.vertices().uv[index];

  out[0] = uv.x;
  out[1] = uv.y;
}

void MikktBinding::mikktSetTSpaceBasic(const SMikkTSpaceContext* pContext,
                                       const float               fvTangent[],
                                       const float               fSign,
                                       const int                 iFace,
                                       const int                 iVert) {
  Mesher& mesher = *(Mesher*)pContext->m_pUserData;

  int index = mesher.mIns.startIndex + 3 * iFace + iVert;

  index = mesher.mIns.useIndices ? mesher.mIndices[index] : index;

  vec4& tangent = mesher.mVertices.vertices().tangent[index];

  tangent.x = fvTangent[0];
  tangent.y = fvTangent[1];
  tangent.z = fvTangent[2];
  tangent.w = fSign;
}

void MikktBinding::mikktSetTSpace(const SMikkTSpaceContext* pContext,
                                  const float               fvTangent[],
                                  const float[]             /*fvBiTangent[]*/,
                                  const float               /*fMagS*/,
                                  const float               /*fMagT*/,
                                  const tbool               bIsOrientationPreserving,
                                  const int                 iFace,
                                  const int                 iVert) {
  Mesher& mesher = *(Mesher*)pContext->m_pUserData;

  int index = mesher.mIns.startIndex + 3 * iFace + iVert;

  index = mesher.mIns.useIndices ? mesher.mIndices[index] : index;

  vec4& tangent = mesher.mVertices.vertices().tangent[index];

  tangent.x = fvTangent[0];
  tangent.y = fvTangent[1];
  tangent.z = fvTangent[2];
  tangent.w = (float)bIsOrientationPreserving;
}



Mesher& Mesher::begin(eDrawPrimitive prim, bool useIndices) {
  GUARANTEE_OR_DIE(isDrawing == false, "Call begin before previous end get called.");
  mIns.prim = prim;
  mIns.useIndices = useIndices;

  if(useIndices) {
    mIns.startIndex = mIndices.size();
  } else {
    mIns.startIndex = mVertices.count();
  }

  isDrawing = true;
  return *this;
}

void Mesher::end() {
  GUARANTEE_OR_DIE(isDrawing, "Call end without calling begin before");
  uint end = mIns.startIndex + currentElementCount();

  mIns.elementCount = end - mIns.startIndex;
  isDrawing = false;
}

void Mesher::clear() {
  mVertices.clear();
  mIndices.clear();
  mIns = draw_instr_t();
}
Mesher& Mesher::color(const Rgba& c) {
  mStamp.color = c;
  return *this;
}

Mesher& Mesher::normal(const vec3& n) {
  mStamp.normal = n.normalized();
  return *this;
}

Mesher& Mesher::tangent(const vec3& t, float fSign) {
  EXPECTS(fSign == 1.f || fSign == -1.f);
  mStamp.tangent = vec4(t.normalized(), fSign);
  return *this;
}

Mesher& Mesher::uv(const vec2& uv) {
  mStamp.uv = uv;
  return *this;
}

Mesher& Mesher::uv(float u, float v) {
  mStamp.uv.x = u;
  mStamp.uv.y = v;
  return *this;
}

uint Mesher::vertex3f(const vec3& pos) {
  mStamp.position = pos;
  mVertices.push(mStamp);
  return mVertices.count() - 1u;
}

uint Mesher::vertex3f(span<const vec3> verts) {
  mVertices.reserve(mVertices.count() + verts.size());
  for(const vec3& vert: verts) {
    vertex3f(vert);
  }
  return mVertices.count() - verts.size();
}

uint Mesher::vertex3f(float x, float y, float z) {
  mStamp.position.x = x;
  mStamp.position.y = y;
  mStamp.position.z = z;

  mVertices.push(mStamp);
  return mVertices.count() - 1u;
}

uint Mesher::vertex2f(const vec2& pos) {
  return vertex3f(pos.x, pos.y, 0);
}

Mesher& Mesher::genNormal() {
  uint end = mIns.startIndex + currentElementCount();

  vec3* normals = mVertices.vertices().normal;

  if (mIns.prim == DRAW_TRIANGES) {
    if (mIns.useIndices) {
      for (auto i = mIns.startIndex; i + 2 < end; i += 3) {
        vec3 normal = normalOf(mIndices[i], mIndices[i + 1], mIndices[i + 2]);
        normals[mIndices[i]] += normal;
        normals[mIndices[i + 1]] += normal;
        normals[mIndices[i + 2]] += normal;
        //        ENSURES(normals[i].magnitudeSquared() != 0);
      }
    } else {
      for (auto i = mIns.startIndex; i + 2 < end; i += 3) {
        vec3 normal = normalOf(i, i + 1, i + 2);
        normals[i] += normal;
        normals[i + 1] += normal;
        normals[i + 2] += normal;
      }
    }

    for (auto i = mIns.startIndex; i < mVertices.count(); i++) {
      if (normals[i].magnitudeSquared() != 0) {
        normals[i].normalize();
      }
    }
  }

  return *this;
}

Mesher& Mesher::line(const vec3& from, const vec3& to) {
  uint start = vertex3f(from);
  vertex3f(to);

  mIndices.push_back(start);
  mIndices.push_back(start + 1);

  return *this;
}

Mesher& Mesher::sphere(const vec3& center, float size, uint levelX, uint levelY) {
//  mIns.prim = DRAW_POINTS;
//  mIns.useIndices = false;
  float dTheta = 360.f / (float)levelX;
  float dPhi = 180.f / (float)levelY;

  uint start = mVertices.count();

  for (uint j = 0; j <= levelY; j++) {
    for (uint i = 0; i <= levelX; i++) {
      float phi = dPhi * (float)j - 90.f, theta = dTheta * (float)i;
      vec3 pos = fromSpherical(size, theta, phi) + center;
      uv(theta / 360.f, (phi + 90.f) / 180.f);
      normal(pos - center);
      tangent({ -sinDegrees(theta), 0, cosDegrees(theta) }, 1);
      vertex3f(pos);
    }
  }

  for(uint j = 0; j < levelY; j++) {
    for(uint i = 0; i < levelX; i++) {
      uint current = start + j * (levelX+1) + i;
//      triangle(current, current + 1, current + levelX + 1);
//      triangle(current, current + levelX + 1, current + levelX);
      quad(current, current + 1, current + levelX+1 + 1, current + levelX+1);
    }
  }

  return *this;
}

Mesher& Mesher::triangle() {
  EXPECTS(mIns.useIndices);
  uint last = mVertices.count() - 1;

  if (last < 2) {
    ERROR_RECOVERABLE("cannot construct triangle with less than 4 vertices");
  } else {
    triangle(last - 2, last - 1, last);
  }

  return *this;
}

Mesher& Mesher::triangle(uint a, uint b, uint c) {
  switch(mIns.prim) {
    case DRAW_POINTS:
    case DRAW_TRIANGES: {
      mIndices.push_back(a);
      mIndices.push_back(b);
      mIndices.push_back(c);
    } break;
    case DRAW_LINES: {
      mIndices.push_back(a);
      mIndices.push_back(b);
      mIndices.push_back(b);
      mIndices.push_back(c);
      mIndices.push_back(c);
      mIndices.push_back(a);
    } break;
    default:
      ERROR_AND_DIE("unsupported primitive");
  }

  return *this;
}

Mesher& Mesher::quad() {
  EXPECTS(mIns.useIndices);
  uint last = mVertices.count() - 1;

  if(last < 3) {
    ERROR_RECOVERABLE("cannot construct quad with less than 4 vertices");
  } else {
    quad(last - 3, last - 2, last - 1, last);
  }

  return *this;
}

Mesher& Mesher::quad(uint a, uint b, uint c, uint d) {
  switch(mIns.prim) {
    case DRAW_POINTS: {
      mIndices.push_back(a);
      mIndices.push_back(b);
      mIndices.push_back(c);
      mIndices.push_back(d);
    } break;
    case DRAW_LINES: {
      mIndices.push_back(a);
      mIndices.push_back(b);
      mIndices.push_back(b);
      mIndices.push_back(c);
      mIndices.push_back(c);
      mIndices.push_back(d);
      mIndices.push_back(d);
      mIndices.push_back(a);
    } break;
    case DRAW_TRIANGES: {
      triangle(a, b, c);
      triangle(a, c, d);
    } break;
    default: 
      ERROR_AND_DIE("unsupported primitive");
  }

  return *this;
}

Mesher& Mesher::quad(const vec3& a, const vec3& b, const vec3& c, const vec3& d) {

  tangent(b - a, 1);
  normal((d - a).cross(b - a));

  uint start =
  uv({ 0,0 })
    .vertex3f(a);

  uv({ 1,0 })
    .vertex3f(b);

  uv({ 1,1 })
    .vertex3f(c);

  uv({ 0,1 })
    .vertex3f(d);
  quad(start + 0, start + 1, start + 2, start + 3);

  return *this;
}

Mesher& Mesher::cube(const vec3& center, const vec3& dimension) {
  vec3 bottomCenter = center - vec3::up * dimension.y * .5f;
  float dx = dimension.x * .5f, dy = dimension.y * .5f, dz = dimension.z * .5f;

  std::array<vec3, 8> vertices = {
    bottomCenter + vec3{ -dx, 2.f * dy, -dz },
    bottomCenter + vec3{ dx, 2.f * dy, -dz },
    bottomCenter + vec3{ dx, 2.f * dy,  dz },
    bottomCenter + vec3{ -dx, 2.f * dy,  dz },

    bottomCenter + vec3{ -dx, 0, -dz },
    bottomCenter + vec3{ dx, 0, -dz },
    bottomCenter + vec3{ dx, 0,  dz },
    bottomCenter + vec3{ -dx, 0,  dz }
  };

  quad(vertices[0], vertices[1], vertices[2], vertices[3]);
  quad(vertices[4], vertices[7], vertices[6], vertices[5]);
  quad(vertices[4], vertices[5], vertices[1], vertices[0]);
  quad(vertices[5], vertices[6], vertices[2], vertices[1]);
  quad(vertices[6], vertices[7], vertices[3], vertices[2]);
  quad(vertices[7], vertices[4], vertices[0], vertices[3]);

  return *this;
}

Mesher& Mesher::cone(const vec3& origin, const vec3& direction, float length, float angle, uint slide, bool bottomFace) {

  mat44 trans = mat44::lookAt(origin, origin + direction.normalized() * length).inverse();

  uint top = vertex3f((trans * vec4(vec3::zero, 1)).xyz());

  uint bottom = vertex3f((trans * vec4(0, 0, length, 1)).xyz() );

  float radius = tanDegree(angle) * length;

  float dAngle = 360.f / (float)slide;

  vertex3f((trans * vec4(radius, 0, length, 1)).xyz());

  for(float i = dAngle; i <= 360.f; i+=dAngle) {
    uint current = vertex3f((trans * vec4( cosDegrees(i) * radius, sinDegrees(i) * radius, length, 1)).xyz());

    triangle(top, current - 1, current);
    if(bottomFace) triangle(current - 1, bottom, current);
  }

  return *this;
}

Mesher& Mesher::text(const span<const std::string_view> asciiTexts, float size, const Font* font, const vec3& position, const vec3& right, const vec3& up) {
  vec3 cursor = position;

  for (auto& asciiText : asciiTexts) {
    EXPECTS(!asciiText.empty());
    // Draw each line
    vec3 lineStart = cursor;
    uint i = 0;
    {
      aabb2 bounds = font->bounds(asciiText[i], size);
      vec3 bottomLeft = cursor + bounds.mins.x * right + bounds.mins.y * up;
      vec3 bottomRight = cursor + bounds.maxs.x * right + bounds.mins.y * up;
      vec3 topRight = cursor + bounds.maxs.x * right + bounds.maxs.y * up;
      vec3 topLeft = cursor + bounds.mins.x * right + bounds.maxs.y * up;
      auto uvs = font->uv(asciiText[0]).vertices();
      uint start =
        uv(uvs[0])
        .vertex3f(bottomLeft);
      uv(uvs[1])
        .vertex3f(bottomRight);
      uv(uvs[2])
        .vertex3f(topRight);
      uv(uvs[3])
        .vertex3f(topLeft);
      quad(start, start + 1, start + 2, start + 3);
    }
    //    end();

    cursor += font->advance('\0', asciiText[i], size) * right;
    while (++i < asciiText.size()) {
      aabb2 bounds = font->bounds(asciiText[i], size);
      vec3 bottomLeft = cursor + bounds.mins.x * right + bounds.mins.y * up;
      vec3 bottomRight = cursor + bounds.maxs.x * right + bounds.mins.y * up;
      vec3 topRight = cursor + bounds.maxs.x * right + bounds.maxs.y * up;
      vec3 topLeft = cursor + bounds.mins.x * right + bounds.maxs.y * up;
      auto uvs = font->uv(asciiText[i]).vertices();

      uint start =
        uv(uvs[0])
        .vertex3f(bottomLeft);
      uv(uvs[1])
        .vertex3f(bottomRight);
      uv(uvs[2])
        .vertex3f(topRight);
      uv(uvs[3])
        .vertex3f(topLeft);
      quad(start, start + 1, start + 2, start + 3);

      cursor += font->advance(asciiText[i - 1], asciiText[i], size) * right;
    }

    cursor = lineStart - font->lineHeight(size) * up;
  }

  return *this;
}

void Mesher::mikkt() {
  EXPECTS(!mIns.useIndices);
  MikktBinding binding(this);
  binding.genTangent();
}


vec3 Mesher::normalOf(uint a, uint b, uint c) {
  vec3* verts = mVertices.vertices().position;

  vec3 ab = verts[b] - verts[a];
  vec3 ac = verts[c] - verts[a];

  vec3 xx = vec3::right.cross(vec3::up);
  return ac.cross(ab);
}

uint Mesher::currentElementCount() const {
  return  mIns.useIndices ? mIndices.size() : mVertices.count();
}


void Mesher::surfacePatch(const delegate<vec3(const vec2&)>& parametric) {
  return surfacePatch({ 0.f, 1.f }, { 0.f, 1.f }, 10u, 10u, parametric);
}

void Mesher::surfacePatch(const FloatRange& u, const FloatRange& v, uint levelX, uint levelY, const delegate<vec3(const vec2&)>& parametric) {
  constexpr float eps = 1e-6f;
  uint start = mVertices.count();

  float stepX = u.length() / float(levelX);
  float stepY = v.length() / float(levelY);

  for (uint j = 0; j <= levelY; j++) {
    for (uint i = 0; i <= levelX; i++) {
      float x = u.min + stepX * (float)i;
      float y = v.min + stepY * (float)j;
      uv(x, y);
      vec3 tan = (parametric({ x + eps, y }) - parametric({ x - eps, y })) / (2 * eps);
      vec3 bitan = (parametric({ x, y + eps }) - parametric({ x, y - eps })) / ( 2 * eps);
      if(tan.magnitude() >= eps) {
        tangent(tan);
        normal(bitan.cross(tan));
      }
      vertex3f(parametric({ x,y }));
    }
  }

  for(uint j = 0; j < levelY; j++) {
    for(uint i = 0; i < levelX; i++) {
      uint current = start + j * (levelX+1) + i;
      quad(current, current + 1, current + levelX+1 + 1, current + levelX+1);
    }
  }
}

//
//Mesher& Mesher::sphere(const vec3& center, float size, uint levelX, uint levelY) {
//  //  mIns.prim = DRAW_POINTS;
//  //  mIns.useIndices = false;
//  float dTheta = 360.f / (float)levelX;
//  float dPhi = 180.f / (float)levelY;
//
//  uint start = mVertices.count();
//
//  for (uint j = 0; j <= levelY; j++) {
//    for (uint i = 0; i <= levelX; i++) {
//      float phi = dPhi * (float)j - 90.f, theta = dTheta * (float)i;
//      vec3 pos = fromSpherical(size, theta, phi) + center;
//      uv(theta / 360.f, (phi + 90.f) / 180.f);
//      normal(pos - center);
//      tangent({ -sinDegrees(theta), 0, cosDegrees(theta) }, 1);
//      vertex3f(pos);
//    }
//  }
//
//  for (uint j = 0; j < levelY; j++) {
//    for (uint i = 0; i < levelX; i++) {
//      uint current = start + j * (levelX + 1) + i;
//      //      triangle(current, current + 1, current + levelX + 1);
//      //      triangle(current, current + levelX + 1, current + levelX);
//      quad(current, current + 1, current + levelX + 1 + 1, current + levelX + 1);
//    }
//  }
//
//  return *this;
//}