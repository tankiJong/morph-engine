#pragma once
#include "Engine/Core/common.hpp"
#include "Path.hpp"
#include <optional>
#include "Blob.hpp"
#include <unordered_map>
#include "Engine/File/File.hpp"

class File;

class FileSystem {
public:
  void mount(const fs::path& virtualDir, const fs::path& physicalPath);
  void unmount(const fs::path& virtualDir);

  std::vector<fs::path> map(const fs::path& virtualPath);
  std::optional<File> open(const fs::path& vPath);
  std::optional<fs::path> locate(const fs::path& vPath);
  std::optional<Blob> asBuffer(const fs::path& file);
  std::ifstream asStream(const fs::path& file);

  static FileSystem& Get();

protected:
  FileSystem() = default;
  std::unordered_map<fs::path, std::vector<fs::path>> mMounted;
};
