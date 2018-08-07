#include "Engine/Graphics/RHI/RHI.hpp"
#include "Engine/Graphics/RHI/Shader.hpp"
#include "Engine/File/Utils.hpp"
#include <optional>

class Dx12Include: public ID3DInclude {
public:
  std::vector<fs::path> directories;

  virtual ~Dx12Include() = default;
  Dx12Include() {
    directories.push_back(fs::workingPath());
  }

  std::optional<fs::path> resolve(const char* name) {
    for(auto iter = directories.rbegin(); iter != directories.rend(); ++iter) {
      fs::path fullpath = *iter / name;
      if(fs::exists(fullpath)) return fullpath;
    }

    return {};
  }
  HRESULT Open(D3D_INCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID* ppData, UINT* pBytes) override {
    std::optional<fs::path> fullPath = resolve(pFileName);

    if (!fullPath) return E_FAIL;

    switch(IncludeType) { 
      case D3D_INCLUDE_LOCAL: {
        size_t size;
        const char* ptr;
        bool result = fs::read(fullPath.value(), ptr, size);
        *pBytes = (UINT)size;
        *ppData = ptr;
      }
      break;
      case D3D_INCLUDE_SYSTEM:
        return E_FAIL;
      break;
    }

    directories.push_back(fullPath->parent_path());
    return S_OK;
  };

  HRESULT Close(LPCVOID pData) override {
    delete[] (const char*)pData;
    directories.pop_back();
    return S_OK;
  };
};

static Dx12Include gIncluder;

void Shader::compile() {
  if (!mSource.valid()) return;
  D3D_SHADER_MACRO* defines
    = mDefineDirectives.empty()
    ? nullptr
    : (D3D_SHADER_MACRO*)_alloca(sizeof(D3D_SHADER_MACRO) * mDefineDirectives.size());

  const char* target = nullptr;

  switch(mShaderType) {
    case SHADER_TYPE_VERTEX:
      target = "vs_5_1";
    break;
    case SHADER_TYPE_FRAGMENT:
      target = "ps_5_1";
    break;
    case SHADER_TYPE_COMPUTE:
      target = "cs_5_0";
    break;
    default: ;
  }
  D3D_SHADER_MACRO* iter = defines;
  for(auto& [k, v]: mDefineDirectives) {
    (*iter).Name = k.c_str();
    (*iter).Definition = v.c_str();
    iter++;
  }

#if defined(_DEBUG)
  // Enable better shader debugging with the graphics debugging tools.
  UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
  UINT compileFlags = 0;
#endif

  ID3DBlob* bin = nullptr;
  ID3DBlob* err = nullptr;

  HRESULT re = D3DCompile(mSource, mSource.size(), mFilePath.c_str(), defines, &gIncluder,
             mEntryPoint.c_str(), target, compileFlags, 0, &bin, &err);

  if(re != S_OK) {
    ERROR_AND_DIE((char*)err->GetBufferPointer());
  }

  mBinary.set(bin->GetBufferPointer(), bin->GetBufferSize());
}