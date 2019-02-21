#include "FileSystem.hpp"
#include "Utils.hpp"
#include "Engine/Debug/ErrorWarningAssert.hpp"
#include <fstream>
#include "Engine/Core/Engine.hpp"

bool FileSystem::mount(const fs::path& virtualDir, const fs::path& physicalPath) {
  EXPECTS(fs::isDirectory(physicalPath));


  fs::path fullPath = fs::absolute(physicalPath);

  auto& paths = mMounted[virtualDir];

  for(fs::path& p: paths) {
    if(fullPath == p) {
      return false;
    }
  }

  paths.push_back(fullPath);

  return true;
}

void FileSystem::unmount(const fs::path& virtualDir) {
  mMounted[virtualDir].clear();
}

std::vector<fs::path> FileSystem::map(const fs::path& virtualPath) const {
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

std::optional<fs::path> FileSystem::locate(const fs::path& vFilePath) const {
  EXPECTS(!fs::isDirectory(vFilePath));
  auto paths = map(vFilePath);
  uint i = 0;
  for (size_t size = paths.size(); i<size; ++i) {
    if (fs::exists(paths[i])) return paths[i];
  }

  return {};
}

std::optional<Blob> FileSystem::asBuffer(const fs::path& file) {
  auto paths = map(file);
  uint i = 0;
  for(size_t size = paths.size(); i<size; ++i) {
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

std::ifstream FileSystem::asStream(const fs::path& file) {
  auto paths = map(file);
  uint i = 0;
  for (size_t size = paths.size(); i<size; ++i) {
    if (fs::exists(paths[i])) break;
  }

  if (i == paths.size()) return {};

  std::ifstream f;
  f.open( paths[i]);
  return f;
}

void FileSystem::foreach(fs::path vpath, const delegate<void(const fs::path&, const FileSystem&)>& handler, bool recursive) {

  auto physicalPaths = map(vpath);
  if (physicalPaths.size() == 0) return;

  EXPECTS(fs::isDirectory(physicalPaths[0]));
  for (auto& path : physicalPaths) {
    if (recursive) {
      for (auto& p : fs::RecursiveDirectoryIterator(path)) {
        fs::path vFilePath = vpath / fs::relative(p.path(), path);
        handler(vFilePath, *this);
      }
    } else {
      for (auto& p : fs::DirectoryIterator(path)) {
        fs::path rela = fs::relative(p.path(), path);
        fs::path vFilePath = vpath / rela;

        handler(vFilePath, *this);
      }
    }
  }

}

static FileSystem* gInstance = nullptr;
FileSystem& FileSystem::Get() {
  EXPECTS(Engine::Get().ready());
  if(!gInstance) {
    gInstance = new FileSystem();
  }

  return *gInstance;
}



