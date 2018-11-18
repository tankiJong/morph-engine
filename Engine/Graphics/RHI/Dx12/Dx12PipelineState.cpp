#include "Engine/Graphics/RHI/PipelineState.hpp"
#include "Engine/Graphics/RHI/RHIDevice.hpp"
#include "Engine/File/Utils.hpp"
#include "Engine/Graphics/RHI/Shader.hpp"
#include "Engine/Graphics/Program/Program.hpp"
#include "Engine/Graphics/Model/Vertex.hpp"

void setDepthStencilState(D3D12_DEPTH_STENCIL_DESC& dsv, const RenderState& rs) {
  dsv.DepthEnable = TRUE;
  
  switch(rs.depthMode) { 
    case COMPARE_NEVER:
      dsv.DepthFunc = D3D12_COMPARISON_FUNC_NEVER;
    break;
    case COMPARE_LESS:
      dsv.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
    break;
    case COMPARE_LEQUAL:
      dsv.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
    break;
    case COMPARE_GREATER:
      dsv.DepthFunc = D3D12_COMPARISON_FUNC_GREATER;
    break;
    case COMPARE_GEQUAL:
      dsv.DepthFunc = D3D12_COMPARISON_FUNC_GREATER_EQUAL;
    break;
    case COMPARE_EQUAL:
      dsv.DepthFunc = D3D12_COMPARISON_FUNC_EQUAL;
    break;
    case COMPARE_NOT_EQUAL:
      dsv.DepthFunc = D3D12_COMPARISON_FUNC_NOT_EQUAL;
    break;
    case COMPARE_ALWAYS:
      dsv.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS;
    break;
    case NUM_COMPARE:
    default: 
      BAD_CODE_PATH();
  }

  switch(rs.isWriteDepth) { 
    case FLAG_FALSE:
      dsv.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
    break;
    case FLAG_TRUE:
      dsv.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    break;
    default:
      BAD_CODE_PATH();
  }


}

