#include "Shader.hpp"
#include "Engine/File/FileSystem.hpp"
#include "Engine/Persistence/yaml.hpp"

Shader* fromYaml(fs::path& file) {
  auto vfs = FileSystem::Get();

  std::optional<Blob> f = vfs.asBuffer(file);

  if(!f) {
    return nullptr;
  }

  yaml::Node node = yaml::Load(f->as<const char*>());

//  node["name"].as<std::string>();
}
