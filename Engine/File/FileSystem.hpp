#pragma once
#include "Engine/Core/common.hpp"
#include "Path.hpp"
#include <optional>
#include "Blob.hpp"
#include <unordered_map>


class FileSystem {
public:
  FileSystem() = default;
  void mount(const fs::path& virtualDir, const fs::path& physicalPath);
  void unmount(const fs::path& virtualDir);

  std::vector<fs::path> map(const fs::path& virtualPath);

  std::optional<Blob> open(const fs::path& vpath);

protected:
  std::unordered_map<fs::path, std::vector<fs::path>> mMounted;
};
