#include "Engine.hpp"
#include "Engine/File/Utils.hpp"
#include "Engine/Core/Resource.hpp"
#include "Engine/Application/Window.hpp"
#include "Engine/File/FileSystem.hpp"
#include "Engine/Graphics/RHI/RHIDevice.hpp"
#include "Engine/Debug/Log.hpp"
#include "Engine/Renderer/ImmediateRenderer.hpp"
#include "Engine/Gui/ImGui.hpp"
#include "Engine/Net/Net.hpp"
#include "Engine/Debug/Console/RemoteConsole.hpp"
#include "Engine/Debug/Console/Console.hpp"
#include "Engine/Graphics/Font.hpp"
#include "Engine/Debug/Draw.hpp"

Engine* gEngine = nullptr;


constexpr float CLIENT_ASPECT = 1.77f;
constexpr float SCREEN_HALF_HEIGHT = 3.f;
constexpr float SCREEN_HALF_WIDTH = SCREEN_HALF_HEIGHT * CLIENT_ASPECT;


constexpr float SCREEN_WIDTH = SCREEN_HALF_WIDTH * 2.f; // units
constexpr float SCREEN_HEIGHT = SCREEN_HALF_HEIGHT * 2.f; // units

void Engine::init() {
  Log::startUp();

  mWindow = new Window();
  mWindow->init((const int)SCREEN_WIDTH, (const int)SCREEN_HEIGHT, "Game");
  RHIDevice::create((HWND)Window::Get()->getHandle());
  ImmediateRenderer::get().startUp();
  mReady = true;

  ImGui::startup();
  FileSystem& fs = FileSystem::Get();

#ifdef _DEBUG
  fs::path cppPath = fs::path(__FILE__).parent_path();
  fs.mount("/$Content", cppPath / "../../Content");
#else
  fs.mount("/$Content", "Engine");
#endif

  Resource<Texture2>::define("$default");

  FileSystem::Get().foreach("/$Content", [](const fs::path& p, auto...) {
    if(p.extension() == ".png" || p.extension() == ".jpg") {
      Resource<Texture2>::define(p.generic_string());
    }
    //
    // if (p.extension() == ".shader") {
    //   Resource<Shader>::define(p.generic_string());
    //   return;
    // }
  });
//
  FileSystem::Get().foreach("/$Content", [](const fs::path& p, auto...) {
    if (p.extension() == ".font") {
      Resource<Font>::define(p.generic_string());
      return;
    }
  });
  Font::Default() = Resource<Font>::get("Fira Mono");
//
//
//
//  FileSystem::Get().foreach("/$Content", [](const fs::path& p, auto...) {
//    if (p.extension() == ".mat") {
//      Resource<Material>::define(p.generic_string());
//      return;
//    }
//  });
//
//  Profile::startup();
//  Log::startUp();

  // Blob config = fs::read(".fs");
  // if(config.valid()) {
  //   fs.config(config);
  // }

  Console::Get()->init();
  Net::startup();
  Debug::drawInit();
  // RemoteConsole::startup();
  //
  // RemoteConsole::get().onReceive([](uint index, const RemoteConsole::Instr& instr) {
  //   if(!instr.isEcho) {
  //     auto handler = Console::Get()->onOutput([index] (std::string msg, Console::Severity) {
  //       RemoteConsole::get().issue(index, true, msg.c_str());
  //     });
  //
  //     Console::Get()->exec(instr.content);
  //     bool re = handler.release();
  //     ENSURES(re);
  //   } else {
  //     auto& c = RemoteConsole::get().connection(index);
  //     std::string echo = Stringf("[%s] %s", c.socket.address().toString(), instr.content);
  //     RemoteConsole::get().echo(echo);
  //   }
  // });
}

void Engine::destory() {
  ImGui::shutdown();
  // RemoteConsole::shutdown();
  Net::shutdown();
  // Log::shutDown();
  // SAFE_DELETE(mRenderer);
  SAFE_DELETE(mWindow);
}

Engine& Engine::Get() {

  if (!gEngine) {
    gEngine = new Engine();
  }

  return *gEngine;
}

Engine::~Engine() {
}

