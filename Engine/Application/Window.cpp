#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "Window.hpp"
#include "Engine/Debug/ErrorWarningAssert.hpp"
#include "Engine/Core/Engine.hpp"

#define GAME_WINDOW_CLASS TEXT("Simple Window Class") 

LRESULT CALLBACK gameWndProc(HWND hwnd,
                             UINT msg,
                             WPARAM wparam,
                             LPARAM lparam) {
  Window* window = Window::Get();
  if(nullptr != window) {
    auto delegates = window->getMessageDelegates();
    for(const WinMsgDelegate& handler: delegates) {
      handler(msg, wparam, lparam);
    }
  }

  return ::DefWindowProc(hwnd, msg, wparam, lparam);
}

vec2 Window::clientCenter() const {
  RECT client;
  HWND hwnd = (HWND)Window::Get()->getHandle(); // Get your windows HWND
  ::GetClientRect(hwnd, &client);

  return {
    (client.left + client.right) / 2.f,
    (client.top + client.bottom) / 2.f
  };
}

ivec2 Window::screenToClient(ivec2 pixelPostion) const {
  POINT desktopPos;
  desktopPos.x = pixelPostion.x;
  desktopPos.y = pixelPostion.y;

  ::ScreenToClient((HWND)mHwnd, &desktopPos);
  
  return { desktopPos.x, desktopPos.y };
}

Window::Window(int width, int height, const char* windowName) {
  init(width, height, windowName);
}

Window::~Window() {
  destory();
}

void Window::init(int width, int height, const char* name) {
  registerWindowClass();

  float clientAspect = (float) width / (float) height;
  mSize = { width, height };

  const DWORD windowStyleFlags = WS_CAPTION | WS_BORDER | WS_THICKFRAME | WS_SYSMENU | WS_OVERLAPPED;
  const DWORD windowStyleExFlags = WS_EX_APPWINDOW;

  // Get desktop rect, dimensions, aspect
  RECT desktopRect;
  HWND desktopWindowHandle = GetDesktopWindow();
  GetClientRect(desktopWindowHandle, &desktopRect);
  float desktopWidth = (float) (desktopRect.right - desktopRect.left);
  float desktopHeight = (float) (desktopRect.bottom - desktopRect.top);
  float desktopAspect = desktopWidth / desktopHeight;

  // Calculate maximum client size (as some % of desktop size)
  constexpr float maxClientFractionOfDesktop = 0.90f;
  float clientWidth = desktopWidth * maxClientFractionOfDesktop;
  float clientHeight = desktopHeight * maxClientFractionOfDesktop;
  if(clientAspect > desktopAspect) {
    // Client window has a wider aspect than desktop; shrink client height to match its width
    clientHeight = clientWidth / clientAspect;
  }
  else {
    // Client window has a taller aspect than desktop; shrink client width to match its height
    clientWidth = clientHeight * clientAspect;
  }

  // Calculate client rect bounds by centering the client area
  float clientMarginX = 0.5f * (desktopWidth - clientWidth);
  float clientMarginY = 0.5f * (desktopHeight - clientHeight);
  RECT clientRect;
  clientRect.left = (int) clientMarginX;
  clientRect.right = clientRect.left + (int) clientWidth;
  clientRect.top = (int) clientMarginY;
  clientRect.bottom = clientRect.top + (int) clientHeight;

  mBounds = { {0.f,0.f}, vec2{clientWidth, clientHeight } };

  // Calculate the outer dimensions of the physical window, including frame et. al.
  RECT windowRect = clientRect;
  AdjustWindowRectEx(&windowRect, windowStyleFlags, FALSE, windowStyleExFlags);

  WCHAR windowTitle[1024];
  MultiByteToWideChar(GetACP(), 0, name, -1, windowTitle, sizeof(windowTitle) / sizeof(windowTitle[0]));

  HINSTANCE applicationInstanceHandle = GetModuleHandle(NULL);

  mHwnd = CreateWindowEx(
                          windowStyleExFlags,
                          GAME_WINDOW_CLASS,
                          windowTitle,
                          windowStyleFlags,
                          windowRect.left,
                          windowRect.top,
                          windowRect.right - windowRect.left,
                          windowRect.bottom - windowRect.top,
                          NULL,
                          NULL,
                          applicationInstanceHandle,
                          NULL);

  HWND wnd = (HWND) mHwnd;
  ShowWindow(wnd, SW_SHOW);
  SetForegroundWindow(wnd);
  SetFocus(wnd);

  //	g_displayDeviceContext = GetDC(g_hWnd);

  HCURSOR cursor = LoadCursor(NULL, IDC_ARROW);
  SetCursor(cursor);

  PIXELFORMATDESCRIPTOR pixelFormatDescriptor;
  memset(&pixelFormatDescriptor, 0, sizeof(pixelFormatDescriptor));
  pixelFormatDescriptor.nSize = sizeof(pixelFormatDescriptor);
  pixelFormatDescriptor.nVersion = 1;
  pixelFormatDescriptor.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
  pixelFormatDescriptor.iPixelType = PFD_TYPE_RGBA;
  pixelFormatDescriptor.cColorBits = 24;
  pixelFormatDescriptor.cDepthBits = 24;
  pixelFormatDescriptor.cAccumBits = 0;
  pixelFormatDescriptor.cStencilBits = 8;

//  AllocConsole();
//
//  HANDLE handle_out = GetStdHandle(STD_OUTPUT_HANDLE);
//  int hCrt = _open_osfhandle((long)handle_out, _O_TEXT);
//  FILE* hf_out = _fdopen(hCrt, "w");
//  setvbuf(hf_out, NULL, _IONBF, 1);
//  *stdout = *hf_out;
//
//  HANDLE handle_in = GetStdHandle(STD_INPUT_HANDLE);
//  hCrt = _open_osfhandle((long)handle_in, _O_TEXT);
//  FILE* hf_in = _fdopen(hCrt, "r");
//  setvbuf(hf_in, NULL, _IONBF, 128);
//  *stdin = *hf_in;
}

void Window::destory() {
  DestroyWindow((HWND)mHwnd);
  UnregisterClass(GAME_WINDOW_CLASS, GetModuleHandle(NULL));
}

Window* Window::Get() {
  return Engine::Get().mWindow;
}

void Window::registerWindowClass() {
  // Define a window style/class
  WNDCLASSEX windowClassDescription;
  memset(&windowClassDescription, 0, sizeof(windowClassDescription));
  windowClassDescription.cbSize = sizeof(windowClassDescription);
  windowClassDescription.style = CS_OWNDC; // Redraw on move, request own Display Context
  windowClassDescription.lpfnWndProc = static_cast<WNDPROC>(gameWndProc);
  // Register our Windows message-handling function
  windowClassDescription.hInstance = GetModuleHandle(NULL);
  windowClassDescription.hIcon = NULL;
  windowClassDescription.hCursor = NULL;
  windowClassDescription.lpszClassName = GAME_WINDOW_CLASS;
  RegisterClassEx(&windowClassDescription);
}
