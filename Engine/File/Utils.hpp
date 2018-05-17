#pragma once

#include "Engine/Core/common.hpp"
#include "Engine/File/Blob.hpp"
#include "Path.hpp"

class Blob;


Blob fileToBuffer(const char* nameWithFullPath);

namespace fs {
  // environment
  path workingPath();
  path absolute(const path& path, const Path& base = workingPath());
  bool exists(const path& file);
  bool isDirectory(const path& path);
  
  int64 sizeOf(const path& file);
  byte_t* read(const path& file);
  bool  read(const path& file, void* buffer, int64 size = -1);

  path relative(const path& p, const path& base);

  const path& dotPath();
  const path& dotdotPath();
}