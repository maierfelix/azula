#pragma once

#define NAPI_EXPERIMENTAL
#include <napi.h>

#include "GUIRenderer.h"

#include "utils.h"

namespace nodegui {

  class GUIRenderer;

  class GUIFrame : public Napi::ObjectWrap<GUIFrame> {
  public:
    GUIFrame(const Napi::CallbackInfo &info);
    ~GUIFrame();

    static Napi::Object Initialize(Napi::Env env, Napi::Object exports);
    static Napi::FunctionReference constructor;

    Napi::Env env_;

    Napi::Value Update(const Napi::CallbackInfo &info);
    Napi::Value Render(const Napi::CallbackInfo &info);
    Napi::Value Flush(const Napi::CallbackInfo &info);
    Napi::Value LoadHTML(const Napi::CallbackInfo &info);

    // shared resources for different platforms
    Napi::Value GetSharedHandleD3D11(const Napi::CallbackInfo &info);

    Napi::Value DispatchBinaryBuffer(const Napi::CallbackInfo &info);
    Napi::Value DispatchMouseEvent(const Napi::CallbackInfo& info);
    Napi::Value DispatchKeyEvent(const Napi::CallbackInfo& info);
    Napi::Value DispatchScrollEvent(const Napi::CallbackInfo& info);

    Napi::Value ShouldClose(const Napi::CallbackInfo &info);

    // events passed in from GUIRenderer
    void OnDOMReady(ul::View* caller);
    void OnChangeCursor(ul::View* caller, ul::Cursor cursor);
    void OnAddConsoleMessage(
      ul::View* caller,
      ul::MessageSource source,
      ul::MessageLevel level,
      const ul::String& message,
      uint32_t line_number,
      uint32_t column_number,
      const ul::String& source_id
    );
    ul::JSValue OnDispatchBinaryBuffer(const ul::JSObject& thisObject, const ul::JSArgs& args);

    Napi::FunctionReference onbinarymessage;
    Napi::Value Getonbinarymessage(const Napi::CallbackInfo &info);
    void Setonbinarymessage(const Napi::CallbackInfo &info, const Napi::Value& value);

    Napi::FunctionReference oncursorchange;
    Napi::Value Getoncursorchange(const Napi::CallbackInfo &info);
    void Setoncursorchange(const Napi::CallbackInfo &info, const Napi::Value& value);

    Napi::FunctionReference onconsolemessage;
    Napi::Value Getonconsolemessage(const Napi::CallbackInfo &info);
    void Setonconsolemessage(const Napi::CallbackInfo &info, const Napi::Value& value);

    bool use_offscreen_rendering = false;
  private:
    std::unique_ptr<GUIRenderer> renderer;

  };
}
