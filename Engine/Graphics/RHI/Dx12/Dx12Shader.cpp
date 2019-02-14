#include "Engine/Graphics/RHI/RHI.hpp"
#include "Engine/Graphics/RHI/Shader.hpp"
#include "Engine/File/Utils.hpp"
#include <optional>
#include "Engine/Graphics/RHI/Dx12/Dx12RootSignature.hpp"

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
  HRESULT Open(D3D_INCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID /*pParentData*/, LPCVOID* ppData, UINT* pBytes) override {
    std::optional<fs::path> fullPath = resolve(pFileName);

    if (!fullPath) return E_FAIL;

    switch(IncludeType) { 
      case D3D_INCLUDE_LOCAL: {
        size_t size;
        const char* ptr;
        fs::read(fullPath.value(), ptr, size);
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
      target = "cs_5_1";
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

  reflect();
}

void Shader::reflect() const {

  // shader reflection to get the resource information
  // ID3D12ShaderReflectionPtr reflector = nullptr;
  // D3DReflect(mBinary, mBinary.size(), IID_PPV_ARGS(&reflector));
  //
  // D3D12_SHADER_DESC desc;
  // reflector->GetDesc(&desc);
  //
  // // to build the layout, I care resources binding: T, B, U
  // uint resBinded = desc.BoundResources;
  //
  // for(uint i = 0; i < resBinded; i++) {
  //
  //   D3D12_SHADER_INPUT_BIND_DESC ddesc;
  //   reflector->GetResourceBindingDesc(i, &ddesc);
  //   const char* name = ddesc.Name;
  // }

  auto sig = rootSignature();

  if (!sig) return;

  MAKE_SMART_COM_PTR(ID3D12VersionedRootSignatureDeserializer);

  ID3D12VersionedRootSignatureDeserializerPtr deserializer;
  D3D12CreateVersionedRootSignatureDeserializer(sig->data(), sig->data().size(), IID_PPV_ARGS(&deserializer));

  const D3D12_VERSIONED_ROOT_SIGNATURE_DESC* desc;
  deserializer->GetRootSignatureDescAtVersion(D3D_ROOT_SIGNATURE_VERSION_1_0, &desc);

  mLayouts.clear();

  for(uint i = 0; i < desc->Desc_1_0.NumParameters; i ++) {

    const D3D12_ROOT_PARAMETER& param = desc->Desc_1_0.pParameters[i];
    ENSURES(param.ParameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE);

    ShaderVisibility vb = asShaderVisibility(param.ShaderVisibility);
    auto& layout = mLayouts.emplace_back(vb);

    for(uint j = 0; j < param.DescriptorTable.NumDescriptorRanges; j++) {

      const D3D12_DESCRIPTOR_RANGE& range = param.DescriptorTable.pDescriptorRanges[j];

      DescriptorSet::Type type = asRangeType(range.RangeType);
      layout.addRange(type, range.BaseShaderRegister, range.NumDescriptors, range.RegisterSpace);
    }
  }
}

span<const DescriptorSet::Layout> Shader::descriptorLayouts() const {
  if (mLayouts.empty()) reflect();
  return mLayouts;
}

S<const RootSignature> Shader::rootSignature() const {
  EXPECTS(mBinary.size() > 0);
  if(mRootSignature == nullptr) {
    ID3DBlob* rootBlob;
    HRESULT re = D3DGetBlobPart(
      mBinary,
      mBinary.size(),
      D3D_BLOB_ROOT_SIGNATURE, 0, &rootBlob);

    if (re >= 0) {
      Blob b(rootBlob->GetBufferPointer(), rootBlob->GetBufferSize());
      mRootSignature = RootSignature::create(b);
    }

  }
  return mRootSignature;
}
