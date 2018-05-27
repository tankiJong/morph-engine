#include "Log.hpp"
#include "Engine/Core/Rgba.hpp"
#include "Engine/Math/Primitives/vec2.hpp"
#include <vector>
#include "Engine/Debug/Draw.hpp"
#include "Engine/Renderer/Font.hpp"
#include "Engine/Debug/Console/Console.hpp"
#include "Engine/Core/Time/Clock.hpp"

constexpr float VIEW_FONT_SIZE = 16.f;

const float PADDING = 4.f;
struct log_info_t {

  Rgba color;
  float duration;

  bool toConsole = true;
  bool toView = true;
  bool toMessage = true; // unimplemented yet
  std::string content;
};

std::vector<log_info_t> gLogInfos;

void Debug::log(std::string_view text, const Rgba& color, float duration, bool toView, bool toConsole, bool toMessage) {
  gLogInfos.emplace_back();
  log_info_t& log = gLogInfos.back();

  log.content = text;
  log.color = color;
  log.duration = duration;
  log.toConsole = toConsole;
  log.toView = toView;
  log.toMessage = toMessage;
}

void commitLogs() {
  vec2 start(PADDING);
  S<const Font> font = Font::Default();
  float lineStep = font->lineHeight(VIEW_FONT_SIZE) + PADDING;
  for(log_info_t& log: gLogInfos) {
    if(log.toView) {
      Debug::drawText2(log.content, VIEW_FONT_SIZE, start, 0, log.color, font.get());
    }

    if(log.toConsole) {
      Console::log(log.content, Console::DEBUG_INFO);
      log.toConsole = false;
    }
    start.y += lineStep;
  }
}

void detail::Debug::tick() {

  std::sort(gLogInfos.begin(), gLogInfos.end(), [](const log_info_t& a, log_info_t& b) {
    return a.duration < b.duration;
  });

  commitLogs();

  float frameSec = GetMainClock().frame.second;
  for(uint i = (uint)gLogInfos.size() - 1, size = i + 1; i < size; i--) {

    log_info_t& log = gLogInfos[i];
    log.duration -= frameSec;

    if(log.duration < 0) {
      log = std::move(gLogInfos.back());
      gLogInfos.pop_back();
    }
  }
}



