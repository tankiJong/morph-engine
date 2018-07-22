#pragma once
#include "Engine/Core/common.hpp"
#include "Engine/File/Blob.hpp"
#include <unordered_map>
#include "Engine/Graphics/RHI/RHIType.hpp"

class Shader: public std::enable_shared_from_this<Shader> {
public:
  using sptr_t = S<Shader>;
  void define(std::string key);
  void define(std::string_view key, std::string_view value);

  void compile();

  void* handle() const;
  size_t size() const;
  static sptr_t create(const std::string_view file, std::string_view entry, eShaderType type);
protected:
  std::string mFilePath;
  std::string mEntryPoint;
  eShaderType mShaderType = SHADER_TYPE_UNKNOWN;
  Blob mBinary;
  std::unordered_map<std::string, std::string> mDefineDirectives;
};
