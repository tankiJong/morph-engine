#pragma once
#include "Engine/Core/common.hpp"
#include "Path.hpp"
#include <optional>
#include "Blob.hpp"
#include <unordered_map>
#include "Engine/File/File.hpp"
#include "Engine/Core/Delegate.hpp"

class File;

class FileSystem {
public:
  void mount(const fs::path& virtualDir, const fs::path& physicalPath);
  void unmount(const fs::path& virtualDir);

  std::vector<fs::path> map(const fs::path& virtualPath) const;
  void save(const fs::path vPath, const void* data, size_t size);
  std::optional<fs::path> locate (const fs::path& vPath) const;
  std::optional<Blob> asBuffer(const fs::path& file);
  std::ifstream asStream(const fs::path& file);
  void foreach(fs::path vpath, const delegate<void(const fs::path&, const FileSystem&)>& handler, bool recursive = true);

  void config(const fs::path& configFile, bool reset = false);

  static FileSystem& Get();

protected:
  FileSystem() = default;
  std::unordered_map<fs::path, std::vector<fs::path>> mMounted;
};
