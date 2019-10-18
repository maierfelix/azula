#define NAPI_EXPERIMENTAL
#include <napi.h>

#include "GUIFrame.h"

#ifdef WIN32
#include "GUIRendererD3D11.h"
#endif

namespace ul = ultralight;

namespace nodegui {

  Napi::FunctionReference GUIFrame::constructor;

  GUIFrame::GUIFrame(const Napi::CallbackInfo& info) :
    Napi::ObjectWrap<GUIFrame>(info),
    env_(info.Env()) {

    if (info[0].IsObject()) {
      Napi::Object opts = info[0].As<Napi::Object>();
      if (opts.Has("useOffscreenRendering")) {
        use_offscreen_rendering = opts.Get("useOffscreenRendering").As<Napi::Boolean>().Value();
      }
      if (opts.Has("width")) {
        initialWidth = opts.Get("width").As<Napi::Number>().Uint32Value();
      }
      if (opts.Has("height")) {
        initialHeight = opts.Get("height").As<Napi::Number>().Uint32Value();
      }
    }

    if (use_offscreen_rendering) {
      renderer_ = std::make_unique<GUIRendererD3D11>(this);
    }
    else {
      renderer_ = std::make_unique<GUIRenderer>(this);
    }
    // we can only change the title after creating the renderer (bc Im a lazy piece of shit)
    if (info[0].IsObject()) {
      Napi::Object opts = info[0].As<Napi::Object>();
      if (opts.Has("title")) {
        renderer()->SetTitle(opts.Get("title").As<Napi::String>().Utf8Value());
      }
    }
  }
  GUIFrame::~GUIFrame() {
    if (!onbinarymessage.IsEmpty()) onbinarymessage.Reset();
    if (!oncursorchange.IsEmpty()) oncursorchange.Reset();
    if (!onconsolemessage.IsEmpty()) onconsolemessage.Reset();
    if (!onresize.IsEmpty()) onresize.Reset();
  }

  Napi::Value GUIFrame::Update(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    renderer()->Update();
    return env.Undefined();
  }

  Napi::Value GUIFrame::Flush(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    renderer()->Render();
    renderer()->Flush();
    return env.Undefined();
  }

  // title
  Napi::Value GUIFrame::GetTitle(const Napi::CallbackInfo &info) {
    Napi::Env env = info.Env();
    return Napi::String::New(env, renderer()->GetTitle());
  }
  void GUIFrame::SetTitle(const Napi::CallbackInfo &info, const Napi::Value& value) {
    Napi::Env env = info.Env();
    if (!info[0].IsString()) {
      Napi::TypeError::New(env, "Expected a 'String'").ThrowAsJavaScriptException();
    }
    else {
      renderer()->SetTitle(info[0].As<Napi::String>().Utf8Value());
    }
  }

  // width
  Napi::Value GUIFrame::GetWidth(const Napi::CallbackInfo &info) {
    Napi::Env env = info.Env();
    return Napi::Number::New(env, renderer()->view()->width());
  }
  void GUIFrame::SetWidth(const Napi::CallbackInfo &info, const Napi::Value& value) {
    Napi::Env env = info.Env();
    if (!value.IsNumber()) {
      Napi::TypeError::New(env, "Expected a 'Number'").ThrowAsJavaScriptException();
      return;
    }
    renderer()->Resize(
      value.As<Napi::Number>().Uint32Value(),
      renderer()->view()->height()
    );
  }

  // height
  Napi::Value GUIFrame::GetHeight(const Napi::CallbackInfo &info) {
    Napi::Env env = info.Env();
    return Napi::Number::New(env, renderer()->view()->width());
  }
  void GUIFrame::SetHeight(const Napi::CallbackInfo &info, const Napi::Value& value) {
    Napi::Env env = info.Env();
    if (!value.IsNumber()) {
      Napi::TypeError::New(env, "Expected a 'Number'").ThrowAsJavaScriptException();
      return;
    }
    renderer()->Resize(
      renderer()->view()->width(),
      value.As<Napi::Number>().Uint32Value()
    );
  }

  Napi::Value GUIFrame::LoadFile(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    if (!info[0].IsString()) {
      Napi::TypeError::New(env, "Expected 'String' for argument 1").ThrowAsJavaScriptException();
      return env.Undefined();
    }
    std::string path = info[0].As<Napi::String>().Utf8Value();
    renderer()->LoadFile(path);
    return env.Undefined();
  }

  Napi::Value GUIFrame::LoadHTML(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    if (!info[0].IsString()) {
      Napi::TypeError::New(env, "Expected 'String' for argument 1").ThrowAsJavaScriptException();
      return env.Undefined();
    }
    std::string html = info[0].As<Napi::String>().Utf8Value();
    renderer()->LoadHTML(html);
    return env.Undefined();
  }

