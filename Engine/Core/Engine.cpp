#include "Engine.hpp"
#include "Engine/File/Utils.hpp"
#include "Engine/Core/Resource.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/Shader/Shader.hpp"
#include "Engine/Renderer/Shader/Material.hpp"
#include "Engine/Renderer/Font.hpp"
#include "Engine/Debug/Console/Console.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Application/Window.hpp"
#include "Engine/Debug/Profile/Profiler.hpp"
#include "Engine/File/FileSystem.hpp"
#include "Engine/Net/Net.hpp"
#include "Engine/Debug/Console/RemoteConsole.hpp"

Engine* gEngine = nullptr;


constexpr float CLIENT_ASPECT = 1.77f;
constexpr float SCREEN_HALF_HEIGHT = 3.f;
constexpr float SCREEN_HALF_WIDTH = SCREEN_HALF_HEIGHT * CLIENT_ASPECT;


constexpr float SCREEN_WIDTH = SCREEN_HALF_WIDTH * 2.f; // units
constexpr float SCREEN_HEIGHT = SCREEN_HALF_HEIGHT * 2.f; // units


void Engine::init() {
  Log::startUp();

  mRenderer = new Renderer();
  mWindow = new Window();
  mWindow->init((const int)SCREEN_WIDTH, (const int)SCREEN_HEIGHT, "Game");
  mRenderer->init((HWND)Window::Get()->getHandle());

  mReady = true;


  FileSystem& fs = FileSystem::Get();

#ifdef _DEBUG
  fs::path cppPath = fs::path(__FILE__).parent_path();
  fs.mount("/$Content", cppPath / "../../Content");
#else
  fs.mount("/$Content", "Engine");
#endif

  Resource<Texture>::define("$default");

  FileSystem::Get().foreach("/$Content", [](const fs::path& p, auto...) {
    if(p.extension() == ".png" || p.extension() == ".jpg") {
      Resource<Texture>::define(p.generic_string());
    }

    if (p.extension() == ".shader") {
      Resource<Shader>::define(p.generic_string());
      return;
    }
  });

  FileSystem::Get().foreach("/$Content", [](const fs::path& p, auto...) {
    if (p.extension() == ".font") {
      Resource<Font>::define(p.generic_string());
      return;
    }
  });

  Font::Default() = Resource<Font>::get("Fira Mono");


  FileSystem::Get().foreach("/$Content", [](const fs::path& p, auto...) {
    if (p.extension() == ".mat") {
      Resource<Material>::define(p.generic_string());
      return;
    }
  });

  Profile::startup();
  Net::startup();
  RemoteConsole::startup();

  RemoteConsole::get().onReceive([](uint index, const RemoteConsole::Instr& instr) {
    if(!instr.isEcho) {
      auto handler = Console::Get()->onOutput([index] (std::string msg, Console::Severity s) {
        RemoteConsole::get().issue(index, true, msg.c_str());
      });

      Console::Get()->exec(instr.content);
      bool re = handler.release();
      ENSURES(re);
    } else {
      auto& c = RemoteConsole::get().connection(index);
      std::string echo = Stringf("[%s] %s", c.socket.address().toString(), instr.content);
      Console::log(echo, Rgba::white);
    }
  });
  // Blob config = fs::read(".fs");
  // if(config.valid()) {
  //   fs.config(config);
  // }
}

Engine& Engine::Get() {

  if (!gEngine) {
    gEngine = new Engine();
    gEngine->init();
  }

  return *gEngine;
}

Engine::~Engine() {
  RemoteConsole::shutdown();
  Net::shutdown();
  Log::shutDown();
  SAFE_DELETE(mRenderer);
  SAFE_DELETE(mWindow);
}
