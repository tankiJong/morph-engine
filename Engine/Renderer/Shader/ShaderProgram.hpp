#pragma once
#include "Engine/Core/common.hpp"
#include <array>
#include "ShaderStage.hpp"
#include "Engine/Renderer/UniformBuffer.hpp"
#include <unordered_map>

class Blob;
class PropertyBlockInfo;

struct shader_bind_info_t {
  std::string name;
  uint location = -1;
};

struct property_info_t {
  PropertyBlockInfo* owningBlock;
  std::string name;
  size_t offset;
  size_t size;
  size_t count;
};

class PropertyBlockInfo {
  friend class ShaderProgram;
public:
  const property_info_t* operator()(std::string_view name) const;
  const property_info_t* operator()(std::string_view name, uint index) const;

  static const PropertyBlockInfo invalid;
  shader_bind_info_t bindInfo;
  size_t totalSize = 0;
protected:
  // would protentially cause an insert if the entry does not exist.
  inline property_info_t& operator[](const std::string& name) { return mInfos[name]; };
  std::unordered_map<std::string, property_info_t> mInfos;
};

class PropertyBlock: public UniformBuffer {
public:
  const PropertyBlockInfo* info;
};

class ShaderProgramInfo {
  friend class ShaderProgram;
public:
  inline shader_bind_info_t& at(eTextureSlot slot) { return mTextureInfo[slot]; };
  const PropertyBlockInfo& at(const std::string& blockName) const { return mBlockInfo.at(blockName); };
  PropertyBlockInfo& search(std::string_view propName);

  const PropertyBlockInfo& operator[](std::string_view blockName) const;
  const shader_bind_info_t& operator[](eTextureSlot slot) const;

  inline void clear() { mBlockInfo.clear(); };

protected:
  PropertyBlockInfo& at(std::string_view blockName);
  std::array<shader_bind_info_t, NUM_TEXTURE_SLOT> mTextureInfo;
  std::unordered_map<std::string, PropertyBlockInfo> mBlockInfo;
};

class ShaderProgram {
public:
  ShaderProgram() = default;
  ~ShaderProgram() = default;
  bool fromFile(const char*  relativePath, const char* defineArgs = nullptr);
  inline uint handle() const { return mProgId; }
  inline const ShaderProgramInfo& info() const { return mInfo; };
  void genInfo();
  static uint createAndLinkProgram(uint vs, uint fs, uint handle = 0);
  static void logProgramError(uint programId);
protected:
  static void fillBlockProperty(PropertyBlockInfo& block, uint progId, int index);
  std::array<ShaderStage, NUM_SHADER_TYPE> mStages;
  ShaderProgramInfo mInfo;
  uint mProgId = NULL;
};
