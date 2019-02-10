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
#include "Engine/Debug/Log.hpp"

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
  FileSystem::Get().foreach("/Data", [](const fs::path& p, auto...) {
    if(p.extension() == ".png" || p.extension() == ".jpg") {
      Resource<Texture2>::define(p.generic_string());
    }
    //
    // if (p.extension() == ".shader") {
    //   Resource<Shader>::define(p.generic_string());
    //   return;
    // }
  });

  Window::Get()->addWinMessageHandler([this](uint msg, size_t wparam, size_t lparam) {
    this->windowProc(msg, wparam, lparam);
  });

  Debug::drawInit();

  Debug::setClock(&GetMainClock());

  Log::startUp();
  onInit();
}

void Application::_destroy() {
  onDestroy();
  Log::shutDown();
  mEngine->destory();
  mEngine = nullptr;
}

void Application::_update() {
  GetMainClock().beginFrame();
  Input::Get().beforeFrame();
  ImGui::beginFrame();

  onStartFrame();

  _input();

  {
    if(Console::Get()->isOpen() && !Input::Get().isKeyJustDown(KEYBOARD_OEM_3)) {
      Console::Get()
        ->update((float)GetMainClock().frame.second);
    }
  }

  postUpdate();

  onRender();
  onGui();
  Debug::drawNow();
  ImGui::render();

  if (Console::Get()->isOpen()) {
    Console* cons = Console::Get();
    cons->render();
  }

  RHIDevice::get()->present();

  onEndFrame();

  Input::Get().afterFrame();
}

void Application::_input() {
  if(!Console::Get()->isOpen()) {
    onInput();
  }

  if (Input::Get().isKeyJustDown(KEYBOARD_OEM_3)) {
    Console::Get()->toggle();
  }
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
