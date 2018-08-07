#pragma once
#include "Engine/Core/common.hpp"
#include "Engine/Graphics/RHI/Texture.hpp"
#include "Engine/Graphics/RHI/DescriptorSet.hpp"

class MatParamater {
public:
  struct Entry {
    std::string name;
    size_t offset = 0;
    size_t elementSize = 0;
    size_t arraySize = 0;
  };
protected:
  DescriptorSet mDescriptorSet;
  std::string   mName;
};

class Material {
public:
  void setTexture(eTextureSlot prop, Texture2::sptr_t tex);

  void setProperty(uint registerIndex, const void* data, size_t size);
protected:
  std::array<Texture2::sptr_t, NUM_TEXTURE_SLOT> mPropertyTextures;
  
};
