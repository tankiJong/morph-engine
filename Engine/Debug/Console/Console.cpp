#include "Console.hpp"
#include <fstream>
#include "Engine/Debug/Console/Command.hpp"
#include "Engine/Debug/ErrorWarningAssert.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Application/Window.hpp"
#include "Engine/Math/Primitives/AABB2.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Input/Input.hpp"
#include "Engine/Core/Delegate.hpp"
#include "Engine/Tool/Parser.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Debug/Draw.hpp"
#include "Engine/Renderer/Shader/Shader.hpp"
#include "Engine/Renderer/Geometry/Mesher.hpp"
#include "Engine/Renderer/Font.hpp"
#include "Engine/Renderer/Sampler.hpp"

#define WM_CHAR                 0x0102
#define WM_KEYDOWN              0x0100
#define WM_KEYUP                0x0101
#define JUST_KEYDOWN_STATE      0x80000000

static const Rgba& DEFAULT_TEXT_COLOR = Rgba::white;
static Console* gConsole = nullptr;

static auto colorCode = 
bind(one_of("123456789"), [](char c, parse_input_t rest) {
  parse_result_t<unsigned char> result = many<unsigned char>
    (one_of("0123456789"), c - '0',
     [](unsigned char result, char c1) -> unsigned char { return result * 10u + (c1 - '0'); })(rest);

  return (result->first == 0) ? DEFAULT_TEXT_COLOR : Hue(result->first);
});

// only support font color control code for now
static auto colorString = "\x1b["_P < many_of("0123456789") > "m"_P;

static auto getColor = fmap(colorCode, colorString);

