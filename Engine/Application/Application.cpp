#include "Application.hpp"
#include "Engine/Graphics/Font.hpp"
#include "Engine/File/FileSystem.hpp"
#include "Engine/Debug/Console/Console.hpp"
#include "Engine/Core/Engine.hpp"
#include "Engine/Application/Window.hpp"
#include "Engine/Core/Time/Clock.hpp"
#include "Engine/Input/Input.hpp"
#include "Engine/Gui/ImGui.hpp"
#include "Engine/Debug/Draw.hpp"
#include "Engine/Graphics/RHI/RHIDevice.hpp"

bool Application::runFrame() {
  switch(mRunStatus) { 
    case APP_CREATED: {
      _init();
      mRunStatus = APP_RUNNING;
    } return true;

    case APP_RUNNING: {
      _update();
    } return true;

    case APP_QUITTING: {
      _destroy();
    } return false;
  }

  return false;
}

void Application::_init() {
  mEngine = &Engine::Get();
  mEngine->init();

  FileSystem::Get().mount("/Data", "Data");

  Window::Get()->addWinMessageHandler([this](uint msg, size_t wparam, size_t lparam) {
    this->windowProc(msg, wparam, lparam);
  });

  Debug::drawInit();

  Debug::setClock(&GetMainClock());

  onInit();
}

void Application::_destroy() {
  onDestroy();
  mEngine->destory();
  mEngine = nullptr;
}

void Application::_update() {
  GetMainClock().beginFrame();
  Input::Get().beforeFrame();
  ImGui::beginFrame();

  onStartFrame();

  onInput();
  onUpdate();
  postUpdate();

  onRender();
  onGui();
  ImGui::render();
  Debug::drawNow();

  RHIDevice::get()->present();

  onEndFrame();
}

void Application::windowProc(uint wmMessageCode, size_t /*wParam*/, size_t /*lParam*/) {
  switch (wmMessageCode) {
    // App close requested via "X" button, or right-click "Close Window" on task bar, or "Close" from system menu, or Alt-F4
    case WM_CLOSE:
    {
      mRunStatus = APP_QUITTING;
      return; // "Consumes" this message (tells Windows "okay, we handled it")
    }
  }
}
