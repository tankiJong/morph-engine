#pragma once

#include <filesystem>


namespace fs {
  using path = std::filesystem::path;
  using RecursiveDirectoryIterator = std::filesystem::recursive_directory_iterator;
  using DirectoryIterator = std::filesystem::directory_iterator;
}

using Path = fs::path;

namespace std {
  template<> struct hash<Path> {
    size_t operator()(const Path& p) const {
      return filesystem::hash_value(p);
    }
  };
}

//ref: http://www.boost.org/doc/libs/1_48_0/libs/filesystem/v3/doc/tutorial.html