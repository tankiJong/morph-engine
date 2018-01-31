#pragma once
#include <filesystem>

namespace fs = std::experimental::filesystem;
typedef fs::path Path;

namespace std {
  template<> struct hash<Path> {
    size_t operator()(const Path& p) const {
      return fs::hash_value(p);
    }
  };
}
