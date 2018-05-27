#pragma once
#include "Engine/Core/common.hpp"

class Rgba;
namespace Debug {
  void log(std::string_view text, const Rgba& color, float duration, bool toView = true, bool toConsole = true, bool toMessage = true);

}

namespace detail::Debug {
  void tick();
}