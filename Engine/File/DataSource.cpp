#include "DataSource.hpp"
#include "FileUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

std::unordered_map<Path, DataSourcePath> DataSourcePath::sFileOpened = {};
DataSourcePath::DataSourcePath(DataSourcePath&& source) noexcept
  : path(std::move(source.path))
  , buffer(std::move(source.buffer)){
}

void DataSourcePath::fromString(const std::string& str) {
  path = str;
}

std::string DataSourcePath::toString() {
  return path.string();
}

Blob DataSourcePath::blob() {
  if(!isCached) {
    buffer = fileToBuffer(absolute(path).string().c_str());
    isCached = true;
  }

  return buffer.clone();
}

DataSourcePath& DataSourcePath::create(const Path& path) {
  auto it = sFileOpened.find(path);
  if(it == sFileOpened.end()) {
    bool result = false;
    std::tie(it, result) = sFileOpened.emplace(path, DataSourcePath(path));
    ENSURES(result);
  }

  return it->second;
}

DataSourcePath::DataSourcePath(const Path& path): path(path) {}
