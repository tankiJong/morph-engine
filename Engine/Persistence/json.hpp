#pragma once
#include "Engine/Core/common.hpp"
#define JSON_HAS_CPP_17

#pragma warning( push )
#pragma warning( disable : 4348 ) 
#include "ThirdParty/nlohmann-json/json.hpp"
#pragma warning( pop )

using json = nlohmann::json;