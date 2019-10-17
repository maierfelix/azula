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

  GUIRendererD3D11::GUIRendererD3D11(GUIFrame* frame): GUIRenderer(frame) {
    config.face_winding = ul::kFaceWinding_Clockwise;
    config.device_scale_hint = 1.0f;
    config.use_bgra_for_offscreen_rendering = true;

    font_loader_ = std::make_unique<ul::FontLoaderWin>();
    file_system_ = std::make_unique<ul::FileSystemWin>(L"");

    gpu_context_ = std::make_unique<ul::GPUContextD3D11>();
    gpu_context_.get()->Initialize(NULL, 512, 512, 1.0, false, true, true, 4);
    gpu_driver_ = std::make_unique<ul::GPUDriverD3D11>(gpu_context_.get());

    ul::Platform& platform = ul::Platform::instance();
    platform.set_config(config);
    platform.set_gpu_driver(gpu_driver_.get());
    Initialize(platform);
  }

  Napi::Value GUIRendererD3D11::GetSharedHandleD3D11(Napi::Env env) {
    HRESULT hr = S_OK;
    ComPtr<IDXGIResource1> resource;

    Flush();

    ul::GPUDriverD3D11* gpu_driver = (ul::GPUDriverD3D11*) gpu_driver_.get();

    uint32_t id = view()->render_target().render_buffer_id;
    hr = gpu_driver->GetResolveTexture(id)->QueryInterface(__uuidof(IDXGIResource1), (void**)&resource);
    if FAILED(hr) {
      MessageBoxW(NULL, (LPCWSTR)L"D3D11 Error: Failed to query interface", (LPCWSTR)L"D3D11 Error", MB_OK);
      return env.Undefined();
    }
    HANDLE outputHandle;
    hr = resource->CreateSharedHandle(nullptr, GENERIC_ALL, nullptr, &outputHandle);
    if FAILED(hr) {
      MessageBoxW(NULL, (LPCWSTR)L"D3D11 Error: Failed to create shared handle", (LPCWSTR)L"D3D11 Error", MB_OK);
      return env.Undefined();
    }

    return Napi::BigInt::New(env, reinterpret_cast<uintptr_t>(outputHandle));
  }

  ul::GPUContextD3D11* GUIRendererD3D11::gpu_context() { assert(gpu_context_); return gpu_context_.get(); }

  void GUIRendererD3D11::Flush() {
    gpu_context_->immediate_context()->Flush();
  }

  void GUIRendererD3D11::BeginDrawing() {
    gpu_context()->BeginDrawing();
  }

  void GUIRendererD3D11::EndDrawing() {
    gpu_context()->EndDrawing();
  }

}
