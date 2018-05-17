#pragma once
#include "Engine/Core/common.hpp"

#if defined( _WIN64 )
  #if defined(_DEBUG)
  #pragma comment( lib, "ThirdParty/yaml-cpp/libyaml-cppmdd-x64-debug.lib" )
  #else 
  #pragma comment( lib, "ThirdParty/yaml-cpp/libyaml-cppmd-x64-release.lib" )
  #endif
#else
  #if defined(_DEBUG)
  #pragma comment( lib, "ThirdParty/yaml-cpp/libyaml-cppmdd-x86-debug.lib" )
  #else 
  #pragma comment( lib, "ThirdParty/yaml-cpp/libyaml-cppmd-x86-release.lib" )
  #endif
#endif
#pragma warning( push )
#pragma warning( disable : 4127 ) 
#include "ThirdParty/yaml-cpp/yaml.h"
#pragma warning( pop )
namespace yaml {
  using namespace YAML;
};
