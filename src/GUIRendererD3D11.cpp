#define NAPI_EXPERIMENTAL
#include <napi.h>

#include "GUIRendererD3D11.h"

#include "GUIFrame.h"
#include "GPUDriverD3D11.h"
#include "GPUContextD3D11.h"

#include "FontLoaderWin.h"
#include "FileSystemWin.h"

#include <Ultralight/Ultralight.h>

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
