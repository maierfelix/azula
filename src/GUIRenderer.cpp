#define NAPI_EXPERIMENTAL
#include <napi.h>

#include "GUIRenderer.h"
#include "GUIFrame.h"

namespace ul = ultralight;

namespace nodegui {

  GUIRenderer::GUIRenderer(GUIFrame* frame_): frame_(frame_) {
    // create a window if offscreen rendering is not enabled
    if (!frame()->use_offscreen_rendering) {
      app_ = ul::App::Create();

      window_ = ul::Window::Create(app()->main_monitor(), frame()->width, frame()->height, false, ul::kWindowFlags_Titled | ul::kWindowFlags_Resizable);
      app()->set_window(*window_.get());

      overlay_ = ul::Overlay::Create(*window_.get(), window_->width(), window_->height(), 0, 0);
      overlay()->view()->LoadHTML("");
      overlay()->view()->set_load_listener(this);
      overlay()->view()->set_view_listener(this);

      window_->set_listener(this);
      app()->set_is_running(true);
    }
  }

  void GUIRenderer::OnDOMReady(ul::View* caller) {
    return frame()->OnDOMReady(caller);
  }

  void GUIRenderer::OnChangeCursor(ul::View* caller, ul::Cursor cursor) {
    return frame()->OnChangeCursor(caller, cursor);
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
    return frame()->OnAddConsoleMessage(caller, source, level, message, line_number, column_number, source_id);
  }

  void GUIRenderer::OnClose() {

  }

  void GUIRenderer::OnResize(uint32_t width, uint32_t height) {
    frame()->OnResize(width, height);
    overlay()->Resize(width, height);
  }

  ul::JSValue GUIRenderer::DispatchBinaryBuffer(const ul::JSObject& thisObject, const ul::JSArgs& info) {
    return frame()->OnDispatchBinaryBuffer(thisObject, info);
  }

  void GUIRenderer::Initialize(ul::Platform& platform) {
    platform.set_font_loader(font_loader());
    platform.set_file_system(file_system());
    renderer_ = ul::Renderer::Create();

    view_ = renderer_->CreateView(frame()->width, frame()->height, true);
    view()->set_load_listener(this);
    view()->set_view_listener(this);

    view()->LoadHTML("");
    // called initially so we can take a shared handle immediately
    Update();
    Render();
  }

  void GUIRenderer::Flush() {
    if (!frame()->use_offscreen_rendering) return;
  }

  void GUIRenderer::Update() {
    if (frame()->use_offscreen_rendering) {
      renderer()->Update();
    }
    else {
      app()->UpdateTick();
    }
  }

  void GUIRenderer::Render() {
    if (!frame()->use_offscreen_rendering) return;
    renderer()->Update();
    renderer()->Render();
    if (gpu_driver()->HasCommandsPending()) {
      BeginDrawing();
      gpu_driver()->DrawCommandList();
      EndDrawing();
    }
    UpdateGeometry();
    gpu_driver()->DrawGeometry(geometry_id_, 6, 0, gpu_state_);
  }

  void GUIRenderer::BeginDrawing() {

  }

  void GUIRenderer::EndDrawing() {

  }

  std::string GUIRenderer::GetTitle() {
    if (frame()->use_offscreen_rendering) return "";
    return title_;
  }

  void GUIRenderer::SetTitle(std::string& title) {
    if (frame()->use_offscreen_rendering) return;
    title_ = title;
    window()->SetTitle(title_.c_str());
  }

  void GUIRenderer::LoadFile(std::string& path) {
    std::string filePath = "file:///" + path;
    if (frame()->use_offscreen_rendering) {
      view()->LoadURL(filePath.c_str());
    }
    else {
      overlay()->view()->LoadURL(filePath.c_str());
    }
  }

  void GUIRenderer::LoadHTML(std::string& html) {
    if (frame()->use_offscreen_rendering) {
      view()->LoadHTML(html.c_str());
    }
    else {
      overlay()->view()->LoadHTML(html.c_str());
    }
  }

  void GUIRenderer::UpdateGeometry() {
    if (!frame()->use_offscreen_rendering) return;

    bool initial_creation = false;
    ul::RenderTarget target = view()->render_target();

    if (vertices_.empty()) {
      vertices_.resize(4);
      indices_.resize(6);

      int patternCW[] = { 0, 1, 3, 1, 2, 3 };
      int patternCCW[] = { 0, 3, 1, 1, 3, 2 };
      memcpy(indices_.data(), patternCW, sizeof(int) * indices_.size());

      memset(&gpu_state_, 0, sizeof(gpu_state_));
      ul::Matrix identity;
      identity.SetIdentity();
      gpu_state_.viewport_width = frame()->width;
      gpu_state_.viewport_height = frame()->height;
      gpu_state_.transform = identity.GetMatrix4x4();
      gpu_state_.enable_blend = true;
      gpu_state_.enable_texturing = true;
      gpu_state_.shader_type = ul::kShaderType_Fill;
      gpu_state_.render_buffer_id = 1; // default render target view (screen)
      gpu_state_.texture_1_id = target.texture_id;

      initial_creation = true;
    }

    if (!needs_update_) return;

    ul::Vertex_2f_4ub_2f_2f_28f v;
    memset(&v, 0, sizeof(v));

    v.data0[0] = 1; // Fill Type: Image

    v.color[0] = 255;
    v.color[1] = 255;
    v.color[2] = 255;
    v.color[3] = 255;

    float left = 0.0f;
    float top = 0.0f;
    float right = frame()->width;
    float bottom = frame()->height;

    // TOP LEFT
    v.pos[0] = v.obj[0] = left;
    v.pos[1] = v.obj[1] = top;
    v.tex[0] = target.uv_coords.left;
    v.tex[1] = target.uv_coords.top;

    vertices_[0] = v;

    // TOP RIGHT
    v.pos[0] = v.obj[0] = right;
    v.pos[1] = v.obj[1] = top;
    v.tex[0] = target.uv_coords.right;
    v.tex[1] = target.uv_coords.top;

    vertices_[1] = v;

    // BOTTOM RIGHT
    v.pos[0] = v.obj[0] = right;
    v.pos[1] = v.obj[1] = bottom;
    v.tex[0] = target.uv_coords.right;
    v.tex[1] = target.uv_coords.bottom;

    vertices_[2] = v;

    // BOTTOM LEFT
    v.pos[0] = v.obj[0] = left;
    v.pos[1] = v.obj[1] = bottom;
    v.tex[0] = target.uv_coords.left;
    v.tex[1] = target.uv_coords.bottom;

    vertices_[3] = v;

    ul::VertexBuffer vbuffer;
    vbuffer.format = ul::kVertexBufferFormat_2f_4ub_2f_2f_28f;
    vbuffer.size = (uint32_t)(sizeof(ul::Vertex_2f_4ub_2f_2f_28f) * vertices_.size());
    vbuffer.data = (uint8_t*)vertices_.data();

    ul::IndexBuffer ibuffer;
    ibuffer.size = (uint32_t)(sizeof(ul::IndexType) * indices_.size());
    ibuffer.data = (uint8_t*)indices_.data();

    if (initial_creation) {
      geometry_id_ = gpu_driver()->NextGeometryId();
      gpu_driver()->CreateGeometry(geometry_id_, vbuffer, ibuffer);
      return;
    }

    gpu_driver()->UpdateGeometry(geometry_id_, vbuffer, ibuffer);
  }

}
