#include "Material.hpp"
#include "Engine/Renderer/glFunctions.hpp"
#include "Engine/Core/Resource.hpp"
#include "Engine/Renderer/Shader/Shader.hpp"

void bindFloat(uint loc, uint num, void* data) {
  glUniform1fv(loc, num, (float*)data);
}

void MatProp<float>::bind(uint bindPoint) {
  bindFloat(bindPoint, 1, &value);
}

void MatProp<vec2>::bind(uint bindPoint) {
  bindFloat(bindPoint, 2, &value);
}

void MatProp<vec3>::bind(uint bindPoint) {
  bindFloat(bindPoint, 3, &value);
}

void MatProp<Rgba>::bind(uint bindPoint) {
  bindFloat(bindPoint, 4, value.normalized().data);
}

void MatProp<mat44>::bind(uint bindPoint) {
  bindFloat(bindPoint, 16, value.data);
}

MaterialProperty* MatProp<Texture>::clone() const {
  MatProp<Texture>* prop = new MatProp<Texture>(slot, value, name.c_str());
  prop->sampler = sampler;
  return prop;
}

Material::~Material() {
  for(MaterialProperty*& prop: mProps) {
    SAFE_DELETE(prop);
  } 

  SAFE_DELETE(mShader);

}

Shader* Material::shader(Shader* shader) {
  if(mShader == nullptr) {
    EXPECTS(mResShader != nullptr);

    mShader = Resource<Shader>::clone(mResShader);
  }

  return mShader;
}

const Shader* Material::shader(Shader* shader) const {
  return mShader == nullptr ? mResShader.get() : mShader;
}

MaterialProperty*& Material::property(std::string_view name) {
  for (MaterialProperty*& prop : mProps) {
    if (prop->name == name) return prop;
  }

  mProps.push_back(nullptr);

  return mProps.back();
}

const MaterialProperty* Material::property(std::string_view name) const {
  for (const MaterialProperty* prop : mProps) {
    if (prop->name == name) return prop;
  }

  return nullptr;
}

Material::Material(const Material& mat) {
  mResShader = mat.mResShader;
  mShader = mat.mShader;

  mProps.reserve(mat.mProps.size());
  for(const MaterialProperty* prop: mat.mProps) {
    mProps.push_back(prop == nullptr ? nullptr : prop->clone());
  }

  for(uint i = 0; i<NUM_TEXTURE_SLOT; i++) {
    mTextures[i] = mat.mTextures[i] == nullptr ? nullptr : mat.mTextures[i]->clone();
  }
}

InstaMaterial::InstaMaterial(const Material& mat)
  : Material(mat)
  , mParent(&mat) {}

//template<>
//ResDef<Material> Resource<Material>::load(const fs::path& file) {
//  EXPECTS(file.extension() == ".mat");
//
//
//}
