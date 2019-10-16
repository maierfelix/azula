#pragma once

#define NAPI_EXPERIMENTAL
#include <napi.h>

#include "FontLoaderWin.h"
#include "FileSystemWin.h"

#include "GUIFrame.h"
#include "GPUDriverD3D11.h"
#include "GPUContextD3D11.h"

#include <AppCore/JSHelpers.h>

#include <Ultralight/Ultralight.h>

#include <JavaScriptCore/JavaScript.h>
#include <JavaScriptCore/JSStringRef.h>

#include "utils.h"

using Microsoft::WRL::ComPtr;

namespace ul = ultralight;

namespace nodegui {

  class GUIRenderer : public ul::LoadListener,
                      public ul::ViewListener {

  public:
    GUIRenderer(GUIFrame* frame);
    ~GUIRenderer() {}

    void Update();
    void Render();
    void UpdateGeometry();

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
      const ul::String& source_id);

    std::unique_ptr<ul::GPUContextD3D11> gpu_context;
    std::unique_ptr<ul::GPUDriverD3D11> gpu_driver;
    ul::RefPtr<ul::View> view;
  private:
    ul::Config config;
    std::unique_ptr<ul::FontLoader> font_loader;
    std::unique_ptr<ul::FileSystem> file_system;
    ul::RefPtr<ul::Renderer> renderer;

    uint32_t geometry_id;
    std::vector<ul::Vertex_2f_4ub_2f_2f_28f> vertices;
    std::vector<ul::IndexType> indices;

    ul::GPUState gpu_state;

    GUIFrame* frame_;

    bool needs_update = true;
  };

}