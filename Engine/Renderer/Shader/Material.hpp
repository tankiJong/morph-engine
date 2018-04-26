#pragma once
#include "Engine/Core/common.hpp"
#include "Engine/Renderer/type.h"
#include "Engine/Renderer/Sampler.hpp"
#include "Engine/Core/Resource.hpp"
#include <vector>

class Texture;
class Shader;
class Sampler;

class MaterialProperty {
public:
  MaterialProperty(const char* str) : name(str) {}
  MaterialProperty(const MaterialProperty&&) = delete;
  virtual ~MaterialProperty() = default;
  virtual void bind(uint bindPoint) = 0;
  virtual MaterialProperty* clone() const = 0;
  std::string name;
};


template<typename V>
class MatProp : public MaterialProperty {
public:
  MatProp(const char* name, V&& value) : MaterialProperty(name), value(std::forward(value)) {}
  MatProp(const char* name, const V& value) : MaterialProperty(name), value(value) {}

  void bind(uint bindPoint) override;
  MaterialProperty* clone() const override {
    MatProp<V>* prop = new MatProp<V>(name.c_str(), value);
    return prop;
  }
  V value;
};

template<>
class MatProp<Texture>: public MaterialProperty {
public:
  MatProp(uint slot, const Texture* value, const char* name = "") : MaterialProperty(name), slot(slot), value(value) {}
  uint slot;
  const Texture* value;
  const Sampler* sampler = nullptr;

  inline void bind(uint bindPoint) override { ERROR_AND_DIE("Don't call bind for texture"); };
  MaterialProperty* clone() const override;
};


class Material {
public:
  ~Material();
  Material() = default;
  void shader(S<const Shader> shader);
  const Shader* shader() const;
  Shader* shader();

  template<typename T>
  void setProperty(const char* name, const T& val) {
    static_assert(!std::is_same_v<T, Texture>, "use setTexture to set texture");
    MaterialProperty*& prop = property(name);

    if (prop) {
      delete prop;
    }

    prop = new MatProp<T>(name, val);
  }

  void setTexture(uint slot, const Texture* tex, const Sampler* sampler = nullptr, const char* name = "");

  inline void setTexture(uint slot, const Texture* tex, const char* name, const Sampler* sampler = nullptr) {
    setTexture(slot, tex, sampler, name);
  }

  MaterialProperty*& property(std::string_view name);
  const MaterialProperty* property(std::string_view name) const;

  inline span<MaterialProperty* const> properties() const { return mProps; };
  inline span<MatProp<Texture>* const> textures() const { return mTextures; };
protected:
  Material(const Material& mat);
  S<const Shader> mResShader = nullptr;
  owner<Shader*> mShader = nullptr;
  std::vector<owner<MaterialProperty*>> mProps;
  std::array<MatProp<Texture>*, NUM_TEXTURE_SLOT> mTextures;
};


class InstaMaterial: public Material {
  InstaMaterial(const Material& mat);
  InstaMaterial(const InstaMaterial&) = delete;
  InstaMaterial(const InstaMaterial&&) = delete;
};

template<>
ResDef<Material> Resource<Material>::load(const fs::path& file);