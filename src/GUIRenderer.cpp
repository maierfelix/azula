#define NAPI_EXPERIMENTAL
#include <napi.h>

#include "GUIRenderer.h"
#include "GUIFrame.h"

namespace ul = ultralight;

namespace nodegui {

  GUIRenderer::GUIRenderer(GUIFrame* frame): frame_(frame) {

  }

  GUIFrame* GUIRenderer::frame() { assert(frame_); return frame_; }

  ul::RefPtr<ul::View> GUIRenderer::view() { assert(view_.get()); return view_.get(); }
  ul::RefPtr<ul::Renderer> GUIRenderer::renderer() { assert(renderer_.get()); return renderer_.get(); }

  ul::FontLoader* GUIRenderer::font_loader() { assert(font_loader_.get()); return font_loader_.get(); }
  ul::FileSystem* GUIRenderer::file_system() { assert(file_system_.get()); return file_system_.get(); }
  ul::GPUDriver* GUIRenderer::gpu_driver() { assert(gpu_driver_); return gpu_driver_.get(); }

  void GUIRenderer::OnDOMReady(ul::View* caller) {
    return frame_->OnDOMReady(caller);
  }

  void GUIRenderer::OnChangeCursor(ul::View* caller, ul::Cursor cursor) {
    return frame_->OnChangeCursor(caller, cursor);
  }

  void GUIRenderer::OnAddConsoleMessage(
    ul::View* caller,
    ul::MessageSource source,
    ul::MessageLevel level,
    const ul::String& message,
    uint32_t line_number,
    uint32_t column_number,
    const ul::String& source_id
  ) {
    return frame_->OnAddConsoleMessage(caller, source, level, message, line_number, column_number, source_id);
  }

  ul::JSValue GUIRenderer::DispatchBinaryBuffer(const ul::JSObject& thisObject, const ul::JSArgs& info) {
    return frame_->OnDispatchBinaryBuffer(thisObject, info);
  }

  void GUIRenderer::Initialize(ul::Platform& platform) {

    platform.set_font_loader(font_loader());
    platform.set_file_system(file_system());
    renderer_ = ul::Renderer::Create();

    view_ = renderer_->CreateView(512, 512, true);
    view_->set_load_listener(this);
    view_->set_view_listener(this);

    view_->LoadHTML("");
    // called initially so we can take a shared handle immediately
    Update();
    Render();
  }

  void GUIRenderer::Flush() {

  }

  void GUIRenderer::Update() {
    renderer_->Update();
  }

  void GUIRenderer::Render() {
    renderer_->Update();
    renderer_->Render();

    if (gpu_driver_->HasCommandsPending()) {
      BeginDrawing();
      gpu_driver_->DrawCommandList();
      EndDrawing();
    }
    UpdateGeometry();
    gpu_driver_->DrawGeometry(geometry_id, 6, 0, gpu_state);
  }

  void GUIRenderer::BeginDrawing() {

  }

  void GUIRenderer::EndDrawing() {

  }

  void GUIRenderer::UpdateGeometry() {
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
