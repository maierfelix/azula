#pragma once

#define NAPI_EXPERIMENTAL
#include <napi.h>

#include <d3d11.h>
#include <dxgi1_2.h>
#include <wrl/client.h>

#include "GUIRenderer.h"

#include "utils.h"

using Microsoft::WRL::ComPtr;

namespace nodegui {

  class GUIRenderer;

  class GUIFrame : public Napi::ObjectWrap<GUIFrame> {
  public:
    static Napi::Object Initialize(Napi::Env env, Napi::Object exports);
    static Napi::FunctionReference constructor;

    Napi::Env env_;

    Napi::Value update(const Napi::CallbackInfo &info);
    Napi::Value render(const Napi::CallbackInfo &info);
    Napi::Value flush(const Napi::CallbackInfo &info);
    Napi::Value loadHTML(const Napi::CallbackInfo &info);
    Napi::Value getSharedHandle(const Napi::CallbackInfo &info);

    Napi::Value dispatchBinaryBuffer(const Napi::CallbackInfo &info);
    Napi::Value dispatchMouseEvent(const Napi::CallbackInfo& info);
    Napi::Value dispatchKeyEvent(const Napi::CallbackInfo& info);
    Napi::Value dispatchScrollEvent(const Napi::CallbackInfo& info);

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

    GUIFrame(const Napi::CallbackInfo &info);
    ~GUIFrame();
  private:
    std::unique_ptr<GUIRenderer> renderer;

  };
}
