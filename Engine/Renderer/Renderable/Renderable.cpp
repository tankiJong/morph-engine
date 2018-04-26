#include "Engine/Renderer/Renderable/Renderable.hpp"
#include "Engine/Core/Resource.hpp"
#include "Engine/Renderer/Shader/Material.hpp"

Renderable::Renderable(S<const Material> mat, const Mesh& me, const Transform& trans)
  : mResMaterial(std::move(mat))
  , mMaterial(nullptr)
  , mMesh(&me)
  , mTransform(&trans) {
    
}

Renderable::Renderable(Material& mat, const Mesh& me, const Transform& trans)
  : mResMaterial(nullptr)
  , mMaterial(&mat)
  , mMesh(&me)
  , mTransform(&trans) {}

Renderable::~Renderable() {
  SAFE_DELETE(mMaterial);
}

void Renderable::material(S<const Material> mat) {
  delete mMaterial;
  mResMaterial = mat;
}

Material* Renderable::material() {
  if(!mMaterial) {
    mMaterial = Resource<Material>::clone(mResMaterial);
  }

  return mMaterial;
}

const Material* Renderable::material() const {
  return mMaterial == nullptr ? mResMaterial.get() : mMaterial;
}
