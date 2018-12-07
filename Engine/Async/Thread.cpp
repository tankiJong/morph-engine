#include "Thread.hpp"
#include "Engine/Debug/ErrorWarningAssert.hpp"

#define WIN32_LEAN_AND_MEAN		// Always #define this before #including <windows.h>
#include <windows.h>
#include <thread>
#include "Engine/Debug/Console/Console.hpp"
#include "Engine/Math/MathUtils.hpp"

using thread_handle_t = Thread::thread_handle_t;
using thread_id_t = Thread::thread_id_t;


static DWORD WINAPI threadEntryPoint(void* arg) {
  detail::Launch* launch = static_cast<detail::Launch*>(arg);
  launch->run();

  SAFE_DELETE(launch);
  return 0;
}

thread_handle_t threadCreate(void* args, const char* = "", size_t stackSize = DEFAULT_THREAD_STACK_SIZE) {

  DWORD id = 0;

  thread_handle_t handle = ::CreateThread(NULL, stackSize, threadEntryPoint, args, 0, &id);

  return handle;
}

void threadDetach(thread_handle_t handle) {
  if(Thread::INVALID_HANDLE != handle) {
    ::CloseHandle(handle);
    return;
  }

  // ERROR_RECOVERABLE("illegal thread handle");
}

void threadJoin(thread_handle_t handle) {
  if(Thread::INVALID_HANDLE != handle) {
    ::WaitForSingleObject(handle, INFINITE);
    ::CloseHandle(handle);
    return;
  }

  // ERROR_RECOVERABLE("illegal thread handle");
}

void threadJoin(thread_handle_t* handles, size_t count) {
  for(size_t i = 0; i < count; i++) {
    threadJoin(handles[i]);
  }
}

void threadCreateDetach(void* args, const char* name = "", size_t stackSize = DEFAULT_THREAD_STACK_SIZE) {
  thread_handle_t handle = threadCreate(args, name, stackSize);
  threadDetach(handle);
}

void threadYield() {
  ::SwitchToThread();
}

void threadCountYield(int& cur, int count) {
  ++cur;
  if(cur >= count) {
    threadYield();
    cur = 0;
  }
}

void threadSleep(uint ms) {
  ::Sleep((DWORD)ms);
}

thread_id_t threadGetCurrentId() {
  return (thread_id_t)(uintptr_t)::GetCurrentThread();
}

thread_id_t threadGetId(thread_handle_t handle) {
  return (thread_id_t)(uintptr_t)::GetThreadId(handle);
}


const Thread::thread_handle_t Thread::INVALID_HANDLE = (thread_handle_t)-1;

void Thread::join() {
  threadJoin(mHandle);
  mHandle = INVALID_HANDLE;
}

void Thread::detach() {
  threadDetach(mHandle);
  mHandle = INVALID_HANDLE;
}

Thread::~Thread() {
  if(INVALID_HANDLE != mHandle) detach();
}

void Thread::launch(detail::Launch* launcher) {
  mHandle = threadCreate(launcher, mName);
}

void CurrentThread::yield() {
  return threadYield();
}

void CurrentThread::sleep(uint ms) {
  threadSleep(ms);
}

Thread::thread_id_t CurrentThread::id() {
  return threadGetCurrentId();
}


void ThreadTest() {
  constexpr u64 NUM_COUNT = 12000000;

  for (uint i = 0; i < 12000000; ++i) {
    getRandomf01();
  }

  DebuggerPrintf("Finished ThreadTestWork");
}

COMMAND_REG("thread_test_main", "", "")(Command&) {
  ThreadTest();
  return true;
}

COMMAND_REG("thread_test_threaded", "", "")(Command&) {
  Thread t(ThreadTest);
  return true;
}