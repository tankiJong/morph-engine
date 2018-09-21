#include "Engine/Framework/Light.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Primitives/frustum.hpp"
#include "Engine/Math/Primitives/AABB2.hpp"
#include "Engine/Math/Primitives/aabb3.hpp"
#include "Engine/Debug/Draw.hpp"

void Light::asDirectionalLight(float intensity, const vec3& attenuation, const Rgba& color) {
  mInfo.asDirectionalLight(transform.position(), transform.forward(), intensity, attenuation, color);
  type = LIGHT_DIRECTIONAL;
  updateCamera();
}

void Light::asPointLight(float intensity, const vec3& attenuation, const Rgba& color) {
  mInfo.asPointLight(transform.position(), intensity, attenuation, color);
  type = LIGHT_POINT;
  updateCamera();
}

void Light::asPointLight(float intensity, const vec3& attenuation, const vec3& color) {
  mInfo.asPointLight(transform.position(), intensity, attenuation, color);
  type = LIGHT_POINT;
  updateCamera();
}

void Light::asSpotLight(float innerAngle, float outerAngle, float intensity, const vec3& attenuation, const Rgba& color) {
  mInfo.asSpotLight(transform.position(), transform.forward(), innerAngle, outerAngle, intensity, attenuation, color);
  type = LIGHT_SPOT;
  updateCamera();
}

light_info_t& Light::info() {
  mInfo.vp = mCamera.projection() * mCamera.view();
  mInfo.position = transform.position();
  mInfo.direction = transform.forward();
  return mInfo;
}

// RenderTarget& Light::shadowMap() {
//   if(mShadowMap == nullptr) {
//     mShadowMap = new RenderTarget(SHADOW_MAP_SIZE, SHADOW_MAP_SIZE, TEXTURE_FORMAT_D24S8);
//   }
//   return *mShadowMap;
// }
//
// Camera& Light::camera() {
//   mCamera.setDepthStencilTarget(&shadowMap());
//   mCamera.transfrom() = transform;
//
//   return mCamera;
// }

float Light::fovAngle() const {
  return acosDegrees(mInfo.dotOuterAngle);
}

float Light::attenuation(vec3 position) const {
  float distance = transform.position().distance(position);

  return mInfo.color.a / (mInfo.attenuation.x + mInfo.attenuation.y * distance + mInfo.attenuation.z * distance * distance);
}

void Light::updateCamera() {}

// void Light::updateCamera(const Camera& view) {
//   constexpr float viewFZ = 30.f;
//   mat44 vp = view.projection()*view.view();
//   mat44 viewCameraToWorld = (vp).inverse();
//   vec4 viewFzPosition = vp * vec4(view.transfrom().position() + view.transfrom().forward() * viewFZ, 1.f);
//   // Debug::drawCube(view.transfrom().position() + view.transfrom().forward() * viewFZ, 1.f, false, 0.f);
//   float ndcFarZ = viewFzPosition.z / viewFzPosition.w;
//
//   aabb2 bound{ {-1, -1}, {1, 1} };
//
//   auto verts = bound.vertices();
//
//   mat44 toLightSpace = transform.worldToLocal() * viewCameraToWorld;
//   std::array<vec4, 8> corners = {
//     toLightSpace * vec4{ verts[0], -1.f, 1.f},
//     toLightSpace * vec4{ verts[1], -1.f, 1.f},
//     toLightSpace * vec4{ verts[2], -1.f, 1.f},
//     toLightSpace * vec4{ verts[3], -1.f, 1.f},
//     toLightSpace * vec4{ verts[0], ndcFarZ, 1.f},
//     toLightSpace * vec4{ verts[1], ndcFarZ, 1.f},
//     toLightSpace * vec4{ verts[2], ndcFarZ, 1.f},
//     toLightSpace * vec4{ verts[3], ndcFarZ, 1.f},
//   };
//
//   aabb3 box;
//
//   for(vec4 ele: corners) {
//     box.grow(ele.xyz());
//   }
//
//   box.min.z -= 1.f;
//   {
//     aabb3 b;
//     b.min = vec3(-1.f);
//     b.max = vec3(1.f, 1.f, ndcFarZ);
//     Debug::drawCube(b, viewCameraToWorld, true, 0);
//   }
//   Debug::drawCube(box, transform.localToWorld(), true, 10.f, {Rgba::red, Rgba::green});
//   switch (type) {
//     case LIGHT_UNKNOWN: return;
//     case LIGHT_SPOT: {
//       float fz = 100.f, nz = 0.01f;
//       vec2 size(10.f, 10.f);
//       mCamera.setProjectionPrespective(fovAngle() * 2, 5.f, 5.f, nz, fz);
//       }
//       break;
//     case LIGHT_DIRECTIONAL: {
//       vec3 origin = box.center();
//       origin.z = 0;
//       mat44 t = mat44::makeTranslation((transform.localToWorld() * vec4(origin, 1.f)).xyz());
//       mat44 r = mat44::makeRotation(transform.rotation());
//       mCamera.transfrom().setWorldTransform(t * r);
//       mCamera.setProjectionOrtho(box.size().x, box.size().y, box.min.z, box.max.z);
//     } break;
//     case LIGHT_POINT: return;
//     case NUM_LIGHT_TYPE: return;
//     default:;
//   }
// }
//
// void Light::updateCamera() {
//   float fz = 100.f, nz = 0.01f;
//   vec2 size(30.f, 30.f);
//
//   switch (type) {
//     case LIGHT_UNKNOWN: return;
//     case LIGHT_SPOT:
//       mCamera.setProjectionPrespective(fovAngle()*2, 5.f, 5.f, nz, fz);
//       break;
//     case LIGHT_DIRECTIONAL:
//       mCamera.setProjectionOrtho(size.x, size.y, nz, fz);
//       break;
//     case LIGHT_POINT: return;
//     case NUM_LIGHT_TYPE: return;
//     default:;
//   }
// }
//
