#include "BVH.hpp"
#include "Engine/Debug/ErrorWarningAssert.hpp"
#include <queue>
#include <stack>
#include "Engine/Debug/Draw.hpp"
#include "Engine/Math/Primitives/uvec2.hpp"
#include "Engine/Math/Range.hpp"
#include "Engine/Debug/Log.hpp"

const float& visit(const vec3& vec, BVH::eSortPolicy policy) {
  switch(policy) { 
    case BVH::SORT_X: 
    return vec.x;
    case BVH::SORT_Y: 
    return vec.y;
    case BVH::SORT_Z:
    return vec.z;
  }

  BAD_CODE_PATH();
}

const float& visit(const BVH::Prim& vec, BVH::eSortPolicy policy) {
  switch (policy) {
    case BVH::SORT_X:
      return vec.bounds.min.x;
    case BVH::SORT_Y:
      return vec.bounds.min.y;
    case BVH::SORT_Z:
      return vec.bounds.min.z;
  }

  BAD_CODE_PATH();
}
  
// BVH::Prim::Prim(const vertex_pcu_t& x, const vertex_pcu_t& y, const vertex_pcu_t& z) {
//   position[0] = x.position;
//   position[1] = y.position;
//   position[2] = z.position;
//
//   color[0] = x.color;
//   color[1] = y.color;
//   color[2] = z.color;
//
//   uv[0] = x.uvs;
//   uv[1] = y.uvs;
//   uv[2] = z.uvs;
//
//   bounds.grow(x.position);
//   bounds.grow(y.position);
//   bounds.grow(z.position);
// }

BVH::BVH(span<vec3> vertices, span<vec4> color, uint depth) {

  mPrims.reserve(vertices.size() / 3 + 1);

  EXPECTS(vertices.size() % 3 == 0);

  aabb3 initalBound;
  for(ptrdiff_t i = 0; i < vertices.size(); i+=3) {
    vec3 pos[3] = { vertices[i], vertices[i+1], vertices[i+2] };
    vec4 col[3] = { color[i], color[i+1], color[i+2] };
    Prim& p = mPrims.emplace_back(pos, col);
    initalBound.grow(p.bounds);
  }

  eSortPolicy policy = SORT_X;

  struct Job {
    Node* parent;
    uint depth;
    size_t startIndex; // Inclusive
    size_t endIndex; // exclusive
  };

  std::stack<Job> jobs;

  mNodes.reserve(uint64_t(2) << (depth + 2));

  {
    sort(mPrims, policy);
    
    size_t offset = alignedMedian(mPrims, policy);
    Node& root = mNodes.emplace_back();
    root.bounds = initalBound;
    root.indexInArray = 0;

    Job left = { &root, 1, 0, offset + 1};
    Job right = { &root, 1, offset + 1, mPrims.size()};

    jobs.push(right);
    jobs.push(left);
  }

  while(!jobs.empty()) {
    Job job = jobs.top();
    jobs.pop();

    EXPECTS(job.endIndex >= job.startIndex);

    span<Prim> prims = {};

    if(job.endIndex > job.startIndex) {
      prims =
        { mPrims.data() + job.startIndex, mPrims.data() + job.endIndex};
    }

    // create node
    Node& current = mNodes.emplace_back();
    current.depth = job.depth;

    current.bounds = computeBounds(prims);
    current.indexInArray = (uint)mNodes.size() - 1;
    // link back the node 
    EXPECTS(job.parent->marker == 0xabcdaabb);

    if(job.parent->left == nullptr) {
      // I am processing left
      job.parent->left = &current;
      job.parent->startIndex = (uint)job.startIndex;

      // I will expect the next node I pop out should be right side if I am trying to traverse back
      ENSURES(jobs.top().parent == job.parent);
    } else {
      EXPECTS(job.parent->left->marker == 0xabcdaabb);
      // I am processing right
      job.parent->right = &current;
      job.parent->elementCount = (uint)job.endIndex - job.parent->startIndex;
    }

    if (job.depth >= depth) {
      // I do not have more leafs, correctly set the offset data for my node
      current.startIndex = (uint)job.startIndex;
      current.elementCount = (uint)job.endIndex - (uint)job.startIndex;

      ENSURES(current.left == nullptr && current.right == nullptr);

      continue;
    }

    // sort the node according to current policy
    eSortPolicy currentPolicy = stepPolicy(policy, job.depth);
    sort(prims, currentPolicy);

    size_t offset = alignedMedian(prims, currentPolicy);

    // recursion
    {
      size_t middleIndex = min(job.startIndex + offset + 1, job.endIndex);
      Job left = { &current, job.depth + 1, job.startIndex, middleIndex };

      EXPECTS(middleIndex <= job.endIndex);
      Job right = { &current, job.depth + 1, middleIndex, job.endIndex };

      jobs.push(right);
      jobs.push(left);
    }
  }
}

BVH::Prim::Prim(const vec3 p[3], const vec4 c[3]) {
  memcpy(position, p, sizeof(vec3) * 3);
  memcpy(color, c, sizeof(vec4) * 3);

  bounds.grow(p[0]);
  bounds.grow(p[1]);
  bounds.grow(p[2]);

  ENSURES(bounds.contains(p[0]) && bounds.contains(p[1]) && bounds.contains(p[0]));
}

