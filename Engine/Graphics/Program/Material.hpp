#pragma once
#include "Engine/Core/common.hpp"
#include "Engine/Graphics/RHI/Texture.hpp"

class Material {
public:
  enum eMaterialProp {
    MAT_PROP_ALBEDO,
    MAT_PROP_SPECULAR,
    MAT_PROP_NORMAL,
    MAT_PROP_EMISSIVE,
  };

  void setTexture(eMaterialProp prop, Texture2::sptr_t tex);
};
