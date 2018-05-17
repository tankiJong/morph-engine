#include "PropertyBlock.hpp"

const property_info_t* PropertyBlockInfo::get(std::string_view name) const {
  if (auto kv = mInfos.find(name); kv != mInfos.end()) {
    return &kv->second;
  } else {
    return nullptr;
  }
}

property_info_t& PropertyBlockInfo::operator[](const std::string& propName) {
  auto& block = mInfos[propName];
  block.owningBlock = this;
  return block;
}

size_t typeSizeOf(std::string_view type) {
  if (type == "float") return 4;
  if (type == "vec4") return 4 * 4;
  ERROR_AND_DIE("undefined type");
}