  Napi::Value GUIFrame::GetSharedHandleD3D11(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    if (!use_offscreen_rendering) {
      Napi::Error::New(env, "Calling 'getSharedHandleD3D11' is only allowed in OSR mode").ThrowAsJavaScriptException();
      return Napi::BigInt::New(env, (uint64_t) 0);
    }
#ifdef WIN32
    GUIRendererD3D11* rendererD3D11 = (GUIRendererD3D11*) renderer();
    return rendererD3D11->GetSharedHandleD3D11(env);
#else
    return Napi::BigInt::New(env, (uint64_t)0);
#endif
  }

  Napi::Value GUIFrame::DispatchBinaryBuffer(const Napi::CallbackInfo &info) {
    Napi::Env env = info.Env();
    JSContextRef ctx = ul::GetJSContext();

    ul::JSObject global = ul::JSGlobalObject();
    ul::JSObject win = global["window"].ToObject();

    auto freePtr = [](void* bytes, void*) {
      free(bytes);
    };

    if (info.Length() >= 1 && info[0].IsObject() && win["onbinarymessage"].IsFunction()) {
      Napi::ArrayBuffer buf = info[0].As<Napi::ArrayBuffer>();
      void* data = buf.Data();
      size_t size = buf.ByteLength();

      JSObjectRef buffer = JSObjectMakeArrayBufferWithBytesNoCopy(ctx, data, size, freePtr, NULL, NULL);

      ul::JSValue ret;
      if (info.Length() == 2 && info[1].IsObject()) {
        JSValueRef args = ConvertNAPI2JSCObject(env, info[1]);
        ret = win["onbinarymessage"].ToFunction()({ buffer, args });
      }
      else {
        ret = win["onbinarymessage"].ToFunction()({ buffer });
      }
      if (ret.IsUndefined()) return env.Undefined();
      else if (ret.IsBoolean()) return Napi::Boolean::New(env, ret.ToBoolean());
      else if (ret.IsNumber()) return Napi::Number::New(env, ret.ToNumber());
      else {
        Napi::TypeError::New(env, "'onbinarymessage' returned an invalid type").ThrowAsJavaScriptException();
        return env.Undefined();
      }
    }

    return env.Undefined();
  }

  Napi::Value GUIFrame::DispatchMouseEvent(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    std::string eventType = info[0].As<Napi::String>().Utf8Value();
    ul::MouseEvent evt;
    evt.x = info[1].As<Napi::Number>().Uint32Value();
    evt.y = info[2].As<Napi::Number>().Uint32Value();
    if (info[3].IsNumber()) {
      evt.button = (ul::MouseEvent::Button) GLFWMouseButtonToUltralightMouseButton(
        info[3].As<Napi::Number>().Uint32Value()
      );
    }
    if (eventType == "onmousemove") {
      evt.type = ultralight::MouseEvent::kType_MouseMoved;
    }
    else if (eventType == "onmousedown") {
      evt.type = ultralight::MouseEvent::kType_MouseDown;
    }
    else if (eventType == "onmouseup") {
      evt.type = ultralight::MouseEvent::kType_MouseUp;
    }
    else {
      Napi::Error::New(env, "Invalid event type").ThrowAsJavaScriptException();
      return env.Undefined();
    }
    renderer()->view()->FireMouseEvent(evt);
    return env.Undefined();
  }

  Napi::Value GUIFrame::DispatchKeyEvent(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    ul::KeyEvent evt;
    std::string eventType = info[0].As<Napi::String>().Utf8Value();
    uint32_t keyCode = info[1].As<Napi::Number>().Uint32Value();
    if (eventType == "onkeyup") {
      evt.type = ultralight::KeyEvent::kType_KeyUp;
    }
    else if (eventType == "onkeydown") {
      evt.type = ultralight::KeyEvent::kType_RawKeyDown;
    }
    else {
      Napi::Error::New(env, "Invalid event type").ThrowAsJavaScriptException();
      return env.Undefined();
    }
    evt.virtual_key_code = GLFWKeyCodeToUltralightKeyCode(keyCode);
    renderer()->view()->FireKeyEvent(evt);
    return env.Undefined();
  }

  Napi::Value GUIFrame::DispatchScrollEvent(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    std::string eventType = info[0].As<Napi::String>().Utf8Value();
    ul::ScrollEvent evt;
    evt.type = ultralight::ScrollEvent::kType_ScrollByPixel;
    evt.delta_x = info[1].As<Napi::Number>().Uint32Value();
    evt.delta_y = info[2].As<Napi::Number>().Uint32Value();

    renderer()->view()->FireScrollEvent(evt);
    return env.Undefined();
  }

