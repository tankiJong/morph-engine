#pragma once
#include "Engine/Core/common.hpp"
#include "Engine/Core/Rgba.hpp"

namespace Debug {
  void log(std::string_view text, const Rgba& color = Rgba::white, float duration = 0.f, bool toView = true, bool toConsole = true, bool toMessage = true);

}

namespace detail::Debug {
  void tick();
}