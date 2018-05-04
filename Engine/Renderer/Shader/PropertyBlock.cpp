#include "PropertyBlock.hpp"

property_info_t& PropertyBlockInfo::operator[](const std::string& name) {
  auto& block = mInfos[name];
  block.owningBlock = this;
  return block;
}

size_t typeSizeOf(std::string_view type) {
  if (type == "float") return 4;
  if (type == "vec4") return 4 * 4;
  ERROR_AND_DIE("undefined type");
}
