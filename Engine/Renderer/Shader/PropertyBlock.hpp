#pragma once
#include "Engine/Core/common.hpp"
#include <unordered_map>
#include "Engine/Renderer/UniformBuffer.hpp"

class PropertyBlockInfo;

struct property_info_t {
  PropertyBlockInfo* owningBlock;
  std::string name;
  size_t offset;
  size_t size;
  size_t count;
};


class PropertyBlockInfo {
public:
  const property_info_t* operator()(std::string_view name) const;
  const property_info_t* operator()(std::string_view name, uint index) const;

  // would protentially cause an insert if the entry does not exist.
  property_info_t& operator[](const std::string& name);

  static const PropertyBlockInfo invalid;
  size_t totalSize = 0;
  std::string name = "";
protected:
  std::unordered_map<std::string, property_info_t> mInfos;
};


class PropertyBlock : public UniformBuffer {
public:
  S<const PropertyBlockInfo> info = nullptr;
};

size_t typeSizeOf(std::string_view type);