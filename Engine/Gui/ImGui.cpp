#include "ImGui.hpp"
#include "Engine/Graphics/RHI/DescriptorSet.hpp"
#include "Engine/Graphics/RHI/RHIDevice.hpp"
#include "ThirdParty/imgui/examples/imgui_impl_dx12.h"
#include "ThirdParty/imgui/examples/imgui_impl_win32.h"
#include "Engine/Application/Window.hpp"
#include "Engine/Graphics/RHI/FrameBuffer.hpp"
#include "Engine/Renderer/ImmediateRenderer.hpp"
#include "Engine/Graphics/Camera.hpp"

static DescriptorSet::sptr_t imFontDescriptor;

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
void CALLBACK WndProc(UINT msg, WPARAM wParam, LPARAM lParam) {
  ImGui_ImplWin32_WndProcHandler(HWND(Window::Get()->getHandle()), msg, wParam, lParam);
}


void ImGui::startup() {
  IMGUI_CHECKVERSION();

  ImGui::CreateContext();

  ImGuiIO& io = ImGui::GetIO(); (void)io;
  // io.Fonts->AddFontFromFileTTF("Engine/font/FiraMono-Regular.ttf", 8.0f);
  io.Fonts->AddFontFromFileTTF("Engine/font/FiraMono-Regular.ttf", 16.0f);

  bool re1 = ImGui_ImplWin32_Init(Window::Get()->getHandle());
  ENSURES(re1);

  Window::Get()->addWinMessageHandler(WndProc);

  DescriptorSet::Layout layout;
  layout.addRange(DescriptorPool::Type::TextureSrv, 0, 1);

  const RHIDevice::sptr_t& device = RHIDevice::get();
  imFontDescriptor = DescriptorSet::create(device->gpuDescriptorPool(), layout);
  bool re2  = ImGui_ImplDX12_Init(
                  device->nativeDevice().Get(), 
                  int(device->FRAME_COUNT), toDXGIFormat(TEXTURE_FORMAT_RGBA8),
                      imFontDescriptor->cpuHandle(0), imFontDescriptor->gpuHandle(0));
  ENSURES(re2);

  ImGui::StyleColorsDark();


}


void ImGui::shutdown() {
  ImGui_ImplDX12_Shutdown();
}

void ImGui::beginFrame() {
  ImGui_ImplDX12_NewFrame();
  ImGui_ImplWin32_NewFrame();
  ImGui::NewFrame();
  ImGuizmo::BeginFrame();

  ImGuiIO& io = ImGui::GetIO(); (void)io;
  ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);

}

void ImGui::render() {
  RHIContext::sptr_t ctx = RHIDevice::get()->defaultRenderContext();
  SCOPED_GPU_EVENT(*ctx, "ImGui");

  auto& rt = RHIDevice::get()->backBuffer();
  FrameBuffer fbo;
  fbo.defineColorTarget(rt, 0);
  ctx->setFrameBuffer(fbo);
  ctx->bindDescriptorHeap();
  ctx->transitionBarrier(rt.get(), RHIResource::State::RenderTarget);

  Render();
  ImGui_ImplDX12_RenderDrawData(GetDrawData(), RHIDevice::get()->defaultRenderContext()->contextData()->commandList().Get());
}

void ImGui::gizmos(Camera& cam, Transform& target, ImGuizmo::OPERATION op) {
  mat44 view = cam.view();
  mat44 proj = cam.projection();

  ImGuizmo::SetOrthographic(false);

  mat44 transform = target.localToWorld();

  mat44 delta;
  ImGuizmo::Manipulate((float*)&view, (float*)&proj, op, ImGuizmo::WORLD, (float*)&transform);

  target.setWorldTransform(transform * delta);
}