void BVH::render() {
  for(Node& node: mNodes) {
    // if (node.depth < 3) continue;
    Debug::drawCube(node.bounds, mat44::identity, false, Debug::INF, Rgba(255, 255, 255, 5));
    Debug::drawCube(node.bounds, mat44::identity, true, Debug::INF, Rgba(255, 255, 255, 5));
  }
}

void BVH::uploadNodesToGpu(S<TypedBuffer>& buffer) const {
  struct GPUNode {
    aabb3 bounds;
    struct {
      uint start; // inclusive
      uint end;   // exclusive
    } childRange;

    struct {
      uint start; // inclusive
      uint end;   // exclusive
    } triRange;
  };

  std::vector<GPUNode> nodes;
  nodes.reserve(mNodes.size());

  for(uint i = 0; i < mNodes.size(); i++) {
    const Node& node = mNodes[i];
    GPUNode gnode;
    gnode.bounds = node.bounds;

    EXPECTS(
      (node.left == nullptr && node.right == nullptr) || 
      (node.left != nullptr && node.right != nullptr));


    if(node.left == nullptr) {
      // no children
      gnode.childRange = { i ,i };
    } else {
      uint size = (uint)((mNodes.size() + 1u) >> (node.depth+1)) - 1u;
      ENSURES(node.left == &mNodes[i + 1]);
      ENSURES(node.right == &mNodes[i + 1 + size]);
      gnode.childRange = { i + 1, i + 1 + size + size };
    }

    EXPECTS(node.startIndex <= node.startIndex + node.elementCount);
    gnode.triRange = { node.startIndex, node.startIndex + node.elementCount };
    nodes.push_back(gnode);
  }

  
// #ifdef _DEBUG
//   std::vector<bool> flags(mPrims.size(), false);
//
//   for(const GPUNode& node: nodes) {
//     if(node.childRange.start != node.childRange.end) continue;
//     for(uint i = node.triRange.start; i < node.triRange.end; i++) {
//       flags[i] = true;
//     }
//   }
//
//   for(bool b: flags) {
//     Log::logf("%i", b);
//   }
// #endif

  buffer = TypedBuffer::For<GPUNode>((u32)nodes.size(), RHIResource::BindingFlag::ShaderResource | RHIResource::BindingFlag::UnorderedAccess);
  buffer->set<const GPUNode>(nodes);
  buffer->uploadGpu();
}

void BVH::uploadVerticesToGpu(S<TypedBuffer>& buffer) const {
  buffer = TypedBuffer::For<Prim>((u32)mPrims.size(), RHIResource::BindingFlag::ShaderResource | RHIResource::BindingFlag::UnorderedAccess);
  buffer->set<const Prim>(mPrims);
  buffer->uploadGpu();
}

aabb3 BVH::computeBounds(span<Prim> prims) {

  aabb3 bounds;

  for (auto& p: prims) {
    bounds.grow(p.bounds);
  }

  return bounds;
}

void BVH::sort(span<Prim> vectices, eSortPolicy policy) {
  const auto compx = [](const Prim& a, const Prim& b) {
    return a.bounds.min.x != b.bounds.min.x ?
      (a.bounds.min.x < b.bounds.min.x)
      : (a.bounds.min.y != b.bounds.min.y ?
      (a.bounds.min.y < b.bounds.min.y) : (a.bounds.min.z < b.bounds.min.z));
  };

  const auto compy = [](const Prim& a, const Prim& b) {
    return a.bounds.min.y != b.bounds.min.y ?
      (a.bounds.min.y < b.bounds.min.y)
      : (a.bounds.min.z != b.bounds.min.z ?
      (a.bounds.min.z < b.bounds.min.z) : (a.bounds.min.x < b.bounds.min.x));
  };

  const auto compz = [](const Prim& a, const Prim& b) {
    return a.bounds.min.z != b.bounds.min.z ?
      (a.bounds.min.z < b.bounds.min.z)
      : (a.bounds.min.x != b.bounds.min.x ?
      (a.bounds.min.x < b.bounds.min.x) : (a.bounds.min.y < b.bounds.min.y));
  };

  switch(policy) { 
    case SORT_X:
      std::sort(vectices.begin(), vectices.end(), compx);
    return;
    case SORT_Y: 
      std::sort(vectices.begin(), vectices.end(), compy);
    return;
    case SORT_Z: 
      std::sort(vectices.begin(), vectices.end(), compz);
    return;
  }

  BAD_CODE_PATH();
}

size_t BVH::alignedMedian(span<const Prim> vertices, eSortPolicy policy) {

  ptrdiff_t middle = vertices.size() / 2;

  if(middle <= 1) {
    return middle;
  }
  const Prim& ele = vertices.at(middle);

  ptrdiff_t back = middle, forward = middle;

  EXPECTS(back > 0 && forward < vertices.size() - 1);
  while(true) {
    back--;
    forward++;

    if (
         visit(vertices[back], policy) != visit(ele,policy)
      || forward == vertices.size() - 1) {
      return back;
    }

    if (
         visit(vertices[forward], policy) != visit(ele, policy)
      || back == 0) {
      return forward;
    }
  };
}

BVH::eSortPolicy BVH::stepPolicy(eSortPolicy policy, uint step) {
  return eSortPolicy((policy + step) % NUM_SORT);
}
