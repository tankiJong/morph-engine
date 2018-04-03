#pragma once
#include <string>
#include <tuple>
#include <vector>
#include <set>
#include "Engine/Core/common.hpp"
#include "Engine/File/Path.hpp"
#include "Engine/Renderer/type.h"
namespace std {
  template<typename T>
  class optional;
}

class ShaderStage {
public:
  ShaderStage();
  ShaderStage(eShaderType type);
  ShaderStage(eShaderType type, const char* source);
  bool setFromString(eShaderType type, std::string source);
  bool setFromFile(eShaderType type, const char* path);
  int compile();
  inline int handle() const { return mHandle; }
  inline eShaderStageState status() const { return mStatus; }
  ShaderStage& define(const std::string& k, std::string v = "");
protected:
  bool parse(std::string& source);
  std::optional<std::tuple<std::string, std::string, uint>> parseDirectives(std::string& source);
  bool parseBody(const Path& currentFile, std::string& body, uint currentLine, std::set<Path>& includedFiles);
  std::vector<std::pair<std::string, std::string>> mDefineDirectives;
  uint mVersion = 0;
  Path mPath = "Buffer";
  std::string mShaderString;
  eShaderType mType;
  int mHandle;
  eShaderStageState mStatus;
  std::string makelineDirective(uint version, uint line, const Path& filename);
  int compile(const char* shaderStr, eShaderType type);
};
