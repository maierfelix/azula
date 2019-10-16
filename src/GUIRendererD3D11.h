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

    void Update() override;
    void Render() override;
    void UpdateGeometry() override;

    ul::GPUContextD3D11* gpu_context();
    ul::GPUDriverD3D11* gpu_driver();

  private:
    std::unique_ptr<ul::GPUContextD3D11> gpu_context_;
    std::unique_ptr<ul::GPUDriverD3D11> gpu_driver_;

    ul::Config config;
    std::unique_ptr<ul::FontLoader> font_loader;
    std::unique_ptr<ul::FileSystem> file_system;

    uint32_t geometry_id;
    std::vector<ul::Vertex_2f_4ub_2f_2f_28f> vertices;
    std::vector<ul::IndexType> indices;

    ul::GPUState gpu_state;

    bool needs_update = true;
  };

}