static auto stringContent = many<std::string>( except_of(R"(")") | R"(\")"_P , "", [](auto a, auto b) { return a.append(b); });

template<typename F>
void consumeConsoleText(std::string& in, F&& fn) {

  Rgba currentColor = DEFAULT_TEXT_COLOR;
  while(in.length() > 0) {
    switch(in[0]) {
      case '\x1b': { // \e or \x1b or \033
        parse_result_t<Rgba> parseResult = getColor(in);
        // for now, since don't have echo like command, 
        // error only happen from my own implementation side
        if(parseResult) {
          currentColor = parseResult->first;
          in = std::move(parseResult->second);
          break;
        }
      }
      default: {
        parse_result_t<std::string> parseResult = except_of("\x1b")(in);
        // for now, since don't have echo like command, 
        // error only happen from my own implementation side
        EXPECTS(parseResult);
        in = std::move(parseResult->second);
        fn(parseResult->first, currentColor);
      }
    }
  }
}

bool saveLog(std::string path, const std::deque<std::string>& logStream) {
  // warp this to file util
  std::ofstream file;

  file.open(path);

  if(file.fail()) {
    Console::error("file " + path + " opened failed");
    return false;
  }

  bool flag = true;
  for (std::string aLog : logStream) {
    consumeConsoleText(aLog, [&flag, &file](const std::string& rawText, const Rgba&) {
      flag = flag && (file << rawText);
    });

    flag = flag && (file << '\n');
  }

  if(!flag) {
    Console::error("file " + path + " write failed");
  }

  file.close();

  return flag;

}

void Console::init(Renderer& renderer, Input& input) {
  mRenderer = &renderer;
  mInput = &input;

  const aabb2& bounds = Window::Get()->bounds();
  mCamera = new Camera();
  mCamera->setProjection(mat44::makeOrtho(0, bounds.width(), 0, bounds.height(), -1.f, 1.f));
  mCamera->setColorTarget(mRenderer->getDefaultColorTarget());
//  mCamera->setDepthStencilTarget(mRenderer->getDefaultDepthTarget());

  mTotalLineLogCanRender = uint((bounds.height() - LINE_HEIGHT) / LINE_HEIGHT);
}

void Console::replaceInput(std::string str) {
  mInputStream = std::move(str);
  mCursorPosition = mInputStream.size();
}

void Console::update(float deltaSecond) {
  static uint startSlection = 0;

  while(mLogStream.size() > MAX_LOG_NUMBER) {
    mLogStream.pop_front();
  }
  if (!mIsOpened) return;

  if(mInput->isKeyJustDown(KEYBOARD_ESCAPE)) {
    if (mInputStream.size() > 0) {
      erase(0, mInputStream.size());
    } else {
      mIsOpened = false;
    }
  }
  mCursorFlashSec += deltaSecond;
  if(mCursorFlashSec > .5f) {
    mCursorFlashSec = 0.f;
    mDrawCursor = !mDrawCursor;
  }
}

void Console::consoleCharHandler(unsigned msg, size_t wParam, size_t) {
  if (msg != WM_CHAR ) return;
  if (!mIsOpened) return;
//  if (lParam&JUST_KEYDOWN_STATE)
  DebuggerPrintf("Key %c(%i) pressed\n", (char)wParam, wParam);
  if(wParam >=32 && wParam <= 126) {
    input(char(wParam));
    return;
  }

  switch (wParam) {
    case KEYBOARD_TAB:
      autoCompleteNext(mInputStream);
      return;
    case KEYBOARD_BACK:
      if(mSelection.size() > 0) {
        erase(mSelection.min, mSelection.max);
      } else {
        erase(mCursorPosition - 1, mCursorPosition);
      }
      return;
    case KEYBOARD_DELETE:
      if(mSelection.size() > 0) {
        erase(mSelection.min, mSelection.max);
      } else {
        erase(mCursorPosition, mCursorPosition + 1);
      }
    case KEYBOARD_RETURN:
      exec();
      return;
  }
}

void Console::consoleControlHandler(unsigned msg, size_t wParam, size_t) {
  static uint startSlection = 0;

  if(msg == WM_KEYDOWN) {
    switch(wParam) {
      // close console
//      case KEYBOARD_ESCAPE:
//        if (mInputStream.size() > 0) {
//          erase(0, mInputStream.size());
//        } else {
//          mIsOpened = false;
//        }
//        return;

      // selection
      case KEYBOARD_SHIFT:
        if(!mIsSelecting) {
          startSlection = mCursorPosition;
          mIsSelecting = true;
        }
        return;
      case KEYBOARD_PRIOR:
        if (mLineNumberAtBottom + mTotalLineLogCanRender < mLogStream.size()) {
          mLineNumberAtBottom++;
        }
        return;
      case KEYBOARD_NEXT:
        if (mLineNumberAtBottom > 0) {
          mLineNumberAtBottom--;
        }
        return;
      case KEYBOARD_UP:
        if (mCommandLog.size() == 0) return;
        if(mNextCommandLogIndex == mCommandLog.size()) {
          mNextCommandLogIndex = 0;
        }
        replaceInput(*(mCommandLog.rbegin() + (mNextCommandLogIndex++)));
        return;
      case KEYBOARD_DOWN:
        if (mCommandLog.size() == 0) return;
        if (mNextCommandLogIndex == uint(-1)) {
          mNextCommandLogIndex = mCommandLog.size() - 1;
        }
        replaceInput(*(mCommandLog.rbegin() + (mNextCommandLogIndex--)));
        return;

      // selection & cursor
      case KEYBOARD_LEFT:
        if(mCursorPosition > 0) {
          mCursorPosition--;
          if(mIsSelecting) {
            mSelection.min = std::min(startSlection, mCursorPosition);
            mSelection.max = std::max(startSlection, mCursorPosition);
          }
          else {
            if(mSelection.size() != 0) {
              mCursorPosition = mSelection.min;
              mSelection.reset();
            }
          }
        }
        return;

      // selection & cursor
      case KEYBOARD_RIGHT:
        if(mCursorPosition < mInputStream.length()) {
          mCursorPosition++;
          if(mIsSelecting) {
            mSelection.min = std::min(startSlection, mCursorPosition);
            mSelection.max = std::max(startSlection, mCursorPosition);
          }
          else {
            if (mSelection.size() != 0) {
              mCursorPosition = mSelection.max;
              mSelection.reset();
            }
          }
        }
        return;
    }
  }

  if(msg == WM_KEYUP) {
    switch(wParam) {
      case KEYBOARD_SHIFT:
        mIsSelecting = false;
        return;
    }
  }
}

void Console::print(const CommandDef& cmdDef) {
  record(Stringf("[%s]\x1b[42m%s: \x1b[0m%s", cmdDef.mName.c_str(), cmdDef.mParamInfo.c_str(), cmdDef.mDescription.c_str()), DEBUG_INFO);
}

void Console::record(const std::string& msg, Severity level) {
  switch (level) {
    case DEBUG_INFO:
      mLogStream.push_back("\x1b[92m" + msg + "\x1b[0m");
      break;
    case DEBUG_WARN:
      mLogStream.push_back("\x1b[21m" + msg + "\x1b[0m");
      break;
    case DEBUG_ERROR:
      mLogStream.push_back("\x1b[255m" + msg + "\x1b[0m");
      break;
  }

  if (mLineNumberAtBottom != 0 && mLineNumberAtBottom + mTotalLineLogCanRender < mLogStream.size()) {
    mLineNumberAtBottom++;
  }
}

void Console::erase(uint from, uint to) {
  // only two side cases actually want to deal with, when cursor at begin or end
  if (from == (uint)-1) return;
  if (to == mInputStream.length()+1) return;

  EXPECTS(to - from <= mInputStream.length());
  EXPECTS(to >= from);

  auto stringIterator = mInputStream.erase(mInputStream.cbegin() + from
                                          ,mInputStream.cbegin() + to);
  mCursorPosition = from;
  mSelection.reset();
}

void Console::hookInBuiltInCommand() {
  hook("help", "", "display all registered commands.", [this](Command&) {
    log("============================ Existing Commands ============================", DEBUG_INFO);
    for (uint i = 0; i < mNumCurrentCommand; i++) {
      print(*mlegalCommands[i]);
    }
    log("===========================================================================", DEBUG_INFO);

    return true;
  });

  hook("clear", "", "clear the console.", [this](Command&) {
    clear();
    return true;
  });

  hook("echo_with_color", "[hue] unsigned char, [info] string", "print info with color Hue.", [this](Command& command) {
    unsigned char color = command.arg<0,unsigned char>();
    std::string info = command.arg<1, std::string>();

    mLogStream.push_back(Stringf("\x1b[%um%s\x1b[0m", color, info.data()));
    return true;
  });

  hook("save_log", "[path] string", "save the console log to path.", [this](Command& command) {
    std::string path = std::move(command.arg<0, std::string>());
    bool flag = saveLog(path, mLogStream);
    if(flag) {
      log("save file to " + path + " successed!", DEBUG_INFO);
    } else {
      log("save file to " + path + " failed!", DEBUG_ERROR);
    }
    return flag;
  });

  hook("debug_draw", "[enable] 1/0", "enable/disable the debug_draw(time will still elapse).", [](Command& command) {
    int a = command.arg<0, int>();
    if(a == 0) {
      Debug::toggleDebugRender(false);
      log("disable debug renderer.", DEBUG_INFO);
      return true;
    }

    if (a == 1) {
      Debug::toggleDebugRender(true);
      log("enable debug renderer.", DEBUG_INFO);
      return true;
    }

    return false;
  });

  hook("debug_draw_clear", "[]", "enable/disable the debug_draw(time will still elapse).", [](Command&) {
    Debug::clear();
    return true;
  });
  
}

void Console::render() const {

  float inputBoxHeight = LINE_HEIGHT;
  float descender = mFont->descender(FONT_SIZE);
  const aabb2& screenBounds = { vec2::zero, vec2{(float)mCamera->width(), (float)mCamera->height()}};

  mRenderer->setCamera(mCamera);
  static const Shader* defaultShader = Resource<Shader>::get("shader/ui/default").get();
  mRenderer->setShader(defaultShader);
  mRenderer->setTexture();

  Mesher ms;

  // ###### draw input box
  ms.begin(DRAW_TRIANGES);
  ms.color(Rgba(0, 0, 0, 200));
  ms.quad(vec3(screenBounds.mins, 0.f), 
          vec3{ screenBounds.maxs.x, screenBounds.mins.y, 0 }, 
          vec3{ screenBounds.maxs.x, inputBoxHeight, 0 },
          vec3{ screenBounds.mins.x, inputBoxHeight, 0 });

  // ###### draw log window
  ms.color(Rgba(0, 50, 70, 200));
  ms.quad(vec3{ screenBounds.mins.x, inputBoxHeight , 0 },
          vec3{ screenBounds.maxs.x, inputBoxHeight , 0 },
          vec3{ screenBounds.maxs, 0 },
          vec3{ screenBounds.mins.x, screenBounds.maxs.y, 0 });
  ms.end();

  // ###### draw scrollbar
  ms.begin(DRAW_TRIANGES);
  // bg
  vec2 scrollBarMins =  screenBounds.mins + vec2{ screenBounds.width() - SCROLLBAR_WIDTH, inputBoxHeight };
  const vec2& scrollBarMaxs = screenBounds.maxs;
  ms.color(Rgba(255, 255, 255, 100));
  ms.quad2({ scrollBarMins, scrollBarMaxs }, -.1f);

  // bar
  float barAreaHeight = scrollBarMaxs.y - scrollBarMins.y;
  float barHeight = (float)mTotalLineLogCanRender / (float)mLogStream.size() * barAreaHeight;
  float barPadding = (float)mLineNumberAtBottom / (float)mLogStream.size() *barAreaHeight;
  ms.color(Rgba(255, 255, 255, 150));
  ms.quad2({ scrollBarMins + vec2{ 0, barPadding },
             scrollBarMins + vec2{ SCROLLBAR_WIDTH, barHeight + barPadding } }, -.1f);
  ms.end();

  // ###### draw selection
  EXPECTS(mSelection.min >= 0 && mSelection.max >=0);
  std::string_view inputText(mInputStream);
  if(mSelection.size() > 0) {


    float selectionStart = mFont->advance(inputText.substr(0, mSelection.min), FONT_SIZE);
    float selectionEnd = mFont->advance(inputText.substr(mSelection.min, mSelection.size()), FONT_SIZE);

    vec2 selectionBottomLeft = screenBounds.mins + vec2{ selectionStart, 0 };
    vec2 selectionTopRight = selectionBottomLeft + vec2{ selectionEnd, inputBoxHeight };

    ms.begin(DRAW_TRIANGES);
    ms.color(Rgba::red);
    ms.quad2({ selectionBottomLeft, selectionTopRight }, -.1f);
    ms.end();
  }

  // ###### draw cursor
  if (mDrawCursor) {
    float cursorX = mFont->advance(std::string_view(mInputStream).substr(0, mCursorPosition), FONT_SIZE) + WORD_PADDING;
    float cursorY = screenBounds.mins.y;
    float cursorHeight = LINE_HEIGHT;
    float cursorWidth = 2.f;

    ms.begin(DRAW_TRIANGES);
    ms.color(Rgba::white);
    ms.quad(
      vec3{cursorX + .5f * cursorWidth, cursorY + .5f * cursorHeight, .3f},
      vec3::right,
      vec3::up,
      vec2{ cursorWidth, cursorHeight}
    );
    ms.end();
  }

  Mesh* layout = ms.createMesh<vertex_pcu_t>();
  mRenderer->drawMesh(*layout);

  // ###### render text

  Mesher printer;

  printer.begin(DRAW_TRIANGES);
  // ### input text
  printer.color(Rgba::white);
  printer.text( inputText, FONT_SIZE, mFont.get(),
               vec3{ screenBounds.mins + vec2(WORD_PADDING, descender), -.2f }, vec3::right);

  // ###### draw log
  {
    std::vector<std::string> stringChunks;
    std::vector<Rgba> colors;

    float startY = LINE_HEIGHT;

    auto prepareTextRenderData = [&stringChunks, &colors](const std::string& str, const Rgba& color) {
     stringChunks.push_back(str);
     colors.push_back(color);
    };

    for (auto it = mLogStream.rbegin() + mLineNumberAtBottom; it != mLogStream.rend(); ++it) {
      std::string aLog = *it;
      stringChunks.clear();
      colors.clear();
      float startX = 0.f;
      consumeConsoleText(aLog, prepareTextRenderData);
      for(uint i = 0; i<stringChunks.size(); i++) {
        std::string& txt = stringChunks[i];
        printer.color(colors[i]);
        printer.text(txt, FONT_SIZE, mFont.get(), vec3{ startX, startY + descender, 0 });
        startX += mFont->advance(txt, FONT_SIZE);
      }
      startY += inputBoxHeight;
      stringChunks.clear();
      colors.clear();
    }
  }

  printer.end();

  Mesh* text = printer.createMesh<vertex_pcu_t>();
  static const Shader* fontShader = Resource<Shader>::get("shader/ui/font").get();
  mRenderer->setShader(fontShader);
  mRenderer->setTexture(mFont->texture(0));
  mRenderer->setSampler(0, &Sampler::Linear());
  mRenderer->drawMesh(*text);
}

bool Console::exec(const std::string& cmd) {
  if (cmd.empty()) return true;

  if(mNextCommandLogIndex != uint(-1)) {
    mCommandLog.push_back(cmd);
    mNextCommandLogIndex = 0;
  }
  Command command(cmd);

  if(!command.isLegal()) {
    return false;
  }

  CommandDef* def = findCommand(command.name());

  if (def == nullptr) {
    record("Command [" + command.name() + "] not found.", DEBUG_ERROR);
    return false;
  }

  bool flag = true;
  try {
    flag = def->mHandle(command);
  } catch (const std::exception& e) {
    flag = false;
    log(std::string(e.what()), DEBUG_ERROR);
  }

  return flag;
}

bool Console::exec() {
  bool flag = exec(mInputStream);
  mInputStream.clear();
  mSelection.reset();
  mCursorPosition = 0;

  return flag;
}

Console* Console::get() {
  if(gConsole == nullptr) {
    gConsole = new Console();
  }

  return gConsole;
}

void Console::log(const std::string& msg, Severity level) {
  gConsole->record(msg, level);
}

void Console::info(const std::string& msg) {
  Console::log(msg, Console::DEBUG_INFO);
}

void Console::warn(const std::string& msg) {
  Console::log(msg, Console::DEBUG_WARN);
}

void Console::error(const std::string& msg) {
  Console::log(msg, Console::DEBUG_ERROR);
}

bool Console::clear() {
  mLogStream.clear();
  mLineNumberAtBottom = 0;
  return true;
}

Console::Console() {
  Window::Get()->addWinMessageHandler([this](unsigned msg, size_t wParam, size_t lParam) {
    if (!mIsOpened) return;
    consoleCharHandler(msg, wParam, lParam);
    consoleControlHandler(msg, wParam, lParam);
  });
  mFont = Font::Default();
  EXPECTS(mFont != nullptr);
  LINE_HEIGHT = mFont->lineHeight(FONT_SIZE);
  hookInBuiltInCommand();

  mInputStream.reserve(1024u);


//  for(uint i = 0; i < 20; i++) {
//    exec("help");
//  }
//
//  for (uint i = 0; i < 20; i++) {
//    exec("asdfa");
//  }
//
//  for (uint i = 0; i < 20; i++) {
//    exec("help");
//  }
}

Console::~Console() {
  for(uint i = 0; i < mNumCurrentCommand; i++) {
    delete mlegalCommands[i];
    mlegalCommands[i] = nullptr;
  }

  if(mCamera) {
    delete mCamera;
    mCamera = nullptr;
  }
}

Console::CommandDef* Console::findCommand(const std::string& name) {
  for(uint i = 0; i < mNumCurrentCommand; i++) {
    if(mlegalCommands[i]->mName == name) return mlegalCommands[i];
  }

  return nullptr;
}

void Console::autoCompleteNext(const std::string& txt) {
  if(txt != mAutoCompleteText) {
    mAutoCompleteIndex = 0;
    mAutoCompleteText = txt;
  }

  CommandDef* def = nullptr;
  for (uint i = 0; i < mNumCurrentCommand; i++) {
    const auto& name = mlegalCommands[i]->mName;
    if (mAutoCompleteText.length() > name.length()) continue;
    if(!std::equal(
      name.cbegin(), name.cbegin() + mAutoCompleteText.length(),
      mAutoCompleteText.cbegin(), mAutoCompleteText.cend()
      )) {
      continue;
    }
    def = mlegalCommands[i];
    break;
  }

  if(def != nullptr) {
    if(def->mName == mAutoCompleteText) {
      mSelection.min = 0;
      mSelection.max = 0;
      input(' ');
    } else {
      mSelection.min = (int)mInputStream.size();
      mSelection.max = (int)def->mName.size();
      replaceInput(def->mName);
    }
  } else {
    mSelection.min = 0;
    mSelection.max = 0;
  }
}

