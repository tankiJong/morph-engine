#pragma once
#include "Engine/Core/common.hpp"
#include <atomic>

namespace Job {

   using job_type_t = uint16_t;
   using counter_id_t = uint32_t;
   constexpr uint kMaxDependentJob = 1024;
   constexpr uint kMaxWorker = 6;
   enum ePriority {
      PRI_LOW,
      PRI_NORM,
      PRI_HIGH,
      PRI_MOST,
      NUM_PRI,
   };

   enum eType: uint8_t {
      TYPE_GENERAL = 0,
      TYPE_GENERAL_SLOW,
      TYPE_IO,
   };

   class Counter;
   class Decl {
   };

   class Counter {
      S<Decl> mDecl;
      std::atomic_uint mDispatchCounter;
      std::array<Decl*, kMaxDependentJob> mDependents;
      static std::atomic<counter_id_t> sNextId;
   };

   void startup();
   void shutdown();

   // create a job, define the entry, user data
   S<Decl> create(uint8_t userType, eType systemType);

   W<Counter> dispatch(Decl& decl);

   void wait(W<Counter> counter);

}
