#pragma once

#define NAPI_EXPERIMENTAL
#include <napi.h>

#include "GUIRenderer.h"

#include "GPUDriverD3D11.h"
#include "GPUContextD3D11.h"

namespace ul = ultralight;

namespace nodegui {

  class GUIFrame;

  class GUIRendererD3D11 : public GUIRenderer {

  public:
    GUIRendererD3D11(GUIFrame* frame);
    ~GUIRendererD3D11() {};

    void Flush() override;
    void BeginDrawing() override;
    void EndDrawing() override;

    ul::GPUContextD3D11* gpu_context() { assert(gpu_context_); return gpu_context_.get(); }

    Napi::Value GetSharedHandleD3D11(Napi::Env env);

  private:
    std::unique_ptr<ul::GPUContextD3D11> gpu_context_;
    ul::Config config;

    uint32_t geometry_id;
    std::vector<ul::Vertex_2f_4ub_2f_2f_28f> vertices;
    std::vector<ul::IndexType> indices;

    ul::GPUState gpu_state;

    bool needs_update = true;
  };

}
