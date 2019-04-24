#include "closure.hpp"

arg_list::arg_list(const arg_list& from)
  : mTypeInfo(from.mTypeInfo) {
  memcpy_s(mStorage, kStorageSize, from.mStorage, kStorageSize);
  if(from.heapDataSize() > 0) {
    //mPtr[1] = from.mPtr[1];
    //mPtr[2] = from.mPtr[2];
    
    size_t size = (size_t)mPtr[2];
    mPtr[0] = malloc(size);

    memcpy_s(mPtr[0], size, from.mPtr[0], size);
  }
}

arg_list::arg_list(arg_list&& from) noexcept 
  : mTypeInfo(from.mTypeInfo){
  memcpy_s(mStorage, kStorageSize, from.mStorage, kStorageSize);
  from.mStorage[kStorageSize - 1] = 0;
}

arg_list& arg_list::operator=(arg_list&& rhs) noexcept {
  memcpy_s(mStorage, kStorageSize, rhs.mStorage, kStorageSize);
  rhs.mStorage[kStorageSize - 1] = 0;
  return *this;
}

arg_list& arg_list::operator=(const arg_list& rhs) {
  mTypeInfo = rhs.mTypeInfo;
   memcpy_s(mStorage, kStorageSize, rhs.mStorage, kStorageSize);
  if(rhs.heapDataSize() > 0) {
    //mPtr[1] = rhs.mPtr[1];
    //mPtr[2] = rhs.mPtr[2];
    
    size_t size = (size_t)mPtr[2];
    mPtr[0] = malloc(size);

    memcpy_s(mPtr[0], size, rhs.mPtr[0], size);
  }
  return *this;
}

arg_list::~arg_list() {
  if(heapData()) {
    deleter_t* del = (deleter_t*)mPtr[1];
    del(mPtr[0]);
  }
}
