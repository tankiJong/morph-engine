#include "vary.hpp"

const void* vary::get() const {
  if(mMetaData.useHeap) {
    return mPtr.value;
  }

  return &mStorage;
}

void* vary::get() {
  if(mMetaData.useHeap) {
    return mPtr.value;
  }

  return &mStorage;
}

vary::vary(vary&& from) noexcept
  : mMetaData(from.mMetaData){
  memcpy_s(&mStorage, sizeof(storage_t), &from.mStorage, sizeof(storage_t));
  from.mMetaData.reset();
}

vary::vary(const vary& from) 
  : mMetaData(from.mMetaData) {
  if(from.mMetaData.useHeap) {
    mPtr.size = from.mPtr.size;
    mPtr.value = malloc(mPtr.size);
    mMetaData.copyConstructor(from.mPtr.value, mPtr.value);
  } else {
    mMetaData.copyConstructor(&from.mStorage, &mStorage);
  }
}

vary& vary::operator=(const vary& from) {
  reset();
  new (this)vary(from);
  return *this;
}

vary& vary::operator=(vary&& from) noexcept {
  reset();
  new (this)vary(from);
  return *this;
}

vary::~vary() {
  reset();
}

void vary::meta_data_t::reset() {
  deleter = nullptr;
  useHeap = false;
  typeInfo = nullptr;
}

void vary::reset() {
  if(mMetaData.deleter != nullptr) {
    mMetaData.deleter(*this);
  }
  mMetaData.reset();
}
