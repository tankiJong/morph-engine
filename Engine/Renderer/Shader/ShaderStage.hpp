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
  ~ShaderStage();
  bool setFromString(eShaderType type, std::string source);
  bool setFromFile(eShaderType type, const char* path);
  bool reload();
  int compile();
  inline uint handle() const { return mHandle; }
  inline eShaderStageState status() const { return mStatus; }
  ShaderStage& define(const std::string& k, std::string v = "");
protected:
  void init();
  bool parse(std::string& source);
  bool parseBody(const Path& currentFile, std::string& body, uint currentLine, std::set<Path>& includedFiles);
  std::optional<std::tuple<std::string, std::string, uint>> parseDirectives(std::string& source);
  std::string makelineDirective(uint version, uint line, const Path& filename);
  int compile(const char* shaderStr, eShaderType type);
  uint mVersion = 0;
  Path mPath = "Buffer";
  bool mFromFile = false;
  std::string mShaderString;
  eShaderType mType;
  uint mHandle;
  eShaderStageState mStatus;
  std::vector<std::pair<std::string, std::string>> mDefineDirectives;
  std::vector<fs::path> mIncludeDirectory;

};
