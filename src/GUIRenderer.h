#pragma once

#define NAPI_EXPERIMENTAL
#include <napi.h>

#include <Ultralight/Ultralight.h>

#include "utils.h"

namespace ul = ultralight;

namespace nodegui {

  class GUIFrame;

  class GUIRenderer : public ul::LoadListener,
                      public ul::ViewListener {

  public:
    GUIRenderer(GUIFrame* frame);
    virtual ~GUIRenderer() {}

    virtual void Initialize(ul::Platform& platform);
    virtual void Update();
    virtual void Render();

    virtual void Flush();
    virtual void BeginDrawing();
    virtual void EndDrawing();

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

    virtual GUIFrame* frame();

    virtual ul::RefPtr<ul::View> view();
    virtual ul::RefPtr<ul::Renderer> renderer();

    ul::FontLoader* font_loader();
    ul::FileSystem* file_system();
    ul::GPUDriver* gpu_driver();
  protected:
    GUIFrame* frame_;

    ul::RefPtr<ul::View> view_;
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
