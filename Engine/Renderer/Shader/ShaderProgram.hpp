#pragma once
#include "Engine/Core/common.hpp"
#include <array>
#include "ShaderStage.hpp"
#include "Engine/Renderer/UniformBuffer.hpp"
#include <unordered_map>
#include "Engine/Renderer/Shader/PropertyBlock.hpp"

class Blob;

struct shader_bind_info_t {
  std::string name;
  uint location = -1;
};

class PropertyBlockInfoBinding: public PropertyBlockInfo {
  friend class ShaderProgram;
public:
  shader_bind_info_t bindInfo;
};


class ShaderProgramInfo {
  friend class ShaderProgram;
public:
  inline shader_bind_info_t& at(eTextureSlot slot) { return mTextureInfo[slot]; };
  const PropertyBlockInfoBinding& at(const std::string& blockName) const { return mBlockInfo.at(blockName); };
  const PropertyBlockInfoBinding* find(const std::string& blockName) const;
  PropertyBlockInfoBinding& search(std::string_view propName);

  const PropertyBlockInfoBinding& operator[](std::string_view blockName) const;
  const shader_bind_info_t& operator[](eTextureSlot slot) const;

  inline void clear() { mBlockInfo.clear(); };

protected:
  PropertyBlockInfoBinding& at(std::string_view blockName);
  std::array<shader_bind_info_t, NUM_TEXTURE_SLOT> mTextureInfo;
  std::unordered_map<std::string, PropertyBlockInfoBinding> mBlockInfo;
};

class ShaderProgram {
public:
  ShaderProgram() = default;
  ~ShaderProgram() = default;
  bool fromFile(const char*  relativePath, const char* defineArgs = nullptr);
  inline uint handle() const { return mProgId; }

  inline const ShaderProgramInfo& info() const { return mInfo; }
  void genInfo();
  static uint createAndLinkProgram(uint vs, uint fs, uint handle = 0);
  static void logProgramError(uint programId);
protected:
  static void fillBlockProperty(PropertyBlockInfoBinding& block, uint progId, int index);
  std::array<ShaderStage, NUM_SHADER_TYPE> mStages;
  ShaderProgramInfo mInfo;
  uint mProgId = NULL;
};
