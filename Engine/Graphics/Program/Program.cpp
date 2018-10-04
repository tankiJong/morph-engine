#include "Engine/Graphics/RHI/RHI.hpp"
#include "Program.hpp"
#include "Engine/Graphics/Program/ProgramIns.hpp"

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

  auto& layout = mLayouts.emplace_back();

  layout.addRange(DescriptorSet::Type::Cbv, 0, NUM_UNIFORM_SLOT);
  layout.addRange(DescriptorSet::Type::TextureSrv, TEXTURE_DIFFUSE, NUM_TEXTURE_SLOT);
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

  // for now, all stages shall share the same root signature, which means layout would also be the same.
  S<const RootSignature> rootSig;
  if(mShaders[SHADER_TYPE_COMPUTE].ready()) {
    rootSig = mShaders[SHADER_TYPE_COMPUTE].rootSignature();

    span<const DescriptorSet::Layout> sets = mShaders[SHADER_TYPE_COMPUTE].descriptorLayouts();
    mLayouts.resize(sets.size());
    std::copy(sets.begin(), sets.end(), mLayouts.begin());

  } else {
    rootSig = mShaders[SHADER_TYPE_VERTEX].rootSignature();

    span<const DescriptorSet::Layout> sets = mShaders[SHADER_TYPE_VERTEX].descriptorLayouts();
    mLayouts.resize(sets.size());
    std::copy(sets.begin(), sets.end(), mLayouts.begin());
  }
     
  for(uint i = 0; i < NUM_SHADER_TYPE; ++i) {
    if (mShaders[i].empty()) continue;
    EXPECTS(    (rootSig == nullptr &&  nullptr == mShaders[i].rootSignature())
             || (*rootSig == *mShaders[i].rootSignature())
    );
  }

  mRootSig = rootSig;

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

