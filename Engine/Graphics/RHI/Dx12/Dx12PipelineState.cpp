#include "Engine/Graphics/RHI/PipelineState.hpp"
#include "Engine/Graphics/RHI/RHIDevice.hpp"
#include "Engine/File/Utils.hpp"

void setFboDesc(D3D12_GRAPHICS_PIPELINE_STATE_DESC& desc, FrameBuffer::Desc& fboDesc) {
  static_assert(FrameBuffer::NUM_MAX_COLOR_TARGET <= 8);
  uint numRtv = 0;
  for(uint i = 0; i<FrameBuffer::NUM_MAX_COLOR_TARGET; i++) {
    desc.RTVFormats[i] = toDXGIFormat(fboDesc.colorTargetFormat(i));
    if(desc.RTVFormats[i] != DXGI_FORMAT_UNKNOWN) {
      numRtv = i + 1;
    }
  }
  desc.NumRenderTargets = numRtv;

   if(fboDesc.depthTargetFormat() != TEXTURE_FORMAT_UNKNOWN) {
     desc.DSVFormat = toDXGIFormat(fboDesc.depthTargetFormat());
     desc.DepthStencilState.DepthEnable = TRUE;
     desc.DepthStencilState.StencilEnable = TRUE;
  
     desc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
     desc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
     
     desc.DepthStencilState.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
     desc.DepthStencilState.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;
  
     D3D12_DEPTH_STENCILOP_DESC face;
     face.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
     face.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
     face.StencilPassOp = D3D12_STENCIL_OP_KEEP;
     face.StencilFailOp = D3D12_STENCIL_OP_KEEP;
     desc.DepthStencilState.FrontFace = face;
     desc.DepthStencilState.BackFace = face;
  
   } else {
     desc.DepthStencilState.DepthEnable = FALSE;
     desc.DepthStencilState.StencilEnable = FALSE;
   }
}



void setDx12InputLayout(D3D12_GRAPHICS_PIPELINE_STATE_DESC& desc, const VertexLayout& layout) {
  D3D12_INPUT_LAYOUT_DESC inputDesc;
  auto attrs = layout.attributes();

  D3D12_INPUT_ELEMENT_DESC* eles = new D3D12_INPUT_ELEMENT_DESC[attrs.size()];

  for (uint i = 0; i < attrs.size(); ++i) {
    D3D12_INPUT_ELEMENT_DESC& ele = eles[i];
    const VertexAttribute& attr = attrs[i];
    ele.SemanticName = attr.name.c_str();
    ele.SemanticIndex = 0;
    ele.Format = toDXGIFormat(attr.type, attr.count, attr.isNormalized);
    ele.InputSlot = attr.streamIndex;
    ele.AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
    ele.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
    ele.InstanceDataStepRate = 0;
  }

  desc.InputLayout = { eles, (uint)attrs.size() };
}

bool PipelineState::rhiInit() {
  static uint i = 0;
  D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = {};


  // hard coded shader
  ID3DBlob* vertexShader = nullptr;
  ID3DBlob* pixelShader = nullptr;

#if defined(_DEBUG)
  // Enable better shader debugging with the graphics debugging tools.
  UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
  UINT compileFlags = 0;
#endif
  fs::path shaderPath;
  if(i == 0) {
    shaderPath = fs::absolute("shaders.hlsl");
    i++;
  } else {
    shaderPath = fs::absolute("ssao.hlsl");
  }
  d3d_call(D3DCompileFromFile(shaderPath.c_str(), nullptr, nullptr, "VSMain", "vs_5_0", compileFlags, 0, &vertexShader, nullptr));
  d3d_call(D3DCompileFromFile(shaderPath.c_str(), nullptr, nullptr, "PSMain", "ps_5_0", compileFlags, 0, &pixelShader, nullptr));

  D3D12_SHADER_BYTECODE VS, PS;
  VS.pShaderBytecode = vertexShader->GetBufferPointer();
  VS.BytecodeLength = vertexShader->GetBufferSize();
  PS.pShaderBytecode = pixelShader->GetBufferPointer();
  PS.BytecodeLength = pixelShader->GetBufferSize();

  desc.VS = VS;
  desc.PS = PS;

  TODO("set up render state, rtv");

  // default rasterizer
  D3D12_RASTERIZER_DESC rasterizerDesc;
  rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
  rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
  rasterizerDesc.FrontCounterClockwise = FALSE;
  rasterizerDesc.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
  rasterizerDesc.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
  rasterizerDesc.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
  rasterizerDesc.DepthClipEnable = TRUE;
  rasterizerDesc.MultisampleEnable = FALSE;
  rasterizerDesc.AntialiasedLineEnable = FALSE;
  rasterizerDesc.ForcedSampleCount = 0;
  rasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

  // default Blend
  D3D12_BLEND_DESC blendDesc;
  blendDesc.AlphaToCoverageEnable = FALSE;
  blendDesc.IndependentBlendEnable = FALSE;
  const D3D12_RENDER_TARGET_BLEND_DESC defaultRenderTargetBlendDesc =
  {
    FALSE,FALSE,
    D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
    D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
    D3D12_LOGIC_OP_NOOP,
    D3D12_COLOR_WRITE_ENABLE_ALL,
  };

  for (UINT i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
    blendDesc.RenderTarget[i] = defaultRenderTargetBlendDesc;


  setDx12InputLayout(desc, *mDesc.mLayout);
  setFboDesc(desc, mDesc.mFboDesc);
  desc.SampleMask = mDesc.mSampleMask;
  desc.pRootSignature = mDesc.mRootSignature ? mDesc.mRootSignature->handle() : nullptr;

  desc.RasterizerState = rasterizerDesc;
  desc.BlendState = blendDesc;
  desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
  desc.SampleDesc.Count = 1;

  d3d_call(RHIDevice::get()->nativeDevice()->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(&mRhiHandle)))
  return true;
}
