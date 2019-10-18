#define NAPI_EXPERIMENTAL
#include <napi.h>

#include "GUIRendererD3D11.h"

#include "GUIFrame.h"
#include "GPUDriverD3D11.h"
#include "GPUContextD3D11.h"

#include "FontLoaderWin.h"
#include "FileSystemWin.h"

#include <Ultralight/Ultralight.h>

#include <d3d11.h>
#include <dxgi1_2.h>
#include <wrl/client.h>

#include "utils.h"

namespace ul = ultralight;

using Microsoft::WRL::ComPtr;

namespace nodegui {

  GUIRendererD3D11::GUIRendererD3D11(GUIFrame* frame_): GUIRenderer(frame_) {
    config.face_winding = ul::kFaceWinding_Clockwise;
    config.device_scale_hint = 1.0f;
    if (frame()->use_offscreen_rendering) {
      config.use_bgra_for_offscreen_rendering = true;
    }

    font_loader_ = std::make_unique<ul::FontLoaderWin>();
    file_system_ = std::make_unique<ul::FileSystemWin>(L"");

    gpu_context_ = std::make_unique<ul::GPUContextD3D11>();
    if (frame()->use_offscreen_rendering) {
      gpu_context()->Initialize(NULL, frame()->initialWidth, frame()->initialHeight, 1.0, false, true, true, 4);
    }
    gpu_driver_ = std::make_unique<ul::GPUDriverD3D11>(gpu_context());

    ul::Platform& platform = ul::Platform::instance();
    platform.set_config(config);
    platform.set_gpu_driver(gpu_driver());
    Initialize(platform);
  }

  Napi::Value GUIRendererD3D11::GetSharedHandleD3D11(Napi::Env env) {
    HRESULT hr = S_OK;

    Update();
    Render();
    Flush();

    ul::GPUDriverD3D11* gpu_driver_d3d11 = (ul::GPUDriverD3D11*) gpu_driver();

    ComPtr<IDXGIResource1> resource;
    uint32_t id = view()->render_target().render_buffer_id;
    hr = gpu_driver_d3d11->GetResolveTexture(id)->QueryInterface(__uuidof(IDXGIResource1), (void**)&resource);
    if FAILED(hr) {
      Napi::Error::New(env, "D3D11 Error: Failed to query interface").ThrowAsJavaScriptException();
      return env.Undefined();
    }
    HANDLE outputHandle;
    hr = resource->CreateSharedHandle(nullptr, GENERIC_ALL, nullptr, &outputHandle);
    if FAILED(hr) {
      Napi::Error::New(env, "D3D11 Error: Failed to create shared handle").ThrowAsJavaScriptException();
      return env.Undefined();
    }

    return Napi::BigInt::New(env, reinterpret_cast<uintptr_t>(outputHandle));
  }

  void GUIRendererD3D11::Resize(uint32_t width, uint32_t height) {
    if (view()->width() == width && view()->height() == height) return;

    D3D11_VIEWPORT vp;
    ZeroMemory(&vp, sizeof(vp));
    vp.Width = (float) width;
    vp.Height = (float) width;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    gpu_context()->immediate_context()->RSSetViewports(1, &vp);

    view()->Resize(width, height);

    needs_update_ = true;
    UpdateGeometry();

    ul::RenderTarget target = view()->render_target();
    gpu_state_.render_buffer_id = view()->render_target().render_buffer_id;
    gpu_state_.texture_1_id = target.texture_id;
  }

  void GUIRendererD3D11::Flush() {
    gpu_context()->immediate_context()->Flush();
  }

  void GUIRendererD3D11::BeginDrawing() {
    gpu_context()->BeginDrawing();
  }

  void GUIRendererD3D11::EndDrawing() {
    gpu_context()->EndDrawing();
  }

}
