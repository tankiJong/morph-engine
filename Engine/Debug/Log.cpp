#include "Log.hpp"
#include "Engine/Debug/Draw.hpp"
#include "Engine/Debug/Console/Console.hpp"
#include <queue>
#include <mutex>
#include "Engine/Async/Thread.hpp"
#include <cstdarg>
#include "Engine/Debug/Console/Command.hpp"
#include <iostream>
#include "Engine/File/File.hpp"
#include <time.h>
#include "Engine/File/Utils.hpp"
#include <fstream>
#include <atomic>
#include "Engine/Core/Time/Time.hpp"
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

constexpr float VIEW_FONT_SIZE = 16.f;

static const float PADDING = 4.f;


class LogFileOutput {
public:
  static constexpr uint MAX_FILE_LINE = 128;
  void out(const std::string& line);
  void flush();
  LogFileOutput() {
    time_t timep;
    time(&timep);
    tm t;
    localtime_s(&t, &timep);

    fs::createDir("Log");
    std::string filename = "Log/debug.log";
    std::string filenameStamped = Stringf("Log/debug.%s.log", Timestamp().toString().c_str());

    file.open(filename, std::ofstream::out | std::ofstream::trunc);
    file << "";
    file.close();

    file.open(filename, std::ofstream::out | std::ofstream::app);
    fileStamped.open(filenameStamped, std::ofstream::out | std::ofstream::app);

  }

  ~LogFileOutput() {
    flush();
    file.close();
    fileStamped.close();
  }

  std::ofstream file;
  std::ofstream fileStamped;
protected:
  std::mutex mCacheLock;
  std::array<std::string, MAX_FILE_LINE> mCaches;
  std::atomic<uint> mIndex = 0;
  std::atomic<bool> mFull = false;
};

static LogFileOutput* gFileOutput;

namespace Log {
  class LogBuffer {
  public:
    void enqueue(const log_t& log) {

      std::scoped_lock lock(mLock);
      mPendingList.push(log);
    }

    bool empty() const {

      std::scoped_lock lock(mLock);
      return mPendingList.empty();
    }

    bool dequeue(log_t& log) {

      std::scoped_lock lock(mLock);

      if (mPendingList.empty()) return false;

      log = mPendingList.front();
      mPendingList.pop();
      
      return true;
    }

  protected:
    std::queue<log_t> mPendingList;
    mutable std::mutex mLock;
  };

  class Logger {
  public:
    Logger();
    bool isRunning() { return mIsRunning; };
    void stop();
    void defineTag(tag_t tag);
    void tagv(const char* tag, const char* format, va_list args);
    void tagf(const char* tag, const char* format, ...);
    void logf(const char* format, ...);
    void showAll();
    void hideAll();
    void show(const char* tag);
    void hide(const char* tag);

    log_handle_t hook(log_cb_t cb);
    void unhook(log_handle_t cb);

    void flush();
    bool isFlushing() { return mIsFlushing; }
    void setFlushFlag() { mIsFlushing = true; }
    Thread* workingThread;
  protected:
    tag_t searchTag(const char* tag);
    LogBuffer mBuffer;
    std::vector<log_cb_t> mLogCallbacks;
    bool mIsRunning = true;
    std::unordered_map<std::string, bool> mHiddenTags;
    std::unordered_map<std::string, tag_t> mTags;
    bool mHiddenAll = false;
    bool mIsFlushing = false;
    // mainly for flush, where it's possible to pull out thing from the buffer, but still have not go through the cb, while another thread call flush
  };

  Logger::Logger() {
    defineTag({ "log",  Rgba(0,255,255) });
    defineTag({ "warn",  Rgba(255, 255, 0) });
    defineTag({ "error",  Rgba(255, 0, 0) });
  }

  void Logger::stop() {
    mIsRunning = false;
  }

  void Logger::defineTag(tag_t tag) {
    mTags[tag.name] = std::move(tag);
  }

  void Logger::tagv(const char* tag, const char* format, va_list args) {
    log_t log;

    if(mHiddenTags.find(tag) == mHiddenTags.end()) {
      mHiddenTags[tag] = mHiddenAll;
    }

    log.tag = searchTag(tag);
    log.content = Stringv(format, args);

    mBuffer.enqueue(log);
  }

