#include "dx12util.hpp"
#include <string>
#include <clocale> 
#include "Engine/Debug/ErrorWarningAssert.hpp"

#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d12.lib")

template<typename T>
struct DxgiFormatDesc {
  T key;
  DXGI_FORMAT val;
};

DxgiFormatDesc<eTextureFormat> DxgiFormatDescMap[] = {
  { TEXTURE_FORMAT_UNKNOWN, DXGI_FORMAT_UNKNOWN },    
  { TEXTURE_FORMAT_RGBA8,   DXGI_FORMAT_R8G8B8A8_UINT },   // default color format
  { TEXTURE_FORMAT_RG8,     DXGI_FORMAT_R8G8_UINT },
  { TEXTURE_FORMAT_R8,      DXGI_FORMAT_R8_UINT },
  { TEXTURE_FORMAT_RGBA16,  DXGI_FORMAT_R16G16B16A16_FLOAT },
  { TEXTURE_FORMAT_RG16,    DXGI_FORMAT_R16G16_FLOAT },  
  { TEXTURE_FORMAT_R16,     DXGI_FORMAT_R16_FLOAT },
  { TEXTURE_FORMAT_D24S8,   DXGI_FORMAT_D24_UNORM_S8_UINT },
};

std::wstring make_wstring(const std::string& str) {
  setlocale(LC_CTYPE, "");
  wchar_t p[1024];
  size_t len;
  mbstowcs_s(&len, p, str.data(), 1024);
  std::wstring str1(p);
  return str1;
}

DXGI_FORMAT dxgi_float(uint size) {
  switch(size) {
    case 1:
      return DXGI_FORMAT_R32_FLOAT;
    case 2:
      return DXGI_FORMAT_R32G32_FLOAT;
    case 3:
      return DXGI_FORMAT_R32G32B32_FLOAT;
    case 4:
      return DXGI_FORMAT_R32G32B32A32_FLOAT;
  };
  ERROR_AND_DIE("unimplemented");
}

DXGI_FORMAT dxgi_uchar(uint size, bool normalized) {
  if(normalized) {
    switch (size) {
      case 1:
        return DXGI_FORMAT_R8_UINT;
      case 2:
        return DXGI_FORMAT_R8G8_UINT;
      case 4:
        return DXGI_FORMAT_R8G8B8A8_UINT;
    };
  } else {
    switch (size) {
      case 1:
        return DXGI_FORMAT_R8_UNORM;
      case 2:
        return DXGI_FORMAT_R8G8_UNORM;
      case 4:
        return DXGI_FORMAT_R8G8B8A8_UNORM;
    };
  }
  ERROR_AND_DIE("unimplemented");
}

DXGI_FORMAT dxgi_char(uint size, bool normalized) {
  if (normalized) {
    switch (size) {
      case 1:
        return DXGI_FORMAT_R8_SINT;
      case 2:
        return DXGI_FORMAT_R8G8_SINT;
      case 4:
        return DXGI_FORMAT_R8G8B8A8_SINT;
    };
  } else {
    switch (size) {
      case 1:
        return DXGI_FORMAT_R8_SNORM;
      case 2:
        return DXGI_FORMAT_R8G8_SNORM;
      case 4:
        return DXGI_FORMAT_R8G8B8A8_SNORM;
    };
  }
  ERROR_AND_DIE("unimplemented");
}

DXGI_FORMAT toDXGIFormat(eDataDeclType declType, uint count, bool normalized) {
  switch(declType) {
    case MP_FLOAT:
      return dxgi_float(count);
    case MP_BYTE:
      return dxgi_char(count, normalized);
    case MP_UBYTE:
      return dxgi_uchar(count, normalized);
  }
  ERROR_AND_DIE("unsupported type");
}

DXGI_FORMAT toDXGIFormat(eTextureFormat format) {
  EXPECTS(DxgiFormatDescMap[format].key == format);
  return DxgiFormatDescMap[format].val;
}