  Napi::Value GUIFrame::ShouldClose(const Napi::CallbackInfo &info) {
    Napi::Env env = info.Env();
    if (!use_offscreen_rendering) {
      return Napi::Boolean::New(env, !renderer()->app()->is_running());
    }
    return Napi::Boolean::New(env, false);
  }

  void GUIFrame::OnDOMReady(ul::View* caller) {
    ul::SetJSContext(caller->js_context());
    ul::JSObject global = ul::JSGlobalObject();
    ul::JSObject win = global["window"].ToObject();
    win["dispatchBinaryBuffer"] = (ul::JSCallbackWithRetval)std::bind(&GUIRenderer::DispatchBinaryBuffer, renderer(), std::placeholders::_1, std::placeholders::_2);
  }

  void GUIFrame::OnChangeCursor(ul::View* caller, ul::Cursor cursor) {
    if (oncursorchange.IsEmpty()) return;
    Napi::Env env = env_;
    Napi::Object out = Napi::Object::New(env);
    out.Set("name", Napi::String::New(env, UlCursorToString(cursor)));
    oncursorchange.Value().As<Napi::Function>().Call({ out });
  }

  void GUIFrame::OnAddConsoleMessage(
    ul::View* caller,
    ul::MessageSource source,
    ul::MessageLevel level,
    const ul::String& message,
    uint32_t line_number,
    uint32_t column_number,
    const ul::String& source_id
  ) {
    if (onconsolemessage.IsEmpty()) return;
    Napi::Env env = env_;
    Napi::Object out = Napi::Object::New(env);
    // level
    out.Set("level", Napi::String::New(env, UlMessageLevelToString(level)));
    // callee
    {
      Napi::Object console = env.Global().Get("console").As<Napi::Object>();
      out.Set("callee", console.Get(UlMessageLevelToString(level)).As<Napi::Function>());
    }
    // message
    out.Set("message", Napi::String::New(env, message.utf8().data()));
    // source
    out.Set("source", Napi::String::New(env, source_id.empty() ? "" : source_id.utf8().data()));
    // location
    {
      Napi::Object location = Napi::Object::New(env);
      location.Set("line", Napi::Number::New(env, line_number));
      location.Set("column", Napi::Number::New(env, column_number));
      out.Set("location", location);
    }
    onconsolemessage.Value().As<Napi::Function>().Call({ out });
  }

  ul::JSValue GUIFrame::OnDispatchBinaryBuffer(const ul::JSObject& thisObject, const ul::JSArgs& info) {
    Napi::Env env = env_;
    JSContextRef ctx = ul::GetJSContext();
    ul::JSValueUndefinedTag undefined;

    if (info.size() >= 1 && info[0].IsObject() && !(onbinarymessage.IsEmpty())) {
      void* data = JSObjectGetArrayBufferBytesPtr(ctx, info[0], NULL);
      size_t size = JSObjectGetArrayBufferByteLength(ctx, info[0], NULL);

      Napi::Object out = Napi::Object::New(env);

      Napi::ArrayBuffer buffer = Napi::ArrayBuffer::New(env, data, size);

      Napi::Value ret;
      if (info.size() == 2 && info[1].IsObject()) {
        ret = onbinarymessage.Value().As<Napi::Function>().Call({ buffer, ConvertJSC2NAPIObject(env, info[1]) });
      }
      else {
        ret = onbinarymessage.Value().As<Napi::Function>().Call({ buffer });
      }
      if (ret.IsUndefined()) return ul::JSValue(undefined);
      else if (ret.IsBoolean()) return ul::JSValue(ret.As<Napi::Boolean>().Value());
      else if (ret.IsNumber()) return ul::JSValue(ret.As<Napi::Number>().Int32Value());
      else {
        Napi::TypeError::New(env, "'onbinarymessage' returned an invalid type").ThrowAsJavaScriptException();
        return ul::JSValue(undefined);
      }
    }

    return ul::JSValue(undefined);
  };

  void GUIFrame::OnResize(uint32_t width, uint32_t height) {
    Napi::Env env = env_;
    if (onresize.IsEmpty()) return;
    Napi::Object out = Napi::Object::New(env);
    out.Set("width", Napi::Number::New(env, width));
    out.Set("height", Napi::Number::New(env, height));
    onresize.Value().As<Napi::Function>().Call({ out });
  }

  // onbinarymessage
  Napi::Value GUIFrame::Getonbinarymessage(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    if (this->onbinarymessage.IsEmpty()) return env.Null();
    return this->onbinarymessage.Value().As<Napi::Function>();
  }
  void GUIFrame::Setonbinarymessage(const Napi::CallbackInfo& info, const Napi::Value& value) {
    Napi::Env env = info.Env();
    if (value.IsFunction()) this->onbinarymessage.Reset(value.As<Napi::Function>(), 1);
    else if (value.IsNull()) this->onbinarymessage.Reset();
    else Napi::TypeError::New(env, "Argument 1 must be of type 'Function'").ThrowAsJavaScriptException();
  }

