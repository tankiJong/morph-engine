#include "Shader.hpp"

Shader::sptr_t Shader::create(const std::string_view file, std::string_view entry, eShaderType type) {
  sptr_t shader = sptr_t(new Shader());

  shader->mFilePath = file;
  shader->mEntryPoint = entry;
  shader->mShaderType = type;

  return shader;
}

void* Shader::handle() const {
  return mBinary;
}

size_t Shader::size() const {
  return mBinary.size();
}
