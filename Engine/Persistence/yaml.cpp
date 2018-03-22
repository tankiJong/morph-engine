#include "Yaml.hpp"
#define _SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING
#pragma comment( lib, "ThirdParty/yaml-cpp/libyaml-cppmd.lib" )
#pragma warning( push )
#pragma warning( disable : 4127 ) 
#include "ThirdParty/yaml-cpp/yaml.h"
#pragma warning( pop )
#undef _SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING

YAML::Node node = YAML::Load("[1, 2, 3]");

