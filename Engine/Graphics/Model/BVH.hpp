#pragma once

#include <vector>
#include "Engine/Core/common.hpp"
#include "Engine/Math/Primitives/vec3.hpp"
#include "Engine/Math/Primitives/aabb3.hpp"
#include "Engine/Math/Primitives/vec4.hpp"
#include "Engine/Math/Primitives/vec2.hpp"

struct vertex_pcu_t;
class RHIBuffer;
class TypedBuffer;
class BVH {
public:
  static constexpr size_t LEAF_NODE_MAX_SIZE = 5;
  struct Node {
    aabb3 bounds;
    Node* left = nullptr;
    Node* right = nullptr;

    uint startIndex = 0;
    uint elementCount = 0;

    uint depth = 0;
    uint marker = 0xabcdaabb;
    uint indexInArray;
  };
  struct Prim {
    vec3 position[3] = { vec3::zero, vec3::zero, vec3::zero };
    vec4 color[3] = { vec4::zero, vec4::zero, vec4::zero };
    vec2 uv[3] = { vec2::zero, vec2::zero, vec2::zero };
    aabb3 bounds;
    Prim(const vec3 position[3], const vec4 color[3]);
  };

  enum eSortPolicy: uint8_t {
    SORT_X = 0,
    SORT_Y = 1,
    SORT_Z = 2,
    NUM_SORT = 3,
  };


  BVH(span<vec3> vertices, span<vec4> color, uint depth);
  void render();

  void uploadNodesToGpu(S<TypedBuffer>& buffer) const;
  void uploadVerticesToGpu(S<TypedBuffer>& buffer) const;
protected:
  aabb3 computeBounds(span<Prim> span);
  static void sort(span<Prim> vectices, eSortPolicy policy);
  static size_t alignedMedian(span<const Prim> vertices, eSortPolicy policy);
  static eSortPolicy stepPolicy(eSortPolicy policy, uint step);
  std::vector<Node> mNodes;
  std::vector<Prim> mPrims;
};


