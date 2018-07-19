#pragma once
#include "Engine/Core/common.hpp"
#include "Engine/File/Path.hpp"
#include "Engine/File/Blob.hpp"

class File;

class File {
  friend class FileSystem;
public:

  template<typename T>
  inline T* as() { return mData; }

protected:
  fs::path mPath;
  Blob mData;
};