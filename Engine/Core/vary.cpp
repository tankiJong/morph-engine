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
  : mMetaData(from.mMetaData) {
  static_assert(sizeof(mStorage) >= sizeof(mPtr));
  memcpy_s(&mStorage, sizeof(storage_t), &from.mStorage, sizeof(storage_t));
  from.mMetaData.reset();
}

vary::vary(const vary& from) 
  : mMetaData(from.mMetaData) {
  if(from.mMetaData.useHeap) {
    mPtr.size = from.mPtr.size;
    mPtr.value = malloc(mPtr.size);
    if(mMetaData.copyConstructor != nullptr) {
      mMetaData.copyConstructor(from.mPtr.value, mPtr.value);
	  }
  } else {
    if(mMetaData.copyConstructor != nullptr) {
      mMetaData.copyConstructor(&from.mStorage, &mStorage);
    }
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
  deleter = &defaultDelete;
  copyConstructor = &defaultCopy;
  useHeap = false;
  typeInfo = nullptr;
}

void vary::reset() {
  mMetaData.deleter(*this);
  mMetaData.reset();
}
