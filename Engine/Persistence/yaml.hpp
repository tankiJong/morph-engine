#pragma once

#pragma comment( lib, "ThirdParty/yaml-cpp/libyaml-cppmd.lib" )
#pragma warning( push )
#pragma warning( disable : 4127 ) 
#include "ThirdParty/yaml-cpp/yaml.h"
#pragma warning( pop )
namespace yaml {
  using namespace YAML;
};
