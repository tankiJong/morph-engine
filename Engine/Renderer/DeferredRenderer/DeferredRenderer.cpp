#include "DeferredRenderer.hpp"
#include "Engine/Renderer/Renderable/Renderable.hpp"
#include "Engine/Graphics/RHI/RHITexture.hpp"
#include "Engine/Application/Window.hpp"
#include "Engine/Graphics/RHI/RHIDevice.hpp"

/*

class CommandList;

struct TextureDesc {};
struct GraphResource {};

void DeferredRenderer::bind(const RenderScene& scene) {

  auto& gbufferPass = mRenderGraph.definePass( "GBuffer", [&](RenderGraphBuilder& builder) {
    
    TextureDesc desc;
    RHITexture::sptr_t gBufferColorTex = builder.createTexture(desc);
    GraphResource gBufferColor = builder.write("g-buffer", gBufferColorTex);

    RenderGraph gBufferRenderables;

    // construct forward render for each object
    char i = '0';
    {
      std::string name = "DrawObject-0";
      name.back() = i;
      Renderable* r;
      gBufferRenderables.definePass(name, [=](RenderGraphBuilder& builder) {
        builder.read(r->mesh()); // read vertex buffer, read index buffer;
        builder.read(r->material()); // read different material texture;

        return [=](CommandList& clist) {
          
          //TODO: I need somewhere to bind vbo, ibo, cbuffer, textures... basically apply material
          // TODO: then I need to set output to GBuffer

          // clist.draw(r.mesh);
        };
      });
    }

    builder.invoke(gBufferRenderables);

    // declare inputs: potentially draw calls
    // declare outputs: GBuffers textures

    return [=](CommandList& cmdList) {
      // do nothing
    };

  });


  auto deferredLighting = mRenderGraph.definePass("DeferredLighting", [&](RenderGraphBuilder& builder) {
    
    builder.read("g-buffer");
    builder.write("final", mFinalTexture);

    return [=](CommandList& list) {

    // TODO: I need somewhere to bind textures
    // TODO: then I need to set output to Final

      list.draw(0, 3);
    };
  });
  deferredLighting.depends(gbufferPass);

}
 */

void DeferredRenderer::setScene(RenderScene& scene) {
  mTargetScene = &scene;
}

void DeferredRenderer::onLoad(RHIContext&) {
  auto size = Window::Get()->bounds().size();
  uint width = (uint)size.x;
  uint height = (uint)size.y;


  mTFinal = Texture2::create( width, height, TEXTURE_FORMAT_RGBA8, 
                 RHIResource::BindingFlag::RenderTarget | RHIResource::BindingFlag::ShaderResource);
  NAME_RHIRES(mTFinal);

  mGAlbedo = Texture2::create( width, height, TEXTURE_FORMAT_RGBA8, 
                 RHIResource::BindingFlag::RenderTarget | RHIResource::BindingFlag::ShaderResource);
  NAME_RHIRES(mGAlbedo);

  mGPosition = Texture2::create( width, height, TEXTURE_FORMAT_RGBA16, 
                 RHIResource::BindingFlag::RenderTarget | RHIResource::BindingFlag::ShaderResource);
  NAME_RHIRES(mGPosition);

  mGNormal = Texture2::create( width, height, TEXTURE_FORMAT_RGBA16, 
                 RHIResource::BindingFlag::RenderTarget | RHIResource::BindingFlag::ShaderResource);
  NAME_RHIRES(mGNormal);

}

void DeferredRenderer::onRenderFrame(RHIContext& ctx) {

  mGDepth = RHIDevice::get()->depthBuffer();

  //---------
  ctx.transitionBarrier(mGAlbedo.get(), RHIResource::State::RenderTarget);
  ctx.transitionBarrier(mGPosition.get(), RHIResource::State::RenderTarget);
  ctx.transitionBarrier(mGNormal.get(), RHIResource::State::RenderTarget);
  ctx.transitionBarrier(mGDepth.get(), RHIResource::State::DepthStencil);
  cleanBuffers(ctx);

  //---------
  generateGbuffer(ctx);

  //---------
  ctx.transitionBarrier(mTFinal.get(), 
                        RHIResource::State::RenderTarget);
  ctx.transitionBarrier(mGAlbedo.get(), RHIResource::State::ShaderResource);
  ctx.transitionBarrier(mGPosition.get(), RHIResource::State::ShaderResource);
  ctx.transitionBarrier(mGNormal.get(), RHIResource::State::ShaderResource);
  ctx.transitionBarrier(mGDepth.get(), RHIResource::State::ShaderResource);
  deferredShading(ctx);


  //---------
  ctx.transitionBarrier(mTFinal.get(), RHIResource::State::CopySource);
  ctx.transitionBarrier(RHIDevice::get()->backBuffer().get(), RHIResource::State::CopyDest);
  copyToBackBuffer(ctx);
}

void DeferredRenderer::cleanBuffers(RHIContext& ctx) {
  ctx.clearRenderTarget(*mGAlbedo->rtv(), Rgba::white);
  ctx.clearRenderTarget(*mGPosition->rtv(), Rgba::black);
  ctx.clearRenderTarget(*mGNormal->rtv(), Rgba::gray);
  
  ctx.clearDepthStencilTarget(*mGDepth->dsv());
}

void DeferredRenderer::generateGbuffer(RHIContext& ctx) {
  SCOPED_GPU_EVENT(ctx, "Gen G-Buffer");

}

void DeferredRenderer::copyToBackBuffer(RHIContext& ctx) {
  ctx.copyResource(*mTFinal, *RHIDevice::get()->backBuffer());
}
