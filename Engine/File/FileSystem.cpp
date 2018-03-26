﻿#include "FileSystem.hpp"
#include "Utils.hpp"
#include "Engine/Debug/ErrorWarningAssert.hpp"
#include <fstream>

void FileSystem::mount(const fs::path& virtualDir, const fs::path& physicalPath) {
  Expects(fs::isDirectory(physicalPath));


  fs::path fullPath = fs::absolute(physicalPath);

  auto& paths = mMounted[virtualDir];

  for(fs::path& p: paths) {
    if(fullPath == p) {
      ERROR_RECOVERABLE("try to mount an existing path to the file system");
      return;
    }
  }

  paths.push_back(fullPath);
}

void FileSystem::unmount(const fs::path& virtualDir) {
  mMounted[virtualDir].clear();
}

std::vector<fs::path> FileSystem::map(const fs::path& virtualPath) {
  auto it = virtualPath.begin();
  TODO("case where vpath is not root_directory");
  fs::path p, rest;
  p /= *(it++);
  p /= *(it++);

  for(; it != virtualPath.end(); ++it) {
    rest /= *it;
  }

  // vpath can be /mnt/ or /mnt 
  if (rest.has_root_path()) {// in this case, vpath is like: /mnt/xxxxx, so now rest is /xxxx
    rest = rest.relative_path(); 
  }

  // rest is now start without /
  for (; it != virtualPath.end(); ++it) {
    rest /= *it;
  };
  if (auto mounted = mMounted.find(p); mounted == mMounted.end()) {
    return {};
  } else {
    std::vector<fs::path> paths = mounted->second;
    for(fs::path& path: paths) {
      path = fs::absolute(rest, path);
    }
    
    return paths;
  }
}

std::optional<Blob> FileSystem::asBuffer(const fs::path& file) {
  auto paths = map(file);
  uint i = 0;
  for(uint size = paths.size(); i<size; ++i) {
    if (fs::exists(paths[i])) break;
  }

  if (i == paths.size()) return {};

  std::ifstream f(paths[i], std::ios::binary | std::ios::ate);

  std::streamsize size = f.tellg();

  if (size == -1) {
    return {};
  }
  f.seekg(0, std::ios::beg);

  char* buffer = new char[(uint)size + 1];

  if (f.read(buffer, size)) {
    buffer[size] = 0;
    return Blob(buffer, (uint)size + 1);
  } else {
    return {};
  }
}

static FileSystem* gInstance = nullptr;
FileSystem& FileSystem::Get() {
  if(!gInstance) {
    gInstance = new FileSystem();
  }

  return *gInstance;
}


