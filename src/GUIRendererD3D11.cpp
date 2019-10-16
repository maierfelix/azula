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

  GUIRendererD3D11::GUIRendererD3D11(GUIFrame* frame) {
    frame_ = frame;
    config.face_winding = ul::kFaceWinding_Clockwise;
    config.device_scale_hint = 1.0f;
    config.use_bgra_for_offscreen_rendering = true;

    gpu_context_ = std::make_unique<ul::GPUContextD3D11>();
    gpu_context_.get()->Initialize(NULL, 512, 512, 1.0, false, true, true, 4);
    gpu_driver_ = std::make_unique<ul::GPUDriverD3D11>(gpu_context_.get());
    font_loader = std::make_unique<ul::FontLoaderWin>();
    file_system = std::make_unique<ul::FileSystemWin>(L"");

    ul::Platform& platform = ul::Platform::instance();
    platform.set_config(config);
    platform.set_gpu_driver(gpu_driver_.get());
    platform.set_font_loader(font_loader.get());
    platform.set_file_system(file_system.get());

    renderer_ = ul::Renderer::Create();

    view_ = renderer_->CreateView(512, 512, true);
    view_->set_load_listener(this);
    view_->set_view_listener(this);

    view_->LoadHTML("");
    // called initially so we can take a shared handle immediately
    Update();
    Render();
  }

  ul::GPUContextD3D11* GUIRendererD3D11::gpu_context() { assert(gpu_context_); return gpu_context_.get();  }
  ul::GPUDriverD3D11* GUIRendererD3D11::gpu_driver() { assert(gpu_driver_); return gpu_driver_.get(); }

  void GUIRendererD3D11::Update() {
    renderer_->Update();
  }

  void GUIRendererD3D11::Render() {
    renderer_->Update();
    renderer_->Render();

    if (gpu_driver_->HasCommandsPending()) {
      gpu_context_->BeginDrawing();
      gpu_driver_->DrawCommandList();
      gpu_context_->EndDrawing();
    }
    UpdateGeometry();
    gpu_driver_->DrawGeometry(geometry_id, 6, 0, gpu_state);
  }

  void GUIRendererD3D11::UpdateGeometry() {
    bool initial_creation = false;
    ul::RenderTarget target = view_->render_target();

    if (vertices.empty()) {
      vertices.resize(4);
      indices.resize(6);

      int patternCW[] = { 0, 1, 3, 1, 2, 3 };
      int patternCCW[] = { 0, 3, 1, 1, 3, 2 };
      memcpy(indices.data(), patternCW, sizeof(int) * indices.size());

      memset(&gpu_state, 0, sizeof(gpu_state));
      ul::Matrix identity;
      identity.SetIdentity();
      gpu_state.viewport_width = 512;
      gpu_state.viewport_height = 512;
      gpu_state.transform = identity.GetMatrix4x4();
      gpu_state.enable_blend = true;
      gpu_state.enable_texturing = true;
      gpu_state.shader_type = ul::kShaderType_Fill;
      gpu_state.render_buffer_id = 1; // default render target view (screen)
      gpu_state.texture_1_id = target.texture_id;

      initial_creation = true;
    }

    if (!needs_update) return;

    ul::Vertex_2f_4ub_2f_2f_28f v;
    memset(&v, 0, sizeof(v));

    v.data0[0] = 1; // Fill Type: Image

    v.color[0] = 255;
    v.color[1] = 255;
    v.color[2] = 255;
    v.color[3] = 255;

    float left = 0.0f;
    float top = 0.0f;
    float right = 512;
    float bottom = 512;

    // TOP LEFT
    v.pos[0] = v.obj[0] = left;
    v.pos[1] = v.obj[1] = top;
    v.tex[0] = target.uv_coords.left;
    v.tex[1] = target.uv_coords.top;

    vertices[0] = v;

    // TOP RIGHT
    v.pos[0] = v.obj[0] = right;
    v.pos[1] = v.obj[1] = top;
    v.tex[0] = target.uv_coords.right;
    v.tex[1] = target.uv_coords.top;

    vertices[1] = v;

    // BOTTOM RIGHT
    v.pos[0] = v.obj[0] = right;
    v.pos[1] = v.obj[1] = bottom;
    v.tex[0] = target.uv_coords.right;
    v.tex[1] = target.uv_coords.bottom;

    vertices[2] = v;

    // BOTTOM LEFT
    v.pos[0] = v.obj[0] = left;
    v.pos[1] = v.obj[1] = bottom;
    v.tex[0] = target.uv_coords.left;
    v.tex[1] = target.uv_coords.bottom;

    vertices[3] = v;

    ul::VertexBuffer vbuffer;
    vbuffer.format = ul::kVertexBufferFormat_2f_4ub_2f_2f_28f;
    vbuffer.size = (uint32_t)(sizeof(ul::Vertex_2f_4ub_2f_2f_28f) * vertices.size());
    vbuffer.data = (uint8_t*)vertices.data();

    ul::IndexBuffer ibuffer;
    ibuffer.size = (uint32_t)(sizeof(ul::IndexType) * indices.size());
    ibuffer.data = (uint8_t*)indices.data();

    if (initial_creation) {
      geometry_id = gpu_driver_->NextGeometryId();
      gpu_driver_->CreateGeometry(geometry_id, vbuffer, ibuffer);
      return;
    }

    gpu_driver_->UpdateGeometry(geometry_id, vbuffer, ibuffer);
  }

}
