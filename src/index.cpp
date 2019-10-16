#define NAPI_EXPERIMENTAL
#include <napi.h>

#include "GUIFrame.h"

Napi::Object Init(Napi::Env env, Napi::Object exports) {
  nodegui::GUIFrame::Initialize(env, exports);
  return exports;
}

NODE_API_MODULE(addon, Init)
