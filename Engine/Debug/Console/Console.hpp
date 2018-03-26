#pragma once
#include <array>
#include <deque>
#include <sstream>
#include "Engine/Core/common.hpp"
#include "Engine/Core/Delegate.hpp"
#include "Engine/Core/Rgba.hpp"
#include "Engine/Core/Utils.hpp"
#include "Engine/Math/Primitives/IntRange.hpp"
#include "Engine/Core/StringUtils.hpp"

class Command;
class Renderer;
class Input;
class Camera;
using CommandHandler = delegate<bool(Command&)> ;


class Console {
  struct CommandDef {
    std::string mName = "";
    std::string mParamInfo;
    std::string mDescription;
    CommandHandler mHandle;

    template<typename Functor>
    CommandDef(const std::string& name, const std::string& paramInfo, const std::string& description, Functor&& cb)
      : mName(name)
      , mParamInfo(paramInfo)
      , mDescription(description)
      , mHandle(std::forward<Functor>(cb)) {}
    CommandDef() = default;
  };


public:
  enum Severity {
    DEBUG_INFO,
    DEBUG_WARN,
    DEBUG_ERROR,
  };
  static bool isOpen() { return get()->mIsOpened; };
  static Console* get();
  static void log(const std::string& msg, Severity level);
  static void info(const std::string& msg);
  static void warn(const std::string& msg);
  static void error(const std::string& msg);
  static constexpr uint MAX_COMMAND_COUNT = 1000u;
  static constexpr float FONT_SIZE = 16;
  static constexpr float FONT_ASPECT = 0.7f;
  static constexpr float WORD_PADDING = 1;
  static constexpr float LINE_HEIGHT = FONT_SIZE + WORD_PADDING * 2;
  static constexpr float SCROLLBAR_WIDTH = 3;
  static constexpr uint MAX_LOG_NUMBER = 200u;
public:
  void init(Renderer& renderer, Input& input);;
  template<typename T>
  void input(const T& val) {
    std::string str = toString(val);
    input(v);
  }

  template<>
  void input(const std::string& val) {
    if (mSelection.size() > 0) {
      erase(mSelection.min, mSelection.max);
      mSelection.reset();
    }
    mInputStream.insert(mInputStream.cbegin() + mCursorPosition, val.cbegin(), val.cend());
    mCursorPosition+=val.length();
  }

  template<>
  void input(const char& c) {
    if (mSelection.size() > 0) {
      erase(mSelection.min, mSelection.max);
      mSelection.reset();
    }
    mInputStream.insert(mInputStream.cbegin() + mCursorPosition, c);
    mCursorPosition++;
  }
  
  void replaceInput(std::string str);
  void update(float deltaSecond);
  void render() const;
        
  inline void open() { mIsOpened = true; };
  inline void close() { mIsOpened = false; };
  inline void toggle() { mIsOpened = !mIsOpened; };
  inline bool opened() const { return mIsOpened; }
  bool exec(const std::string& cmd);
  bool exec();
  template<typename Functor>
  bool hook(const std::string& cmdName, const std::string& paramInfo, const std::string& description, Functor&& cb);
  bool clear();

protected:
  Console();
  ~Console();
  CommandDef* findCommand(const std::string& name);
  void autoCompleteNext(const std::string& txt);
  void consoleCharHandler(unsigned msg, size_t wParam, size_t lParam);
  void consoleControlHandler(unsigned msg, size_t wParam, size_t lParam);
  void print(const CommandDef& cmdDef);

  void record(const std::string& msg, Severity level);

  void erase(uint from, uint to);

  bool mIsOpened = false;
  Renderer* mRenderer = nullptr;
  Input* mInput = nullptr;

  // input state control
  std::string mInputStream;
  uint mCursorPosition;
  IntRange mSelection;
  bool mIsSelecting = false ;
  Rgba mFontColor;
  bool mDrawCursor;
  float mCursorFlashSec = 0.f;
  uint mLineNumberAtBottom = 0;
  uint mTotalLineLogCanRender = 0;
  uint mAutoCompleteIndex = 0;
  std::string mAutoCompleteText;
  // display window content
  std::deque<std::string> mLogStream;

  uint mNumCurrentCommand = 0;
  std::array<owner<CommandDef*>, MAX_COMMAND_COUNT> mlegalCommands;
  std::vector<std::string> mCommandLog;
  uint mNextCommandLogIndex = 0;
  Camera* mCamera = nullptr;
private:
  void hookInBuiltInCommand();
};

template<typename Functor>
bool Console::hook(const std::string& cmdName, const std::string& paramInfo, const std::string& description,
  Functor&& cb) {
  for(uint i = 0; i<mNumCurrentCommand; i++) {
    if (mlegalCommands[i]->mName == cmdName) return false;
  }

  mlegalCommands[mNumCurrentCommand++] = new CommandDef(cmdName, paramInfo, description, std::forward<Functor>(cb));

  return true;
}