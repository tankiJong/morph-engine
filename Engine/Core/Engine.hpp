#pragma once

class Renderer;
class Input;
class Window;

class Engine {
  friend class Renderer;
  friend class Window;
public:

  inline bool ready() const { return mReady; };

  static Engine& Get();

  ~Engine();
  void init();
  void destory();
protected:
  Engine() = default;

  Renderer* mRenderer = nullptr;
  Window* mWindow = nullptr;
  bool mReady = false;
};