  // oncursorchange
  Napi::Value GUIFrame::Getoncursorchange(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    if (this->oncursorchange.IsEmpty()) return env.Null();
    return this->oncursorchange.Value().As<Napi::Function>();
  }
  void GUIFrame::Setoncursorchange(const Napi::CallbackInfo& info, const Napi::Value& value) {
    Napi::Env env = info.Env();
    if (value.IsFunction()) this->oncursorchange.Reset(value.As<Napi::Function>(), 1);
    else if (value.IsNull()) this->oncursorchange.Reset();
    else Napi::TypeError::New(env, "Argument 1 must be of type 'Function'").ThrowAsJavaScriptException();
  }

  // onconsolemessage
  Napi::Value GUIFrame::Getonconsolemessage(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    if (this->onconsolemessage.IsEmpty()) return env.Null();
    return this->onconsolemessage.Value().As<Napi::Function>();
  }
  void GUIFrame::Setonconsolemessage(const Napi::CallbackInfo& info, const Napi::Value& value) {
    Napi::Env env = info.Env();
    if (value.IsFunction()) this->onconsolemessage.Reset(value.As<Napi::Function>(), 1);
    else if (value.IsNull()) this->onconsolemessage.Reset();
    else Napi::TypeError::New(env, "Argument 1 must be of type 'Function'").ThrowAsJavaScriptException();
  }

  // onresize
  Napi::Value GUIFrame::Getonresize(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    if (this->onresize.IsEmpty()) return env.Null();
    return this->onresize.Value().As<Napi::Function>();
  }
  void GUIFrame::Setonresize(const Napi::CallbackInfo& info, const Napi::Value& value) {
    Napi::Env env = info.Env();
    if (value.IsFunction()) this->onresize.Reset(value.As<Napi::Function>(), 1);
    else if (value.IsNull()) this->onresize.Reset();
    else Napi::TypeError::New(env, "Argument 1 must be of type 'Function'").ThrowAsJavaScriptException();
  }

  Napi::Object GUIFrame::Initialize(Napi::Env env, Napi::Object exports) {
    Napi::HandleScope scope(env);
    Napi::Function func = DefineClass(env, "Window", {
      // methods
      InstanceMethod(
        "update",
        &GUIFrame::Update,
        napi_enumerable
      ),
      InstanceMethod(
        "flush",
        &GUIFrame::Flush,
        napi_enumerable
      ),
      InstanceAccessor(
        "title",
        &GUIFrame::GetTitle,
        &GUIFrame::SetTitle,
        napi_enumerable
      ),
      InstanceAccessor(
        "width",
        &GUIFrame::GetWidth,
        &GUIFrame::SetWidth,
        napi_enumerable
      ),
      InstanceAccessor(
        "height",
        &GUIFrame::GetHeight,
        &GUIFrame::SetHeight,
        napi_enumerable
      ),
      InstanceMethod(
        "loadFile",
        &GUIFrame::LoadFile
      ),
      InstanceMethod(
        "loadHTML",
        &GUIFrame::LoadHTML
      ),
      InstanceMethod(
        "getSharedHandleD3D11",
        &GUIFrame::GetSharedHandleD3D11
      ),
      InstanceMethod(
        "dispatchBinaryBuffer",
        &GUIFrame::DispatchBinaryBuffer
      ),
      InstanceMethod(
        "dispatchMouseEvent",
        &GUIFrame::DispatchMouseEvent
      ),
      InstanceMethod(
        "dispatchKeyEvent",
        &GUIFrame::DispatchKeyEvent
      ),
      InstanceMethod(
        "dispatchScrollEvent",
        &GUIFrame::DispatchScrollEvent
      ),
      InstanceMethod(
        "shouldClose",
        &GUIFrame::ShouldClose
      ),
      InstanceAccessor(
        "onbinarymessage",
        &GUIFrame::Getonbinarymessage,
        &GUIFrame::Setonbinarymessage,
        napi_enumerable
      ),
      InstanceAccessor(
        "oncursorchange",
        &GUIFrame::Getoncursorchange,
        &GUIFrame::Setoncursorchange,
        napi_enumerable
      ),
      InstanceAccessor(
        "onconsolemessage",
        &GUIFrame::Getonconsolemessage,
        &GUIFrame::Setonconsolemessage,
        napi_enumerable
      ),
      InstanceAccessor(
        "onresize",
        &GUIFrame::Getonresize,
        &GUIFrame::Setonresize,
        napi_enumerable
      ),
    });
    constructor = Napi::Persistent(func);
    constructor.SuppressDestruct();
    exports.Set("Window", func);
    return exports;
  }

}