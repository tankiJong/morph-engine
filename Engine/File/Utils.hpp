#pragma once

#include "Engine/Core/common.hpp"
#include "Engine/File/Blob.hpp"
#include "Path.hpp"

class Blob;

namespace fs {
  // environment
  path workingPath();
  path absolute(const path& path, const Path& base = workingPath());
  bool exists(const path& file);
  bool isDirectory(const path& path);
  
  int64 sizeOf(const path& file);
  Blob read(const path& filePath);
  bool  read(const path& file, void* buffer, int64 size = -1);

  path relative(const path& p, const path& base);

  const path& dotPath();
  const path& dotdotPath();
}