  void Logger::tagf(const char* tag, const char* format, ...) {
    va_list args;
    va_start(args, format);
    tagv(tag, format, args);
    va_end(args);
  }

  void Logger::logf(const char* format, ...) {
    va_list args;
    va_start(args, format);
    tagv("log", format, args);
    va_end(args);
  }

  void Logger::showAll() {
    mHiddenAll = false;
    for(auto& tag: mHiddenTags) {
      tag.second = false;
    }
  }

  void Logger::hideAll() {
    mHiddenAll = true;
    for (auto& tag : mHiddenTags) {
      tag.second = true;
    }
  }

  void Logger::show(const char* tag) {
    mHiddenTags[tag] = false;
  }

  void Logger::hide(const char* tag) {
    mHiddenTags[tag] = true;
  }

  log_handle_t Logger::hook(log_cb_t cb) {
    mLogCallbacks.emplace_back(std::move(cb));
    return &mLogCallbacks.back();
  }

  void Logger::unhook(log_handle_t cb) {
    log_cb_t* handle = (log_cb_t*)cb;
    for(size_t i = mLogCallbacks.size() - 1; i < mLogCallbacks.size(); --i) {
      if (&mLogCallbacks[i] != handle) continue;

      std::swap(mLogCallbacks[i], mLogCallbacks.back());
      mLogCallbacks.pop_back();
      break;
    }
  }

  void Logger::flush() {
    mIsFlushing = true;
    log_t log;
    while(mBuffer.dequeue(log)) {
      auto iter = mHiddenTags.find(log.tag.name);
      EXPECTS(iter != mHiddenTags.end());
      if(!iter->second) {
        for(auto& cb: mLogCallbacks) {
          cb(log);
        }
      }
    }
    gFileOutput->flush();
    mIsFlushing = false;
  }

  tag_t Logger::searchTag(const char* tag) {
    auto iter = mTags.find(tag);
    if (iter != mTags.end()) return iter->second;

    return { tag, Rgba::white };
  }

  static Log::Logger* gLogger = nullptr;

  void worker() {
    while(gLogger->isRunning()) {
      gLogger->flush();
      CurrentThread::yield();
    }

    gLogger->flush();
  }

  std::string log_t::toString() const {
    return Stringf("[%s] [%s]: %s\n", Timestamp().toString().c_str(), tag.name.c_str(), content.c_str());
  }

  void log(std::string_view text, const Rgba& /*color*/) {
    gLogger->logf(text.data());
  }

  //void log(std::string_view text, const Rgba& color, float duration, bool toView, bool toConsole, bool toMessage) {
  //  
  //}

  void defineTag(const char* tag, unsigned char hue) {
    gLogger->defineTag({ tag, Hue(hue) });
  }

  void tagv(const char* tag, const char* format, va_list args) {
    gLogger->tagv(tag, format, args);
  }

  void tagf(const char* tag, const char* format, ...) {
    va_list args;
    va_start(args, format);
    gLogger->tagv(tag, format, args);
    va_end(args);
  }

  void logf(const char* format, ...) {
    va_list args;
    va_start(args, format);
    gLogger->tagv("log", format, args);
    va_end(args);
  }

  void warnf(const char* format, ...) {
    va_list args;
    va_start(args, format);
    gLogger->tagv("warning", format, args);
    va_end(args);
  }

  void errorf(const char* format, ...) {
    va_list args;
    va_start(args, format);
    gLogger->tagv("error", format, args);
    va_end(args);
  }

  void showAll() {
    gLogger->showAll();
  }

  void hideAll() {
    gLogger->hideAll();
  }

  void show(const char* tag) {
    gLogger->show(tag);
  }

  void hide(const char* tag) {
    gLogger->hide(tag);
  }

  void flush() {
    gLogger->setFlushFlag();
    while (gLogger->isFlushing());
  }

  log_handle_t hook(log_cb_t cb) {
    if(gLogger == nullptr) {
      startUp();
    }

    return gLogger->hook(cb);
  }

  void unhook(log_handle_t cb) {
    EXPECTS(gLogger != nullptr);
    gLogger->unhook(cb);
  }

  void startUp() {
    if (gLogger != nullptr) return;
    gLogger = new Logger();
    gFileOutput = new LogFileOutput();
    gLogger->workingThread = new Thread(worker);
  }