void setFboDesc(D3D12_GRAPHICS_PIPELINE_STATE_DESC& desc, FrameBuffer::Desc& fboDesc, const RenderState& renderState) {
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
     desc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

     setDepthStencilState(desc.DepthStencilState, renderState);
     desc.DepthStencilState.StencilEnable = TRUE;
     
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

void setRasterizerState(const RenderState& renderState, D3D12_RASTERIZER_DESC& rs) {

  switch(renderState.fillMode) { 
    case FILL_SOLID:
      rs.FillMode = D3D12_FILL_MODE_SOLID;
    break;
    case FILL_WIRE:
      rs.FillMode = D3D12_FILL_MODE_WIREFRAME;
    break;
    case NUM_FILL_MODE:
    default: 
      BAD_CODE_PATH();
  }

  switch(renderState.cullMode) { 
    case CULL_BACK:
      rs.CullMode = D3D12_CULL_MODE_BACK;
    break;
    case CULL_FRONT: 
      rs.CullMode = D3D12_CULL_MODE_FRONT;
    break;
    case CULL_NONE:
      rs.CullMode = D3D12_CULL_MODE_NONE;
    break;
    case NUM_CULL_MODE:
    default:
      BAD_CODE_PATH();

  }

  switch(renderState.frontFace) { 
    case WIND_CLOCKWISE:
      rs.FrontCounterClockwise = FALSE;
    break;
    case WIND_COUNTER_CLOCKWISE:
      rs.FrontCounterClockwise = TRUE;
    break;
    case NUM_WIND_ORDER: 
    default:
      BAD_CODE_PATH();
  }

  rs.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
  rs.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
  rs.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;

  
  rs.DepthClipEnable = renderState.alphaBlendOp == COMPARE_ALWAYS ? FALSE : TRUE;
  rs.MultisampleEnable = TRUE;
  rs.AntialiasedLineEnable = TRUE;
  rs.ForcedSampleCount = 0;
  rs.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
}

D3D12_BLEND toDx12Blend(eBlendFactor f) {
  switch(f) { 
    case BLEND_F_ONE:
    return D3D12_BLEND_ONE;
    case BLEND_F_ZERO: 
    return D3D12_BLEND_ZERO;
    case BLEND_F_SRC_ALPHA: 
    return D3D12_BLEND_SRC_ALPHA;
    case BLEND_F_DST_ALPHA: 
    return D3D12_BLEND_DEST_ALPHA;
    case BLEND_F_INV_SRC_ALPHA: 
    return D3D12_BLEND_SRC_ALPHA;
    case BLEND_F_INV_DST_ALPHA: 
    return D3D12_BLEND_INV_DEST_ALPHA;
  }
  BAD_CODE_PATH();
}

D3D12_BLEND_OP toDx12BlendOp(eBlendOp op) {
  switch(op) {
    case BLEND_OP_ADD:
    return D3D12_BLEND_OP_ADD;
    case BLEND_OP_SUB: 
    return D3D12_BLEND_OP_SUBTRACT;
    case BLEND_OP_REV_SUB: 
    return D3D12_BLEND_OP_REV_SUBTRACT;
    case BLEND_OP_MIN: 
    return D3D12_BLEND_OP_MIN;
    case BLEND_OP_MAX: 
    return D3D12_BLEND_OP_MAX;
  }
  BAD_CODE_PATH();
}

void setDx12BlendState(const RenderState& rs, D3D12_RENDER_TARGET_BLEND_DESC& bs) {
  bs = {
    FALSE,FALSE,
    D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
    D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
    D3D12_LOGIC_OP_NOOP,
    D3D12_COLOR_WRITE_ENABLE_ALL,
  };

  if(rs.alphaBlendOp == BLEND_OP_DISABLE || rs.colorBlendOp == BLEND_OP_DISABLE) {
    EXPECTS(rs.alphaBlendOp == rs.colorBlendOp);
    bs.BlendEnable = FALSE;
    return;
  }

  bs.BlendEnable = TRUE;
  bs.LogicOpEnable = FALSE;

  bs.SrcBlend = toDx12Blend(rs.colorSrcFactor);
  bs.DestBlend = toDx12Blend(rs.colorDstFactor);
  bs.BlendOp = toDx12BlendOp(rs.colorBlendOp);

  bs.SrcBlendAlpha = toDx12Blend(rs.alphaSrcFactor);
  bs.DestBlendAlpha = toDx12Blend(rs.alphaDstFactor);
  bs.BlendOpAlpha = toDx12BlendOp(rs.alphaBlendOp);

}
bool GraphicsState::rhiInit() {
  D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = {};


  // hard coded shader
  // ID3DBlob* vertexShader = nullptr;
  // ID3DBlob* pixelShader = nullptr;

// #if defined(_DEBUG)
//   // Enable better shader debugging with the graphics debugging tools.
//   UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
// #else
//   UINT compileFlags = 0;
// #endif

  // d3d_call(D3DCompileFromFile(shaderPath.c_str(), nullptr, nullptr, "VSMain", "vs_5_0", compileFlags, 0, &vertexShader, nullptr));
  // d3d_call(D3DCompileFromFile(shaderPath.c_str(), nullptr, nullptr, "PSMain", "ps_5_0", compileFlags, 0, &pixelShader, nullptr));

  // ID3D12ShaderReflection* reflactor = nullptr;
  // HRESULT re = D3DReflect(pixelShader->GetBufferPointer(), pixelShader->GetBufferSize(), IID_PPV_ARGS(&reflactor));
  //
  // D3D12_SHADER_INPUT_BIND_DESC bindingDesc;
  // D3D12_SHADER_DESC shaderDesc;
  // reflactor->GetDesc(&shaderDesc);
  // for (uint i = 0; i < shaderDesc.BoundResources; i++) {
  //   reflactor->GetResourceBindingDesc(i, &bindingDesc);
  // }
  // auto const0 = reflactor->GetResourceBindingDesc(0, &bindingDesc);

  D3D12_SHADER_BYTECODE VS, PS;
  VS.pShaderBytecode = mDesc.mProgram->stage(SHADER_TYPE_VERTEX).handle();
  VS.BytecodeLength = mDesc.mProgram->stage(SHADER_TYPE_VERTEX).size();
  PS.pShaderBytecode = mDesc.mProgram->stage(SHADER_TYPE_FRAGMENT).handle();
  PS.BytecodeLength = mDesc.mProgram->stage(SHADER_TYPE_FRAGMENT).size();

  desc.VS = VS;
  desc.PS = PS;

  TODO("set up render state, rtv");

  // default rasterizer

  // default Blend
  D3D12_BLEND_DESC blendDesc;
  blendDesc.AlphaToCoverageEnable = FALSE;
  blendDesc.IndependentBlendEnable = FALSE;

  D3D12_RENDER_TARGET_BLEND_DESC defaultRenderTargetBlendDesc;
  setDx12BlendState(mDesc.mProgram->renderState(), defaultRenderTargetBlendDesc);

  for (UINT i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
    blendDesc.RenderTarget[i] = defaultRenderTargetBlendDesc;


  setDx12InputLayout(desc, *mDesc.mLayout);
  setRasterizerState(mDesc.mProgram->renderState(), desc.RasterizerState);
  setFboDesc(desc, mDesc.mFboDesc, mDesc.mProgram->renderState());
  desc.SampleMask = mDesc.mSampleMask;
  desc.pRootSignature = mDesc.mRootSignature ? mDesc.mRootSignature->handle().Get() : nullptr;

  desc.BlendState = blendDesc;

  switch(mDesc.mPrimType) { 
    case PrimitiveType::Point: 
      desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
    break;
    case PrimitiveType::Line: 
      desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
    break;
    case PrimitiveType::Triangle: 
      desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    break;
    case PrimitiveType::Patch:
    case PrimitiveType::Undefined:
    default: 
    BAD_CODE_PATH();
  }
  desc.SampleDesc.Count = 1;

  d3d_call(RHIDevice::get()->nativeDevice()->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(&mRhiHandle)));
  return true;
}

ComputeState::sptr_t ComputeState::create(const Desc& desc) {
  sptr_t pps = sptr_t(new ComputeState(desc));

  if (!pps->rhiInit()) return nullptr;

  return pps;
}

bool ComputeState::rhiInit() {
  D3D12_COMPUTE_PIPELINE_STATE_DESC desc = {};


  D3D12_SHADER_BYTECODE CS;
  CS.pShaderBytecode = mDesc.mProg->stage(SHADER_TYPE_COMPUTE).handle();
  CS.BytecodeLength = mDesc.mProg->stage(SHADER_TYPE_COMPUTE).size();

  desc.CS = CS;

  desc.pRootSignature = mDesc.mRootSignature ? mDesc.mRootSignature->handle().Get() : nullptr;
  d3d_call(RHIDevice::get()->nativeDevice()->CreateComputePipelineState(&desc, IID_PPV_ARGS(&mRhiHandle)));
  return true;
}
