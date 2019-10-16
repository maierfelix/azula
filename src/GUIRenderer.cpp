#define NAPI_EXPERIMENTAL
#include <napi.h>

#include "GUIRenderer.h"
#include "GUIFrame.h"

namespace ul = ultralight;

using Microsoft::WRL::ComPtr;

namespace nodegui {

  GUIFrame* GUIRenderer::frame() { assert(frame_); return frame_; }

  ul::RefPtr<ul::View> GUIRenderer::view() { assert(view_.get()); return view_.get(); }
  ul::RefPtr<ul::Renderer> GUIRenderer::renderer() { assert(renderer_.get()); return renderer_.get(); }

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

  void GUIRenderer::Update() {

  }

  void GUIRenderer::Render() {

  }

  void GUIRenderer::UpdateGeometry() {

  }

}
