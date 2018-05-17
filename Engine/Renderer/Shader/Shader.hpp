#pragma once
#include "Engine/Core/common.hpp"
#include "Engine/Renderer/type.h"
#include "Engine/Core/Resource.hpp"
#include "Engine/Persistence/yaml.hpp"

class ShaderPass;
class Shader;
namespace YAML {
  class Node;
  template<>
  struct convert<Shader*> {
    static bool decode(const Node& node, Shader*& shader);;
  };
};

class Shader {
  friend bool YAML::convert<Shader*>::decode(const YAML::Node& node, Shader*& shader);
  friend class Resource<Shader>;
public:
  std::string name;
  ~Shader();
  Shader() = default;
  inline ShaderPass*& pass(uint i) { return mPasses.at(i); }
  inline const ShaderPass& pass(uint i) const { return *mPasses.at(i); }

  inline void add(ShaderPass& pass) { mPasses.push_back(&pass); }

  inline span<const ShaderPass* const> passes() const { return mPasses; }

  static owner<Shader*> Shader::fromYaml(const fs::path& file);
protected:
  Shader(const Shader& from);
  std::vector<owner<ShaderPass*>> mPasses;
};


template<>
ResDef<Shader> Resource<Shader>::load(const std::string& file);

template<>
bool Resource<Shader>::reload(const fs::path& file, Shader& shader);

template<>
owner<Shader*> Resource<Shader>::clone(S<const Shader> res);