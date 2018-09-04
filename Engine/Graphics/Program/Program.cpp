#include "Engine/Graphics/RHI/RHI.hpp"
#include "Program.hpp"

static const std::string defaultShaderStr = R"(
float4 VSMain(float3 position : POSITION) {
  return float4(position, 1.f);
}
)";

Program::Program() {
  for(uint i = 0; i < NUM_SHADER_TYPE; i++) {
    mShaders[i].setType((eShaderType)i);
  }

  // stage(SHADER_TYPE_VERTEX).setFromString(defaultShaderStr, "VSMain");

  // same layout to material, for now, this is hard coded
  mLayout.addRange(DescriptorSet::Type::Cbv, 0, NUM_UNIFORM_SLOT);
  mLayout.addRange(DescriptorSet::Type::TextureSrv, TEXTURE_DIFFUSE, NUM_TEXTURE_SLOT);
}

void Program::setRenderState(const RenderState& state) {
  mIsDirty = true;
  mRenderState = state;
}

bool Program::compile() {
  if (!mIsDirty) return true;

  for(Shader& shader: mShaders) {
    shader.compile();
  }

  S<const RootSignature> rootSig;
  if(mShaders[SHADER_TYPE_COMPUTE].ready()) {
    rootSig = mShaders[SHADER_TYPE_COMPUTE].rootSignature();
  } else {
    rootSig = mShaders[SHADER_TYPE_VERTEX].rootSignature();
  }
     
  for(uint i = 0; i < NUM_SHADER_TYPE; ++i) {
    if (mShaders[i].empty()) continue;
    EXPECTS(    (rootSig == nullptr &&  nullptr == mShaders[i].rootSignature())
             || (*rootSig == *mShaders[i].rootSignature())
    );
  }

  mRootSig = rootSig;

  // grab information from shader reflection and construct layout

  mIsDirty = true;
  return true;
}

RenderState& Program::renderState() {
  mIsDirty = true;
  return mRenderState;
}

S<const RootSignature> Program::rootSignature() const {
  return mRootSig;
}

// void Program::setDescriptorLayout(const DescriptorSet::Layout& layout) {
//   mIsDirty = true;
//   mLayout = layout;
// }

