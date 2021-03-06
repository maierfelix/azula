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
    Napi::Value Flush(const Napi::CallbackInfo &info);

    // general getter/setter
    Napi::Value GetTitle(const Napi::CallbackInfo &info);
    void SetTitle(const Napi::CallbackInfo &info, const Napi::Value& value);
    Napi::Value GetWidth(const Napi::CallbackInfo &info);
    void SetWidth(const Napi::CallbackInfo &info, const Napi::Value& value);
    Napi::Value GetHeight(const Napi::CallbackInfo &info);
    void SetHeight(const Napi::CallbackInfo &info, const Napi::Value& value);

    Napi::Value LoadFile(const Napi::CallbackInfo &info);
    Napi::Value LoadHTML(const Napi::CallbackInfo &info);

    // shared resources for different platforms
    Napi::Value GetSharedHandleD3D11(const Napi::CallbackInfo &info);

    // messaging
    Napi::Value DispatchMouseEvent(const Napi::CallbackInfo& info);
    Napi::Value DispatchKeyEvent(const Napi::CallbackInfo& info);
    Napi::Value DispatchScrollEvent(const Napi::CallbackInfo& info);
    Napi::Value DispatchBinaryBuffer(const Napi::CallbackInfo &info);
    Napi::Value DispatchObject(const Napi::CallbackInfo &info);

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
    void OnResize(uint32_t width, uint32_t height);
    ul::JSValue OnDispatchBinaryBuffer(const ul::JSObject& thisObject, const ul::JSArgs& args);
    ul::JSValue OnDispatchObject(const ul::JSObject& thisObject, const ul::JSArgs& args);

    // callback function getter/setter
    Napi::FunctionReference onbinarymessage;
    Napi::Value Getonbinarymessage(const Napi::CallbackInfo &info);
    void Setonbinarymessage(const Napi::CallbackInfo &info, const Napi::Value& value);
    Napi::FunctionReference onobjectmessage;
    Napi::Value Getonobjectmessage(const Napi::CallbackInfo &info);
    void Setonobjectmessage(const Napi::CallbackInfo &info, const Napi::Value& value);
    Napi::FunctionReference oncursorchange;
    Napi::Value Getoncursorchange(const Napi::CallbackInfo &info);
    void Setoncursorchange(const Napi::CallbackInfo &info, const Napi::Value& value);
    Napi::FunctionReference onconsolemessage;
    Napi::Value Getonconsolemessage(const Napi::CallbackInfo &info);
    void Setonconsolemessage(const Napi::CallbackInfo &info, const Napi::Value& value);
    Napi::FunctionReference onresize;
    Napi::Value Getonresize(const Napi::CallbackInfo &info);
    void Setonresize(const Napi::CallbackInfo &info, const Napi::Value& value);

    uint32_t initialWidth = 640;
    uint32_t initialHeight = 480;

    bool use_offscreen_rendering = false;

    virtual GUIRenderer* renderer() { assert(renderer_); return renderer_.get(); }

  private:
    std::unique_ptr<GUIRenderer> renderer_;

  };
}
