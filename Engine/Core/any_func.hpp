#pragma once

#include "Engine/Core/common.hpp"
#include "Engine/Debug/ErrorWarningAssert.hpp"
#include "Engine/Core/vary.hpp"

template<typename T>
struct function_t;

template<typename ...Ts>
struct func_params;

template<typename T, typename ...Rest>
struct func_params<T, Rest...> {
   using rest_func_param = func_params<Rest...>;
};

template<typename T>
struct func_params<T> {
   using rest_func_param = func_params<void>;
};

enum eFunctionType {
   FUNCTION_PTR,
   MEMBER_PTR,
   MEMBER_CONST_PTR,
   FUNCTOR,
};

template<typename R, typename ...Args>
struct function_t<R(Args...)> {
   using type = R(Args...);
   using return_t = R;

   template<size_t N>
   struct param {
      using type = typename std::tuple_element<N, std::tuple<Args...>>::type;
      static_assert(N < sizeof...(Args), "index out of range");
   };

   using param_pack_t = func_params<Args...>;
   
   template<size_t N>
   using param_t = typename param<N>::type;
   static constexpr bool is_member_function = false;
};

// function pointer
template<typename R, typename ...Args>
struct function_t<R(*)(Args...)>: public function_t<R(Args...)> {
   using type = R(*)(Args...);
   static constexpr eFunctionType function_type = FUNCTION_PTR;
   static constexpr bool is_member_function = false;
};

// member function
template<typename O, typename R, typename ...Args>
struct function_t<R(O::*)(Args...)>: public function_t<R(O&, Args...)> {
   using type = R(O::*)(Args...);
   using target_t = O;
   using param_pack_t = typename function_t<R(O&, Args...)>::param_pack_t::rest_func_param;
   static constexpr eFunctionType function_type = MEMBER_PTR;
   static constexpr bool is_member_function = true;
};

// const member function
template<typename O, typename R, typename ...Args>
struct function_t<R(O::*)(Args...) const>: public function_t<R(O&, Args...)> {
   using type = R(O::*)(Args...) const;
   using target_t = O;
   using param_pack_t = typename function_t<R(O&, Args...)>::param_pack_t::rest_func_param;
   static constexpr eFunctionType function_type = MEMBER_CONST_PTR;
   static constexpr bool is_member_function = true;
};

// functor, only works with non-operator()-overloading functors
template<typename F>
struct function_t {
private:
   using callable_t = function_t<decltype(&F::operator())>;
public:
   using type = F;
   using return_t = typename callable_t::return_t;
   using param_pack_t = typename callable_t::param_pack_t;
   static constexpr eFunctionType function_type = FUNCTOR;
   static constexpr bool is_member_function = false;

   template<size_t N>
   struct param {
      using type = typename callable_t::template param_t<N+1>; // cuz the first argument will be the object
      static_assert(N < sizeof...(param_pack_t), "index out of range");
   };
};

template<typename F>
using function_decay_t = function_t<std::decay_t<F>>;

class any_func {
private:
   template<typename T, 
            typename ...Args,
            typename F     = function_t<std::decay_t<T>>,
            typename C     = typename F::type, 
            typename R     = typename F::return_t>
   any_func(T&& func, const void* object, func_params<Args...>)
      : mArgInfo(&tid<std::decay_t<Args>...>::value)
      , mFunctionInfo(&tid<C>::value) {

      static_assert(sizeof(C) <= sizeof(mFunction));
      mObject = const_cast<void*>(object);

      mFunction.set<T, C>(std::forward<T>(func));

      if constexpr(F::function_type == FUNCTION_PTR) { 
         mCallback = &invokeStatic<C, std::decay_t<Args>...>;
      }
      if constexpr(F::function_type == MEMBER_PTR) { 
         mCallback = &invokeMember<F::target_t, C, std::decay_t<Args>...>;
      }
      if constexpr(F::function_type == MEMBER_CONST_PTR) { 
         mCallback = &invokeMember<F::target_t, C, std::decay_t<Args>...>;
      }
      if constexpr(F::function_type == FUNCTOR) {  
         mCallback = &invokeStatic<C, std::decay_t<Args>...>;
      }
   }

public:

