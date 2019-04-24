#pragma once
#include "Engine/Core/common.hpp"
#include <forward_list>


#define DEFAULT_THREAD_STACK_SIZE 65536

namespace detail {
  struct Launch {
    virtual void run() = 0;
    virtual ~Launch() {}
  };
  template<typename ...Pack>
  struct Launcher final: public Launch {
    Launcher(Pack&& ...pack)
      : data(std::forward<Pack>(pack)...) {};

    std::tuple<Pack...> data;

    template<size_t... Idxs>
    static void execute(std::tuple<Pack...>& tuple, std::index_sequence<Idxs...>) {
      std::invoke(
        std::move(std::get<Idxs>(tuple))...);
      
    }

    void run() override {
      execute(
        data,
        std::make_index_sequence
          <std::tuple_size_v<std::tuple<Pack...>>>()
      );
    }
  };
}

class Thread {
public:
  using thread_handle_t = void*;
  using thread_id_t = void*;
  static const thread_handle_t INVALID_HANDLE;

  Thread() = default;

  template<typename Fn, typename ...Args>
  Thread(Fn&& fn, Args&& ...args) {
    auto* launcher = 
      new detail::Launcher<Fn, Args...>(
        std::forward<Fn>(fn),
        std::forward<Args>(args)...);
    launch(launcher);
  };

  template<typename Fn, typename ...Args>
  Thread(const char* name, Fn&& fn, Args&& ...args) {
    mName = name;
    auto* launcher =
      new detail::Launcher<Fn, std::decay_t<Args>...>(
        std::forward<Fn>(fn),
        std::forward<std::decay_t<Args>>(args)...);
    launch(launcher);
  };

  Thread(const Thread&) = delete;
  Thread(Thread&&) noexcept;

  Thread& operator=(const Thread&) = delete;
  Thread& operator=(Thread&&) noexcept;

  void join();
  void detach();
  ~Thread();

protected:
  static unsigned long __stdcall called();
  void launch(detail::Launch* launcher);
  thread_id_t   mId = 0;
  thread_handle_t mHandle = INVALID_HANDLE;
  const char* mName = "";

};

namespace CurrentThread {
  void yield();
  void sleep(uint ms);
  Thread::thread_id_t id();
}