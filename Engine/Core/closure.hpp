#pragma once

#include "Engine/Core/common.hpp"
#include "Delegate.hpp"
#include "Engine/Core/vary.hpp"

class arg_list {
public:
   static constexpr size_t kStorageSize = 92; // in byte

   template<typename ...Args>
   using storage_data_t = std::tuple<Args...>;
   using deleter_t = void(void*);

   template<typename ...Args>
   arg_list(Args&& ...args) {
      using data_t = storage_data_t<Args...>;
      mData.set<data_t>(data_t{std::forward<Args>(args)...});
   }

   template<typename ...Args>
	storage_data_t<Args...>& get() {
      return mData.get<storage_data_t<Args...>>();
   }

protected:
   vary mData;
};

class closure {
   public:
      template<typename ...Args, typename T, typename C = typename detail::closure_decay<T, void, Args...>::type>
      closure(T&& func, Args&&... args)
         : mArgs(std::forward<std::decay_t<Args>>(args)...) {
         static_assert(sizeof(C) <= sizeof(mFunction));
         if constexpr(std::is_convertible_v<C, void(*)(Args...)>) {
            using invoke_t = void(*)(Args...);
            new (&mFunction)invoke_t{func};
            mCallback = &invokeStatic<invoke_t, Args...>;
            // pure function, lambda without capture
         } else {
            new (&mFunction)C{std::forward<T>(func)};
            mCallback = &invokeStatic<C, std::decay_t<Args>...>;
         }
      }

      template<typename Object, typename R, typename ...Args>
      closure(Object* object, R(Object::*func)(Args...), Args&&... args)
	      : mObject(object)
         , mArgs(std::forward<std::decay_t<Args>>(args)...) {
         using invoke_t = R(Object::*)(Args...);
         new (&mFunction)invoke_t{func};
         mCallback = &invokeMember<Object, invoke_t, std::decay_t<Args>...>;
      }

      template<typename Object, typename R, typename ...Args>
      closure(const Object* object, R(Object::*func)(Args...) const, Args&&... args)
	      : mObject(const_cast<Object*>(object))
         , mArgs(std::forward<std::decay_t<Args>>(args)...) {
         using invoke_t = R(Object::*)(Args...) const;
         new (&mFunction)invoke_t{func};
         mCallback = &invokeMember<Object, invoke_t, std::decay_t<Args>...>;
      }

      void operator()() {
         callback_t* func = (callback_t*)mCallback;
         std::invoke(func, *this);
      }

      
   protected:
      using callback_t = void(closure&);
      
      callback_t* mCallback = nullptr;
      void* mObject = nullptr;

      struct {
         uint8_t storage[32];
      } mFunction; // 32 bytes

      arg_list mArgs; // 128 bytes

      template<typename Object, typename MemberFunc, typename ...Args, size_t ...Indices>
      static void tupleExecuteMember(Object& obj, MemberFunc& func, std::tuple<Args...>& args, std::index_sequence<Indices...>) {
         std::invoke(func, obj, std::get<Indices>(args)...);
      }

      template<typename C, typename ...Args, size_t ...Indices>
      static void tupleExecuteStatic(C&& func, std::tuple<Args...>& args, std::index_sequence<Indices...>) {
         std::invoke(func, std::get<Indices>(args)...);
      }

      template<typename Object, typename MemberFunc, typename ...Args>
      static void invokeMember(closure& decl) {
         MemberFunc* func = reinterpret_cast<MemberFunc*>(&decl.mFunction);
         arg_list::storage_data_t<Args...>& data = decl.mArgs.get<std::decay_t<Args>...>();
         tupleExecuteMember(*(Object*)decl.mObject, *func, data, 
			            std::make_index_sequence<std::tuple_size_v<std::tuple<Args...>>>());
      };

      template<typename C, typename ...Args>
      static void invokeStatic(closure& decl) {
         arg_list::storage_data_t<Args...>& data = decl.mArgs.get<Args...>();
         C* func = reinterpret_cast<C*>(&decl.mFunction);
         tupleExecuteStatic(*func, data, 
			            std::make_index_sequence<std::tuple_size_v<std::tuple<Args...>>>());
      }
   };
   // https://isocpp.org/wiki/faq/pointers-to-members#addr-of-memfn