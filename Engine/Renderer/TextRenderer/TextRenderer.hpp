#pragma once
#include "Engine/Core/common.hpp"
#include "Engine/Graphics/RHI/RHIContext.hpp"
#include "Engine/Graphics/Font.hpp"
#include "Engine/Math/Primitives/vec3.hpp"
#include "Engine/Graphics/RHI/PipelineState.hpp"
#include "Engine/Core/Rgba.hpp"
#include "Engine/Graphics/Program/Material.hpp"
class Mesh;

class TextRenderer {
public:
  TextRenderer();
  void setContext(RHIContext& context);
  void setFont(const S<const Font>& font);
  void setFbo(const FrameBuffer& fbo);
  void setModelMatrix(const mat44 model = mat44::identity);
  void draw(std::string_view text, const vec3& pos, const Rgba& color = Rgba::white, const vec3& right = vec3::right, const vec3& up = vec3::up);
  void draw(Mesh& mesh);

protected:
  RHIContext::sptr_t mContext;
  S<const Font> mFont;
  GraphicsState::Desc mGraphicDesc;
  Material mMaterial;
};
