#include "Yaml.hpp"

#pragma comment( lib, "ThirdParty/yaml-cpp/libyaml-cppmd.lib" )
#pragma warning( push )
#pragma warning( disable : 4127 ) 
#include "ThirdParty/yaml-cpp/yaml.h"
#pragma warning( pop )

YAML::Node node = YAML::Load("[1, 2, 3]");

