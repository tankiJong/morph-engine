#pragma once
#include <unordered_map>
#include "Engine/Core/common.hpp"
#include "Engine/File/Blob.hpp"
#include "Engine/Graphics/RHI/RHIType.hpp"
#include "Engine/Graphics/RHI/RootSignature.hpp"

class Shader: public std::enable_shared_from_this<Shader> {
  friend class Program;
public:
  using sptr_t = S<Shader>;
  void define(std::string key);
  void define(std::string_view key, std::string_view value);

  bool isReady() const { return mBinary.valid(); }
  void setType(eShaderType type);
  void setFromFile(const std::string_view file, std::string_view entry);
  void setFromString(const std::string_view source, std::string_view entry);
  void Shader::setFromBinary(const void* data, size_t size);
  void compile();
  S<const RootSignature> rootSignature() const;
  void* handle() const;
  size_t size() const;
  static sptr_t create(const std::string_view file, std::string_view entry, eShaderType type);

  Shader(const std::string_view file, std::string_view entry, eShaderType type);
  Shader() = default;

  bool empty() const;
  bool ready() const;


  span<const DescriptorSet::Layout> descriptorLayouts() const;

protected:
  void reflect() const;

  std::string mFilePath;
  Blob mSource;
  std::string mEntryPoint;
  eShaderType mShaderType = SHADER_TYPE_UNKNOWN;
  Blob mBinary;
  std::unordered_map<std::string, std::string> mDefineDirectives;
  mutable RootSignature::sptr_t mRootSignature;
  mutable std::vector<DescriptorSet::Layout> mLayouts;
};
