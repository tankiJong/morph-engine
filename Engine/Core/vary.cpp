#include "vary.hpp"

vary::~vary() {
  reset();
}

void vary::reset() {
  if(mDeleter != nullptr) {
    mDeleter(*this);
  }
}
