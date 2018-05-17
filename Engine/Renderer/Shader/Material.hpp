#pragma once
#include "Engine/Core/common.hpp"
#include "Engine/Renderer/type.h"
#include "Engine/Renderer/Sampler.hpp"
#include "Engine/Core/Resource.hpp"
#include "Engine/Renderer/Shader/PropertyBlock.hpp"
#include <vector>

class Texture;
class Shader;
class Sampler;

class MaterialProperty {
public:
  MaterialProperty(const char* str) : name(str) {}
  MaterialProperty(const char* name, S<const PropertyBlockInfo> info, void* data) : MaterialProperty(name) {
    EXPECTS(info.get() != nullptr);
    value.info = std::move(info);
    value.put(value.info->totalSize, data);
  }

  MaterialProperty(const MaterialProperty&&) = delete;
  virtual ~MaterialProperty() = default;
  virtual void bind(uint bindPoint);
  virtual MaterialProperty* clone() const {
    MaterialProperty* prop = new MaterialProperty(name.c_str(), value.info, value.mData);
    return prop;
  };
  std::string name;
  PropertyBlock value;
};

template<typename V>
class MatProp : public MaterialProperty {
public:
  MatProp(const char* name, const V& val) : MaterialProperty(name) {
    INFO("using property instead of property block, be careful about the layout and padding");
    value.set(val);
  }

  void set(const V& val) { value.set(val); };
};

template<>
class MatProp<Texture> : public MaterialProperty {
public:
  MatProp(uint slot, const Texture* value, const char* name = "") : MaterialProperty(name), slot(slot), value(value) {}
  uint slot;
  const Texture* value;
  const Sampler* sampler = nullptr;

  inline void bind(uint /*bindPoint*/) override { ERROR_AND_DIE("Don't call bind for texture"); };
  MaterialProperty* clone() const override;
};

class Material {
public:
  virtual ~Material();
  Material() { mTextures.fill(nullptr); };
  void shader(S<const Shader> shader);
  const Shader* shader() const;
  template<typename T>
  void setProperty(const char* name, const T& val) {
    static_assert(!std::is_same_v<T, Texture>, "use setTexture to set texture");
    static_assert(!std::is_pointer_v<T>, "does not expect T is pointer type");

    setProperty(name, &val, sizeof(T));
  }

  void setProperty(std::string_view name, const void* data, uint size);
  void setTexture(uint slot, const Texture* tex, const Sampler* sampler = nullptr, const char* nameid = "");

  inline void setTexture(uint slot, const Texture* tex, const char* nameid, const Sampler* sampler = nullptr) {
    setTexture(slot, tex, sampler, nameid);
  }

  void setPropertyBlock(S<const PropertyBlockInfo> layout, void* data);
  void setPropertyBlock(std::string_view name, void* data, uint size);

  template<typename T>
  void setPropertyBlock(std::string_view name, const T& data) {
    setPropertyBlock(name, &data, sizeof(T));
  }
  MaterialProperty*& property(std::string_view name);
  const MaterialProperty* property(std::string_view name) const;

  inline span<MaterialProperty* const> properties() const { return mProps; };
  inline span<MatProp<Texture>* const> textures() const { return mTextures; };

  std::string name = "";
  static owner<Material*> fromYaml(const fs::path& file);
protected:
  Material(const Material& mat);
  S<const Shader> mResShader = nullptr;
  owner<Shader*> mShader = nullptr;
  std::vector<owner<MaterialProperty*>> mProps;
  std::array<MatProp<Texture>*, NUM_TEXTURE_SLOT> mTextures;
};


class InstaMaterial: public Material {
public:
  InstaMaterial(const Material& mat);
  InstaMaterial(const InstaMaterial&) = delete;
  InstaMaterial(const InstaMaterial&&) = delete;
};

template<>
ResDef<Material> Resource<Material>::load(const std::string& file);

template<>
owner<Material*> Resource<Material>::clone(S<const Material> res);