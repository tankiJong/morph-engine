#include "dx12util.hpp"
#include <string>
#include <locale.h> 
#include "Engine/Debug/ErrorWarningAssert.hpp"

#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d12.lib")

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