  void shutDown() {
    gLogger->stop();
    gLogger->workingThread->join();
    SAFE_DELETE(gLogger->workingThread);
  }

}

//
//
//static std::vector<log_info_t> gLogInfos;
//
//void Debug::log(std::string_view text, const Rgba& color, float duration, bool toView, bool toConsole, bool toMessage) {
//  gLogInfos.emplace_back();
//  log_info_t& log = gLogInfos.back();
//
//  log.content = text;
//  log.color = color;
//  log.duration = duration;
//  log.toConsole = toConsole;
//  log.toView = toView;
//  log.toMessage = toMessage;
//}
//
//void commitLogs() {
//  vec2 start(PADDING);
//  S<const Font> font = Font::Default();
//  float lineStep = font->lineHeight(VIEW_FONT_SIZE) + PADDING;
//  for(log_info_t& log: gLogInfos) {
//    if(log.toView) {
//      Debug::drawText2(log.content, VIEW_FONT_SIZE, start, 0, log.color, font.get());
//    }
//
//    if(log.toConsole) {
//      Console::log(log.content, Console::DEBUG_INFO);
//      log.toConsole = false;
//    }
//    start.y += lineStep;
//  }
//}
//
//void detail::Debug::tick() {
//
//  std::sort(gLogInfos.begin(), gLogInfos.end(), [](const log_info_t& a, log_info_t& b) {
//    return a.duration < b.duration;
//  });
//
//  commitLogs();
//
//  float frameSec = (float)GetMainClock().frame.second;
//  for(uint i = (uint)gLogInfos.size() - 1, size = i + 1; i < size; i--) {
//
//    log_info_t& log = gLogInfos[i];
//    log.duration -= frameSec;
//
//    if(log.duration < 0) {
//      log = std::move(gLogInfos.back());
//      gLogInfos.pop_back();
//    }
//  }
//}
//
//

void LogFileOutput::out(const std::string& line) {
  {
    file << line;
    fileStamped << line;
  }

}

void LogFileOutput::flush() {
  file.flush();
  fileStamped.flush();
}

COMMAND_REG("log_filter", "name: string, display: bool", "display/hide log with certain tag") (Command& cmd) {

  bool enabled = cmd.arg<1, bool>();
  if (enabled) {
    Log::gLogger->show(cmd.arg<0, std::string>().c_str());
  } else {
    Log::gLogger->hide(cmd.arg<0, std::string>().c_str());
  }

  return true;
}

static bool gOutputToConsole = true;
LOG_CB_REG(const Log::log_t& log) {
  std::string out = log.toString();
#if defined( _WIN32 )
  if (IsDebuggerAvailable()) {
    OutputDebugStringA(out.c_str());
  }
#endif
  if(gOutputToConsole) {
    Console::log(log.toString(), log.tag.color);
  }
  std::cout << out;
}

COMMAND_REG("block_log", "blocked: bool", "toggle whether to block log output") (Command& cmd) {
  gOutputToConsole = !cmd.arg<0, bool>();
  return true;
}

LOG_CB_REG(const Log::log_t& log) {
  gFileOutput->out(log.toString());
}

void logTest(uint threadCount) {
  for(uint i = 0; i < threadCount; i++) {
    Thread t([i]() {
      std::ifstream file("big.txt");
      EXPECTS(file.is_open());
      std::string str;
      uint line = 1;
      while(!file.eof()) {
        std::getline(file, str);
        Log::logf("[%u: %u] %s", i, line++, str.c_str());
      }
    });
  }
}

void logFlushTest() {
  for(int i = 0; i< 100; i++) {
    Log::logf("from flush test: %d", i);
  }
  Log::logf("FLUSH TEST FINISH");
  Log::gLogger->flush();
  DEBUGBREAK;
}

COMMAND_REG("log_test", "", "") (Command&){
  logTest(4);
  return true;
}

COMMAND_REG("disable_log", "tag: string", "filter display log with tag") (Command& cmd) {
  Log::hide(cmd.arg<0, std::string>().c_str());
  return true;
}

COMMAND_REG("enable_log", "tag: string", "display log with tag") (Command& cmd) {
  Log::show(cmd.arg<0, std::string>().c_str());
  return true;
}

COMMAND_REG("log_flush_test", "", "") (Command&) {
  logFlushTest();
  return true;
}