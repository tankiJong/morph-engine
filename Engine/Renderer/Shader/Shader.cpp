#include "Shader.hpp"
#include "Engine/File/FileSystem.hpp"
#include "Engine/Persistence/yaml.hpp"
#include "Engine/Debug/ErrorWarningAssert.hpp"
#include "Engine/Renderer/Shader/ShaderPass.hpp"
#include "Game/GameCommon.hpp"

Shader::~Shader() {
  for(ShaderPass* pass: mPasses) {
    SAFE_DELETE(pass);
  }   
}

/* implemented version:
x name: shader/example
program:
  path:
  define: defines;seperated;by;semicolons

layer: 0
sort: 1
cull: back|front|none
fill: solid|wire
frontface: ccw|cw
blend: false
blend:
  color:
    op: add|sub|rev_sub|min|max
    src: one|zero|src_color|inv_src_color|src_alpha|inv_src_alpha|dest_color|inv_dest_color|dest_alpha|inv_dest_alpha|constant|inv_constant
    dest: zero
depth:
  write: true|false
  test: less|never|equal|lequal|greater|gequal|not|always
 */
owner<Shader*> Shader::fromYaml(const fs::path& file) {
  auto vfs = FileSystem::Get();

  std::optional<Blob> f = vfs.asBuffer(file);

  if(!f) {
    return false;
  }

  yaml::Node node = yaml::Load(f->as<const char*>());

  Shader* shader = node.as<Shader*>();
  return shader;
}

Shader::Shader(const Shader& from) {
  for(ShaderPass* const pass: from.mPasses) {
    mPasses.push_back(new ShaderPass(*pass));
  }
}

template<>
ResDef<Shader> Resource<Shader>::load(const std::string& file) {
  Shader* shader = Shader::fromYaml(file);
  return { shader->name, shader };
}


template<>
bool Resource<Shader>::reload(const fs::path& file, Shader& shader) {
  auto vfs = FileSystem::Get();

  std::optional<Blob> f = vfs.asBuffer(file);

  if (!f) {
    return false;
  }
  yaml::Node node = yaml::Load(f->as<const char*>());

  shader.name = node["name"] ? node["name"].as<std::string>() : "";

  auto passes = node["pass"];
  EXPECTS(passes.IsSequence() && passes.size() >= 1);

  for(ShaderPass* pass: shader.mPasses) {
    delete pass;
  }
  shader.mPasses.clear();

  for (auto pass : passes) {
    ShaderPass* p = new ShaderPass(std::move(pass.as<ShaderPass>()));
    shader.mPasses.push_back(p);
  }

  return true;
}

bool YAML::convert<Shader*>::decode(const Node& node, Shader*& shader) {
  shader = new Shader();
  shader->name = node["name"] ? node["name"].as<std::string>() : "";

  auto passes = node["pass"];
  EXPECTS(passes && passes.IsSequence() && passes.size() >= 1);

  for (auto pass : passes) {
    ShaderPass* p = new ShaderPass(std::move(pass.as<ShaderPass>()));
    shader->mPasses.push_back(p);
  }

  return true;
}


template<>
owner<Shader*> Resource<Shader>::clone(S<const Shader> res) {
  return new Shader(*res);
};