#pragma once
#include "Engine/Core/common.hpp"
#include "Engine/Renderer/Geometry/Vertex.hpp"

class Mesh;

struct ID3D12DescriptorHeap;
struct ID3D12RootSignature;
struct ID3D12CommandAllocator;
struct ID3D12GraphicsCommandList;
struct ID3D12Fence;

class RootSignature;
class PipelineState;

struct vertex_pc_t {
  vec3 position;
  vec4 color;
};

class Dx12CommandList {
  friend class Dx12Device;
public:
  void beginFrame();
  void drawVertexArray(span<vertex_pcu_t> verts);
  void drawMesh(const Mesh& mesh);
  void clearScreen();
  void afterFrame();
  
public:
  Dx12CommandList(S<Dx12Device> device);
  void waitForPreviousFrame();
  S<RootSignature> mRootSignature = nullptr;
  S<PipelineState> mPipelineState = nullptr;

  S<ID3D12CommandAllocator> mCommandAllocator = nullptr;
  S<ID3D12GraphicsCommandList> mCommandList = nullptr;
  
  u64 mFenceValue = 0;
  S<Dx12Device> mDevice;
};
