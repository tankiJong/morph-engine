#include "Window.hpp"
#include <Windows.h>
static Window* gWindow = nullptr; // Instance Pointer; 

LRESULT CALLBACK gameWndProc(HWND hwnd,
                             UINT msg,
                             WPARAM wparam,
                             LPARAM lparam) {
  Window* window = Window::getInstance();
  if(nullptr != window) {
    auto delegates = gWindow->getMessageDelegates();
    for(const WinMsgDelegate& handler: delegates) {
      handler(msg, wparam, lparam);
    }
  }

  return ::DefWindowProc(hwnd, msg, wparam, lparam);
}

Window::Window(int width, int height, const char* windowName) {
  init(width, height, windowName);
}

Window::~Window() {
  destory();
}

void Window::init(int width, int height, const char* name) {
  float clientAspect = (float) width / (float) height;
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
  windowClassDescription.lpszClassName = TEXT("Simple Window Class");
  RegisterClassEx(&windowClassDescription);

  // #SD1ToDo: Add support for fullscreen mode (requires different window style flags than windowed mode)
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

  // Calculate the outer dimensions of the physical window, including frame et. al.
  RECT windowRect = clientRect;
  AdjustWindowRectEx(&windowRect, windowStyleFlags, FALSE, windowStyleExFlags);

  WCHAR windowTitle[1024];
  MultiByteToWideChar(GetACP(), 0, name, -1, windowTitle, sizeof(windowTitle) / sizeof(windowTitle[0]));

  HINSTANCE applicationInstanceHandle = windowClassDescription.hInstance;

  mHwnd = CreateWindowEx(
                          windowStyleExFlags,
                          windowClassDescription.lpszClassName,
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
}

void Window::destory() {
  // QA: what to clean
}

Window* Window::getInstance() {
  if(gWindow == nullptr) {
    gWindow = new Window();
  }

  return gWindow;
}
