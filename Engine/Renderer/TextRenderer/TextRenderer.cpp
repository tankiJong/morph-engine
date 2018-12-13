#include "TextRenderer.hpp"

#include "Engine/Graphics/Program/Program.hpp"
#include "Engine/Graphics/Program/ProgramInst.hpp"
#include "Engine/Core/Resource.hpp"
TextRenderer::TextRenderer() {
  // Program::sptr_t prog = Resource<Program>::get()
  // auto progIns = GraphicsProgramInst::create(prog);
  // {
  //   {
  //     FrameBuffer::Desc desc;
  //     desc.defineColorTarget(0, TEXTURE_FORMAT_RGBA8);
  //     mGraphicDesc.setFboDesc(desc);
  //   }
  //
  //   mGraphicDesc.setProgram(progIns->prog());
  //   mGraphicDesc.setPrimTye(GraphicsState::PrimitiveType::Triangle);
  //   mGraphicDesc.setRootSignature(progIns->prog()->rootSignature());
  // }
}

void TextRenderer::draw(Mesh& mesh) {
 
}