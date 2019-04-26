#pragma once

#include "Engine/Core/common.hpp"
#include "Delegate.hpp"

class arg_list {
public:
   static constexpr size_t kStorageSize = 92; // in byte

   template<typename ...Args>
   using storage_data_t = std::tuple<Args...>;
   using deleter_t = void(void*);

   template<typename ...Args>
   arg_list(Args&& ...args)
      : mTypeInfo(&tid<Args...>::value){
      using data_t = storage_data_t<Args...>;
      if constexpr (sizeof(data_t) >= kStorageSize) { // I need extra byte at the end as flag
         size_t dataSize = sizeof(data_t);
         *((size_t*)mPtr[2]) = dataSize;
         mPtr[0] = (uint8_t*)malloc(dataSize);
         new (mPtr[0]) data_t(std::forward<Args>(args)...);
         mPtr[1] = &deleter<Args...>;
         mStorage[kStorageSize - 1] = 1;
      } else {
         new (mStorage) data_t(std::forward<Args>(args)...);
         mStorage[kStorageSize - 1] = 0;
      }
   }

   arg_list(const arg_list& from);
   arg_list(arg_list&& from) noexcept;

   arg_list& operator=(arg_list&& rhs) noexcept;
   arg_list& operator=(const arg_list& rhs);
   template<typename ...Args>
	storage_data_t<Args...>& get() const {
      EXPECTS(*mTypeInfo == tid<Args...>::value);
      if(heapData()) {
         return *((storage_data_t<Args...>*)mPtr[0]);
      } else {
         return *((storage_data_t<Args...>*)mStorage);
      }
   }

   bool heapData() const { return mStorage[kStorageSize - 1] != 0; }
   size_t heapDataSize() const { return mStorage[kStorageSize - 1] != 0 ? *((size_t*)mPtr[2]) : 0; }
   ~arg_list();

protected:
   union {
      uint8_t mStorage[kStorageSize];
      void*   mPtr[3]; // [0] data ptr, [1] deleter ptr [2] data size
   };
   const unique* mTypeInfo;
   template<typename ...Args>
   static void deleter(void* ptr) {
      using data_t = std::tuple<Args...>;
      data_t* tuplePtr = (data_t*)ptr;
      tuplePtr->~DataType();
      free(ptr);
   }
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