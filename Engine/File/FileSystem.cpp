#include "FileSystem.hpp"
#include "Utils.hpp"
#include "Engine/Debug/ErrorWarningAssert.hpp"

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



