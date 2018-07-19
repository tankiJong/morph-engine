#pragma once
#include "Engine/Core/common.hpp"
#include "Engine/Core/Rgba.hpp"
#include "Engine/Core/Delegate.hpp"

namespace Log {

  struct tag_t {
    std::string name;
    Rgba color;
  };

  struct log_t {
    tag_t tag;
    std::string content;

    std::string toString() const;
  };

  using log_cb_t = delegate<void(const log_t&)>;

  using log_handle_t = void*;
  //void log(std::string_view text, const Rgba& color = Rgba::white, float duration = 0.f, bool toView = true, bool toConsole = true, bool toMessage = true);
  void log(std::string_view text, const Rgba& color = Rgba::white);
  void defineTag(const char* tag, unsigned char hue);
  void tagv(const char* tag, const char* format, va_list args);
  void tagf(const char* tag, const char* format, ...);
  void logf(const char* format, ...);
  void warnf(const char* format, ...);
  void errorf(const char* format, ...);
  void showAll();
  void hideAll();
  void show(const char* tag);
  void hide(const char* tag);
  void flush();
  log_handle_t hook(log_cb_t cb);
  void unhook(log_handle_t cb);

  void startUp();
  void shutDown();


}

#define LOG_CB_REG \
struct APPEND(__LOG_CB, __LINE__) { \
  APPEND(__LOG_CB, __LINE__)() { \
    Log::hook(exec); \
  }; \
  static void exec(const Log::log_t&); \
} APPEND(__LOG_CBIns, __LINE__); \
inline void APPEND(__LOG_CB, __LINE__)::exec