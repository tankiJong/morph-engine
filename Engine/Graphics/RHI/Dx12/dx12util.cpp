#include "dx12util.hpp"
#include <string>
#include <clocale> 
#include "Engine/Debug/ErrorWarningAssert.hpp"

#if defined( MORPH_DXR )
#if defined(_DEBUG)
#pragma comment( lib, "ThirdParty/d3d12/FallbackLayer/FallbackLayer_debug.lib" )
#else 
#pragma comment( lib, "ThirdParty/d3d12/FallbackLayer/FallbackLayer_release.lib" )
#endif
#endif

#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d12.lib")

template<typename T>
struct DxgiFormatDesc {
  T key;
  DXGI_FORMAT val;
};

DxgiFormatDesc<eTextureFormat> DxgiFormatDescMap[] = {
  { TEXTURE_FORMAT_UNKNOWN,       DXGI_FORMAT_UNKNOWN },    
  { TEXTURE_FORMAT_RGBA8,         DXGI_FORMAT_R8G8B8A8_UNORM },   // default color format
  { TEXTURE_FORMAT_RG8,           DXGI_FORMAT_R8G8_UNORM },
  { TEXTURE_FORMAT_R8,            DXGI_FORMAT_R8_UNORM },
  { TEXTURE_FORMAT_R8_UINT,       DXGI_FORMAT_R8_UINT },

  { TEXTURE_FORMAT_RGBA16,        DXGI_FORMAT_R16G16B16A16_FLOAT },
  { TEXTURE_FORMAT_RG16,          DXGI_FORMAT_R16G16_FLOAT },  
  { TEXTURE_FORMAT_R16,           DXGI_FORMAT_R16_FLOAT },
  { TEXTURE_FORMAT_R16_UINT,      DXGI_FORMAT_R16_UINT },

  { TEXTURE_FORMAT_R32_UINT,      DXGI_FORMAT_R32_UINT },

  { TEXTURE_FORMAT_D24S8,         DXGI_FORMAT_R24G8_TYPELESS },
  { TEXTURE_FORMAT_D32,           DXGI_FORMAT_R32_FLOAT },
};


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

size_t DXGIFormatSize(DXGI_FORMAT fmt) {
  switch(fmt) {
    case DXGI_FORMAT_UNKNOWN: return 0;    
    case DXGI_FORMAT_R8G8B8A8_UNORM: return 32;   // default color format
    case DXGI_FORMAT_R8G8_UNORM: return 16;
    case DXGI_FORMAT_R8_UNORM: return 8;
    case DXGI_FORMAT_R8_UINT: return 8;
    case DXGI_FORMAT_R16G16B16A16_FLOAT: return 64;
    case DXGI_FORMAT_R16G16_FLOAT: return 32;  
    case DXGI_FORMAT_R16_FLOAT: return 16;
    case DXGI_FORMAT_R16_UINT: return 16;
    case DXGI_FORMAT_R32_UINT: return 32;
    case DXGI_FORMAT_R24G8_TYPELESS: return 32;
	  case DXGI_FORMAT_R32_FLOAT: return 32;
  }
  BAD_CODE_PATH();
}

eTextureFormat toTextureFormat(DXGI_FORMAT format) {
  for(DxgiFormatDesc<eTextureFormat>& entry: DxgiFormatDescMap) {
    if (entry.val == format) return entry.key;
  }
  return TEXTURE_FORMAT_UNKNOWN;
}
