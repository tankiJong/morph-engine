#include "Utils.hpp"
#include <cstdlib>
#include <fstream>
#include "Path.hpp"
#include <filesystem>
namespace sfs {
  using namespace std::experimental::filesystem;
}

fs::path fs::workingPath() {
  return sfs::current_path();
}

fs::path fs::absolute(const path& path, const Path& base) {
  return sfs::absolute(path, base);
}

bool fs::exists(const path& file) {
  return sfs::exists(file);
}

bool fs::isDirectory(const path& path) {
  return is_directory(path);
}

Blob fs::read(const path& filePath) {
  std::ifstream file(filePath.c_str(), std::ios::binary | std::ios::ate);

  std::streamsize size = file.tellg();

  if (size == -1) {
    return Blob();
  }
  file.seekg(0, std::ios::beg);

  char* buffer = new char[(uint)size + 1];

  if (file.read(buffer, size)) {
    buffer[size] = 0;
    Blob b(buffer, (uint)size + 1);
    delete[] buffer;
    return b;
  } else {
    return Blob();
  }
}

// reference from Boost filesystem
fs::path fs::relative(const path& p, const path& base) {
  std::pair<path::iterator, path::iterator> mm
    = std::mismatch(p.begin(), p.end(), base.begin(), base.end());

  if (mm.first == p.begin() && mm.second == base.begin())
    return base.root_directory();

  if (mm.first == p.end() && mm.second == base.end())
    return fs::dotPath();

  path tmp;
  for (; mm.second != base.end(); ++mm.second)
    tmp /= fs::dotdotPath();
  for (; mm.first != p.end(); ++mm.first)
    tmp /= *mm.first;
  return tmp;
}

const fs::path& fs::dotPath() {
  static fs::path dot = ".";

  return dot;
}

const fs::path& fs::dotdotPath() {
  static const fs::path dotdot("..");

  return dotdot;
}
