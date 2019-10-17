#pragma once

#define NAPI_EXPERIMENTAL
#include <napi.h>

#include <Ultralight/Ultralight.h>

#include <AppCore/App.h>
#include <AppCore/Window.h>
#include <AppCore/Overlay.h>

#include "utils.h"

namespace ul = ultralight;

namespace nodegui {

  class GUIFrame;

  class GUIRenderer : public ul::LoadListener,
                      public ul::ViewListener,
                      public ul::WindowListener {

  public:
    GUIRenderer(GUIFrame* frame);
    virtual ~GUIRenderer() {}

    virtual void Initialize(ul::Platform& platform);
    virtual void Update();
    virtual void Render();

    virtual void Flush();
    virtual void BeginDrawing();
    virtual void EndDrawing();

    virtual void LoadHTML(std::string& html);

    virtual void UpdateGeometry();

    ul::JSValue DispatchBinaryBuffer(const ul::JSObject& thisObject, const ul::JSArgs& args);

    virtual void OnDOMReady(ul::View* caller);
    virtual void OnChangeCursor(ul::View* caller, ul::Cursor cursor);
    virtual void OnAddConsoleMessage(
      ul::View* caller,
      ul::MessageSource source,
      ul::MessageLevel level,
      const ul::String& message,
      uint32_t line_number,
      uint32_t column_number,
      const ul::String& source_id
    );

    virtual void OnClose() override;
    virtual void OnResize(uint32_t width, uint32_t height) override;

    virtual GUIFrame* frame() { assert(frame_.get()); return frame_.get(); }

    virtual ul::RefPtr<ul::App> app() { assert(app_.get()); return app_.get(); }
    virtual ul::RefPtr<ul::View> view() { assert(view_.get()); return view_.get(); }
    virtual ul::RefPtr<ul::Window> window() { assert(window_.get()); return window_.get(); }
    virtual ul::RefPtr<ul::Overlay> overlay() { assert(overlay_.get()); return overlay_.get(); }
    virtual ul::RefPtr<ul::Renderer> renderer() { assert(renderer_.get()); return renderer_.get(); }

    virtual ul::GPUDriver* gpu_driver() { assert(gpu_driver_); return gpu_driver_.get(); }
    virtual ul::FontLoader* font_loader() { assert(font_loader_.get()); return font_loader_.get(); }
    virtual ul::FileSystem* file_system() { assert(file_system_.get()); return file_system_.get(); }

  protected:
    std::unique_ptr<GUIFrame> frame_;

    ul::RefPtr<ul::App> app_;
    ul::RefPtr<ul::View> view_;
    ul::RefPtr<ul::Window> window_;
    ul::RefPtr<ul::Overlay> overlay_;
    ul::RefPtr<ul::Renderer> renderer_;

    std::unique_ptr<ul::GPUDriver> gpu_driver_;
    std::unique_ptr<ul::FontLoader> font_loader_;
    std::unique_ptr<ul::FileSystem> file_system_;

  private:
    ul::Config config;

    uint32_t geometry_id;
    std::vector<ul::Vertex_2f_4ub_2f_2f_28f> vertices;
    std::vector<ul::IndexType> indices;

    ul::GPUState gpu_state;

    bool needs_update = true;
  };

}
