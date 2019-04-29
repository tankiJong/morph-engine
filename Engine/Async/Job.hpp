#pragma once
#include "Engine/Core/common.hpp"
#include <atomic>
#include "Engine/Debug/ErrorWarningAssert.hpp"
#include "Engine/Async/Thread.hpp"
#include "Engine/Core/Delegate.hpp"
#include "Engine/Core/closure.hpp"

namespace Job {

   using job_type_t = uint16_t;
   using counter_id_t = uint32_t;
   using category_t = uint8_t;
   constexpr uint kMaxDependentJob = 1024;

   enum eCategory: category_t {
      CAT_GENERIC = 0,
      CAT_GENERIC_SLOW,
      CAT_MAIN_THREAD,
      CAT_IO,
      NUM_CATEGORY,
   };

   class Counter;

   class Decl {
   public:
      template<typename ...Args, typename T, 
               typename C = typename detail::closure_decay<T, void, Args...>::type,
               typename = std::enable_if_t<std::is_invocable_v<T, Args...>>,
               typename = std::enable_if_t<!std::is_same_v<C, Decl>>>
      Decl(T&& func, Args&&... args)
         : mClosure(std::forward<T>(func), std::forward<Args>(args)...) {
      }

      template<typename Object, typename R, typename ...Args>
      Decl(Object* object, R(Object::*func)(Args...), Args&&... args)
		  : mClosure(object, func, std::forward<Args>(args)...) {}

      template<typename Object, typename R, typename ...Args>
      Decl(const Object* object, R(Object::*func)(Args...) const, Args&&... args)
		  : mClosure(object, func, std::forward<Args>(args)...) {}

      void operator()() {
         return mClosure();
      }

      void execute() {
         return mClosure();
      }
   protected:
      closure mClosure;
   };

   class Counter {
   friend class Consumer;
   friend void chain(const S<Counter>& prerequisite, const S<Counter>& afterFinish);
   public:
      uint counter() const { return mDispatchCounter; }

   void terminate() { mIsDone = true; };

   Counter(S<Decl> decl, category_t cat)
         : mDecl(std::move(decl))
         , mCategory(cat) {
         mBlockees.fill(nullptr);
      }
      void decrementCounter() { --mDispatchCounter; }
      category_t category() const { return mCategory; }

      void dispatchBlockees() const;

      bool done() const { return mIsDone; }

   ~Counter();
   protected:
      void invoke();
      void reset();
      void addBlockee(const S<Counter>& counter);
      static std::atomic<counter_id_t> sNextId;

      S<Decl> mDecl = nullptr;
      category_t mCategory = CAT_GENERIC;
      std::atomic_uint mDispatchCounter = 1;
      std::array<S<Counter>, kMaxDependentJob> mBlockees; // I am blocking them
      std::atomic_size_t mBlockeeCount = 0;
      counter_id_t mId = sNextId++;
      bool mIsDone = false;
   };

   class Consumer {
   public:
      void init(span<category_t> categories);
      bool consume();
      bool consumeAll();
      void consumeFor(uint ms);
   protected:
      std::vector<category_t> mCategories;
   };
   void startup(uint categoryCount);
   void shutdown();
   bool running();

   /*
    * this api is thread specific
    */
   Counter* currentJob();
   S<Counter> create(Decl& decl, category_t cat);
   S<Counter> create(Decl&& decl, category_t cat);
   void dispatch(const S<Counter>& counter);
   W<Counter> dispatch(Decl& decl, category_t cat);
   W<Counter> dispatch(Decl&& decl, category_t cat);
   void chain(const S<Counter>& prerequisite, const S<Counter>& afterFinish);
   
   void wait(W<Counter> counter);
   bool ready();
}
