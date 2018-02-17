#pragma once

#include <vector>
#include "Engine/Core/Delegate.hpp"
#include "Engine/Core/common.hpp"
#include "Engine/Math/Primitives/AABB2.hpp"
#include "Engine/Math/Primitives/ivec2.hpp"

typedef delegate<void(uint msg, size_t wparam, size_t lparam)> WinMsgDelegate;

class Window {
public:
  // Do all the window creation work is currently in App
  ~Window();

  void init(int width, int height, const char* name = "");
  void destory();
  // Register a function callback to the Window.  Any time Windows processing a 
  // message, this callback should forwarded the information, along with the
  // supplied user argument. 

  template<typename CallBack>
  uint addWinMessageHandler(const CallBack& cb) {
    uint idx = this->mHandlers.size();

    mHandlers.emplace_back(cb);
    return idx;
  };

  void removeWinMessageHandler(uint handle) {
    mHandlers.erase(mHandlers.begin() + handle);
  }

  // This is safely castable to an HWND
  void* getHandle() const { return mHwnd; }
  inline const std::vector<WinMsgDelegate>& getMessageDelegates() const { return mHandlers; }
  // ** EXTRAS ** //
  // void SetTitle( char const *new_title ); 

  inline const aabb2& bounds() const { return mBounds; }
  inline const ivec2&  size() const { return mSize; }
private:
  Window(int width, int height, const char* windowName = "");
  Window() = default;
  void* mHwnd = nullptr; // intptr_t  

                // When the WindowsProcedure is called - let all listeners get first crack at the message
                // Giving us better code modularity. 

  std::vector<WinMsgDelegate> mHandlers;
  aabb2 mBounds;
  ivec2 mSize;
public:
  static Window* getInstance();
  static void registerWindowClass();
};