#include "Shader.hpp"
#include "Engine/File/Utils.hpp"

Shader::sptr_t Shader::create(const std::string_view file, std::string_view entry, eShaderType type) {
  sptr_t shader = sptr_t(new Shader(file, entry, type));
  return shader;
}

Shader::Shader(const std::string_view file, std::string_view entry, eShaderType type) {
  setFromFile(file, entry);
  setType(type);
}

void* Shader::handle() const {
  return mBinary;
}

size_t Shader::size() const {
  return mBinary.size();
}

void Shader::setType(eShaderType type) {
  mShaderType = type;
}

void Shader::setFromFile(const std::string_view file, std::string_view entry) {
  mFilePath = file;
  mEntryPoint = entry;
  mSource = fs::read(mFilePath);
}

void Shader::setFromString(const std::string_view source, std::string_view entry) {
  mFilePath = "";
  mEntryPoint = entry;
  mSource.set(source.data(), source.size());
}

void Shader::setFromBinary(const void * data, size_t size) {
  mFilePath = "";
  mEntryPoint = "";
  mBinary.set(data, size);
}

