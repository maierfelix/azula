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
    GUIRenderer() {}
    virtual ~GUIRenderer() {}

    virtual void Update();
    virtual void Render();
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

  protected:
    GUIFrame* frame_;
    ul::RefPtr<ul::View> view_;
    ul::RefPtr<ul::Renderer> renderer_;
  };

}
