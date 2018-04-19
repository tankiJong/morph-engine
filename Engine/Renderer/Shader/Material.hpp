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
  std::string name;
  virtual void bind(uint bindPoint) = 0;
  virtual MaterialProperty* clone() const = 0;
};


template<typename V>
class MatProp : public MaterialProperty {
public:
  MatProp(const char* name, V&& value) : MaterialProperty(name), value(std::forward(value)) {}

  void bind(uint bindPoint) override;
  MaterialProperty* clone() const override {
    MatProp<V>* prop = new MatProp<V>(name, value);
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

  void bind(uint bindPoint) override;
  MaterialProperty* clone() const override;
};


class Material {
public:
  ~Material();
  Shader* shader(Shader* shader);
  const Shader* shader(Shader* shader) const;

  template<typename T>
  void setProperty(char* name, const T& val) {
    MaterialProperty*& prop = property(name);

    if (prop) {
      delete prop;
    }

    prop = new MatProp<T>(name, val);
  }

  void setTexture(uint slot, const Texture* tex, const Sampler* sampler = nullptr, const char* name = "") {
    MaterialProperty*& prop = mTextures[slot];

    if(prop) {
      delete prop;
    }

    auto texture = new MatProp<Texture>(slot, tex, name);

    if(sampler) {
      texture->sampler = sampler;
    } else {
      texture->sampler = &Sampler::Default();
    }

    prop = texture;
  }

  void setTexture(uint slot, const Texture* tex, const char* name = "", const Sampler* sampler = nullptr) {
    setTexture(slot, tex, sampler, name);
  }

  MaterialProperty*& property(std::string_view name);
  const MaterialProperty* property(std::string_view name) const;

protected:
  Material(const Material& mat);
  S<const Shader> mResShader = nullptr;
  owner<Shader*> mShader = nullptr;
  std::vector<owner<MaterialProperty*>> mProps;
  std::array<MaterialProperty*, NUM_TEXTURE_SLOT> mTextures;
};


class InstaMaterial: public Material {
  InstaMaterial(const Material& mat);
  InstaMaterial(const InstaMaterial&) = delete;
  InstaMaterial(const InstaMaterial&&) = delete;

protected:
  const Material* mParent;
};

template<>
ResDef<Material> Resource<Material>::load(const fs::path& file);