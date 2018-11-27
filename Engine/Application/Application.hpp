#pragma once
#include "Engine/Core/common.hpp"

class Engine;
class Application {
  enum eRunStatus {
    APP_CREATED,
    APP_RUNNING,
    APP_QUITTING,
  };
public:
  virtual ~Application() = default;

  virtual void onInit() {}

  virtual void onStartFrame() {}

  virtual void onInput() {}

  virtual void onUpdate() {}
  virtual void postUpdate() {}

  virtual void onRender() const {}

  virtual void onGui() {}
  virtual void onQuit() {}

  virtual void onEndFrame() {}

  virtual void onDestroy() {}

  bool runFrame();
protected:

private:
  void _init();
  void _destroy();
  void _update();

  void __stdcall windowProc(uint wmMessageCode, size_t wParam, size_t lParam);
  Engine* mEngine = nullptr;
  eRunStatus mRunStatus = APP_CREATED;
};
