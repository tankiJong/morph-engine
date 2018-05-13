#pragma once
#include "Engine/Core/common.hpp"
#include "Engine/File/Path.hpp"
#include "Engine/File/Blob.hpp"
#include "Engine/Core/Delegate.hpp"

class File;
using FileChangeHandler = delegate<void(File&)>;

class File {
  friend class FileSystem;
public:
  void onChange(const FileChangeHandler& handle);

  template<typename T>
  inline T* as() { return mData; }

protected:
  File(FileSystem& sys);
  FileSystem& mFileSystem;
  fs::path mPath;
  Blob mData;
};