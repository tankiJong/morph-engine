#pragma once

#include <unordered_map>
#include "Blob.hpp"
#include "Path.hpp"

class DataSource {
public:
  virtual bool isPath() = 0;
  virtual bool isUrl() = 0;

  virtual void fromString(const std::string& str) = 0;
  virtual std::string toString() = 0;
  virtual Blob blob() = 0;

  virtual ~DataSource() = default;
  inline bool cached() const { return isCached; }
protected:
  DataSource() = default;
  bool isCached = false;
};

class DataSourcePath final: public DataSource {
public:
  DataSourcePath(DataSourcePath&& dataSourcePath) noexcept;
  inline bool isPath() override { return true; };
  inline bool isUrl() override { return false; };
  void fromString(const std::string& str) override;
  std::string toString() override;
  Blob blob() override;

  static DataSourcePath& create(const Path& path);
protected:
  DataSourcePath(const Path& path);
  Path path;
  Blob buffer;


  static std::unordered_map<Path, DataSourcePath> sFileOpened;

};


//class DataSourceUrl final: public DataSource {};