   template<typename T, 
            typename = std::enable_if_t<!std::is_same_v<std::decay_t<T>, any_func>>>
   any_func(T&& func)
	   : any_func(std::forward<T>(func), nullptr, typename function_t<std::decay_t<T>>::param_pack_t()) {
      using F = function_t<std::decay_t<T>>;
      static_assert(!F::is_member_function);
   }

   template<typename T, 
            typename = std::enable_if_t<!std::is_same_v<std::decay_t<T>, any_func>>>
   any_func(const void* object, T&& func)
		: any_func(std::forward<T>(func), object, typename function_t<std::decay_t<T>>::param_pack_t()) {
      using F = function_t<std::decay_t<T>>;
      static_assert(F::is_member_function);
   }

   template<typename ...Args>
   void operator()(Args&&... args) const {
      EXPECTS(mArgInfo == &tid<std::decay_t<Args>...>::value);
      callback_t* func = (callback_t*)mCallback;

      std::tuple<Args&...> arguments(std::forward<Args>(args)...);
      std::invoke(func, *this, &arguments);
   }

   void operator()() const {
      EXPECTS(mArgInfo == &tid<void>::value);
      callback_t* func = (callback_t*)mCallback;
      std::invoke(func, *this, nullptr);
   }

   any_func(any_func&& func) noexcept;

   any_func(const any_func& func);

   template<typename T>
   bool operator==(T&& func) const {
      if constexpr(!std::is_same_v<std::decay_t<T>, any_func>) {
         using F = typename function_t<std::decay_t<T>>::type;
         if(&tid<F>::value != mFunctionInfo) return false;
         const F& storedFunc = mFunction.get<F>();
         return func == storedFunc;
      } else {
         return 
            (func.mCallback == mCallback) &&
            (func.mObject == mObject) &&
            (func.mArgInfo == mArgInfo) &&
            (func.mFunctionInfo == mFunctionInfo) &&
            (func.mFunction == mFunction);
      }
   }

   any_func& operator=(any_func&& func) noexcept {
      this->~any_func();
      new (this)any_func(func);
      return *this;
   }

   any_func& operator=(const any_func& func) {
      this->~any_func();
      new (this)any_func(func);
      return *this;
   }

protected:

   using callback_t = void(const any_func&, void*);
   callback_t* mCallback = nullptr;
   void* mObject = nullptr;
   const unique* mArgInfo;
   const unique* mFunctionInfo;
   vary mFunction;

   template<typename Object, typename MemberFunc, typename ...Args, size_t ...Indices>
   static void tupleExecuteMember(Object& obj, MemberFunc& func, std::tuple<Args...>& args, std::index_sequence<Indices...>) {
      std::invoke(func, obj, std::get<Indices>(args)...);
   }

   template<typename C, typename ...Args, size_t ...Indices>
   static void tupleExecuteStatic(C&& func, std::tuple<Args...>& args, std::index_sequence<Indices...>) {
      std::invoke(func, std::get<Indices>(args)...);
   }

   template<typename Object, typename MemberFunc, typename ...Args>
   static void invokeMember(const any_func& decl, void* args) {
      const MemberFunc& func = decl.mFunction.get<MemberFunc>();
      if constexpr((... && !std::is_same_v<Args, void>)) {
         std::tuple<Args&...>* arguments = (std::tuple<Args&...>*)args;

         tupleExecuteMember(*(Object*)decl.mObject, func,*arguments, 
			            std::make_index_sequence<std::tuple_size_v<std::tuple<Args&...>>>());
      } else {
         UNUSED(args);
         std::invoke(func, *(Object*)decl.mObject);
      }
   };

   template<typename C, typename ...Args>
   static void invokeStatic(const any_func& decl, void* args) {
      const C& func = decl.mFunction.get<C>();
      if constexpr((... && !std::is_same_v<Args, void>)) {
         std::tuple<Args&...>* arguments = (std::tuple<Args&...>*)args;

         tupleExecuteStatic(func, *arguments, 
			         std::make_index_sequence<std::tuple_size_v<std::tuple<Args&...>>>());
      } else {
         UNUSED(args);
         std::invoke(func);
      }

   }
};
