#pragma once
#include "Engine/Core/common.hpp"
#include "Engine/File/FileSystem.hpp"

class Renderer;
class Input;
class Window;

class Engine {
  friend class Renderer;
  friend class Window;
public:

  inline bool ready() const { return mReady; };

  static Engine& Get();
protected:
  Engine() = default;
  void init();

  Renderer* mRenderer = nullptr;
  Window* mWindow = nullptr;
  bool mReady = false;
};
