#ifndef __UTILS__
#define __UTILS__

#define NAPI_EXPERIMENTAL
#include <napi.h>

#include <AppCore/JSHelpers.h>

#include <Ultralight/Ultralight.h>
#include <Ultralight/KeyCodes.h>

#include <JavaScriptCore/JavaScript.h>
#include <JavaScriptCore/JSStringRef.h>

#include <GLFW/glfw3.h>

namespace ul = ultralight;

namespace nodegui {

  static Napi::Value ConvertJSC2NAPIObject(Napi::Env env, ul::JSValue value) {
    JSContextRef ctx = ul::GetJSContext();
    JSValueRef exception = nullptr;
    JSStringRef string = JSValueCreateJSONString(ctx, value, 0, &exception);
    if (exception) {
      Napi::TypeError::New(env, "Failed to stringify argument 2").ThrowAsJavaScriptException();
      return env.Undefined();
    }
    size_t bufferSize = JSStringGetMaximumUTF8CStringSize(string);
    char* characters = new char[bufferSize];
    JSStringGetUTF8CString(string, characters, bufferSize);
    JSStringRelease(string);
    Napi::Object json = env.Global().Get("JSON").As<Napi::Object>();
    Napi::Function parse = json.Get("parse").As<Napi::Function>();
    Napi::Value out = parse.Call(json, { Napi::String::New(env, characters) });
    free(characters);
    return out;
  }

  static JSValueRef ConvertNAPI2JSCObject(Napi::Env env, Napi::Value value) {
    JSContextRef ctx = ul::GetJSContext();
    Napi::Object json = env.Global().Get("JSON").As<Napi::Object>();
    Napi::Function stringify = json.Get("stringify").As<Napi::Function>();
    Napi::String string = stringify.Call(json, { value.As<Napi::Object>() }).As<Napi::String>();
    JSStringRef jsstr = JSStringCreateWithCharacters(reinterpret_cast<const JSChar*>(string.Utf16Value().c_str()), string.Utf16Value().size());
    JSValueRef out = JSValueMakeFromJSONString(ctx, jsstr);
    if (out == nullptr) {
      Napi::TypeError::New(env, "Stringify process failed - Cannot convert Object from NAPI to JSC").ThrowAsJavaScriptException();
    }
    return out;
  }

  static std::string UlCursorToString(ul::Cursor cursor) {
    switch (cursor) {
    case ul::kCursor_Pointer: return "Pointer";
    case ul::kCursor_Cross: return "Cross";
    case ul::kCursor_Hand: return "Hand";
    case ul::kCursor_IBeam: return "IBeam";
    case ul::kCursor_Wait: return "Wait";
    case ul::kCursor_Help: return "Help";
    case ul::kCursor_EastResize: return "EastResize";
    case ul::kCursor_NorthResize: return "NorthResize";
    case ul::kCursor_NorthEastResize: return "NorthEastResize";
    case ul::kCursor_NorthWestResize: return "NorthWestResize";
    case ul::kCursor_SouthResize: return "SouthResize";
    case ul::kCursor_SouthEastResize: return "SouthEastResize";
    case ul::kCursor_SouthWestResize: return "SouthWestResize";
    case ul::kCursor_WestResize: return "WestResize";
    case ul::kCursor_NorthSouthResize: return "NorthSouthResize";
    case ul::kCursor_EastWestResize: return "EastWestResize";
    case ul::kCursor_NorthEastSouthWestResize: return "NorthEastSouthWestResize";
    case ul::kCursor_NorthWestSouthEastResize: return "NorthWestSouthEastResize";
    case ul::kCursor_ColumnResize: return "ColumnResize";
    case ul::kCursor_RowResize: return "RowResize";
    case ul::kCursor_MiddlePanning: return "MiddlePanning";
    case ul::kCursor_EastPanning: return "EastPanning";
    case ul::kCursor_NorthPanning: return "NorthPanning";
    case ul::kCursor_NorthEastPanning: return "NorthEastPanning";
    case ul::kCursor_NorthWestPanning: return "NorthWestPanning";
    case ul::kCursor_SouthPanning: return "SouthPanning";
    case ul::kCursor_SouthEastPanning: return "SouthEastPanning";
    case ul::kCursor_SouthWestPanning: return "SouthWestPanning";
    case ul::kCursor_WestPanning: return "WestPanning";
    case ul::kCursor_Move: return "Move";
    case ul::kCursor_VerticalText: return "VerticalText";
    case ul::kCursor_Cell: return "Cell";
    case ul::kCursor_ContextMenu: return "ContextMenu";
    case ul::kCursor_Alias: return "Alias";
    case ul::kCursor_Progress: return "Progress";
    case ul::kCursor_NoDrop: return "NoDrop";
    case ul::kCursor_Copy: return "Copy";
    case ul::kCursor_None: return "None";
    case ul::kCursor_NotAllowed: return "NotAllowed";
    case ul::kCursor_ZoomIn: return "ZoomIn";
    case ul::kCursor_ZoomOut: return "ZoomOut";
    case ul::kCursor_Grab: return "Grab";
    case ul::kCursor_Grabbing: return "Grabbing";
    case ul::kCursor_Custom: return "Custom";
    };
    return "Unknown";
  }

  static std::string UlMessageLevelToString(ul::MessageLevel message) {
    switch (message) {
    case ul::kMessageLevel_Log: return "log";
    case ul::kMessageLevel_Warning: return "warn";
    case ul::kMessageLevel_Error: return "error";
    case ul::kMessageLevel_Debug: return "debug";
    case ul::kMessageLevel_Info: return "info";
    };
    return "";
  }

  static int GLFWMouseButtonToUltralightMouseButton(int button) {
    switch (button) {
    case GLFW_MOUSE_BUTTON_LEFT:
      return ul::MouseEvent::Button::kButton_Left;
    case GLFW_MOUSE_BUTTON_MIDDLE:
      return ul::MouseEvent::Button::kButton_Middle;
    case GLFW_MOUSE_BUTTON_RIGHT:
      return ul::MouseEvent::Button::kButton_Right;
    };
    return ul::MouseEvent::Button::kButton_None;
  }

  static int GLFWKeyCodeToUltralightKeyCode(int key) {
    switch (key) {
    case GLFW_KEY_SPACE:
      return ul::KeyCodes::GK_SPACE;
    case GLFW_KEY_APOSTROPHE:
      return ul::KeyCodes::GK_OEM_7;
    case GLFW_KEY_COMMA:
      return ul::KeyCodes::GK_OEM_COMMA;
    case GLFW_KEY_MINUS:
      return ul::KeyCodes::GK_OEM_MINUS;
    case GLFW_KEY_PERIOD:
      return ul::KeyCodes::GK_OEM_PERIOD;
    case GLFW_KEY_SLASH:
      return ul::KeyCodes::GK_OEM_2;
    case GLFW_KEY_0:
      return ul::KeyCodes::GK_0;
    case GLFW_KEY_1:
      return ul::KeyCodes::GK_1;
    case GLFW_KEY_2:
      return ul::KeyCodes::GK_2;
    case GLFW_KEY_3:
      return ul::KeyCodes::GK_3;
    case GLFW_KEY_4:
      return ul::KeyCodes::GK_4;
    case GLFW_KEY_5:
      return ul::KeyCodes::GK_5;
    case GLFW_KEY_6:
      return ul::KeyCodes::GK_6;
    case GLFW_KEY_7:
      return ul::KeyCodes::GK_7;
    case GLFW_KEY_8:
      return ul::KeyCodes::GK_8;
    case GLFW_KEY_9:
      return ul::KeyCodes::GK_9;
    case GLFW_KEY_SEMICOLON:
      return ul::KeyCodes::GK_OEM_1;
    case GLFW_KEY_EQUAL:
      return ul::KeyCodes::GK_OEM_PLUS;
    case GLFW_KEY_A:
      return ul::KeyCodes::GK_A;
    case GLFW_KEY_B:
      return ul::KeyCodes::GK_B;
    case GLFW_KEY_C:
      return ul::KeyCodes::GK_C;
    case GLFW_KEY_D:
      return ul::KeyCodes::GK_D;
    case GLFW_KEY_E:
      return ul::KeyCodes::GK_E;
    case GLFW_KEY_F:
      return ul::KeyCodes::GK_F;
    case GLFW_KEY_G:
      return ul::KeyCodes::GK_G;
    case GLFW_KEY_H:
      return ul::KeyCodes::GK_H;
    case GLFW_KEY_I:
      return ul::KeyCodes::GK_I;
    case GLFW_KEY_J:
      return ul::KeyCodes::GK_J;
    case GLFW_KEY_K:
      return ul::KeyCodes::GK_K;
    case GLFW_KEY_L:
      return ul::KeyCodes::GK_L;
    case GLFW_KEY_M:
      return ul::KeyCodes::GK_M;
    case GLFW_KEY_N:
      return ul::KeyCodes::GK_N;
    case GLFW_KEY_O:
      return ul::KeyCodes::GK_O;
    case GLFW_KEY_P:
      return ul::KeyCodes::GK_P;
    case GLFW_KEY_Q:
      return ul::KeyCodes::GK_Q;
    case GLFW_KEY_R:
      return ul::KeyCodes::GK_R;
    case GLFW_KEY_S:
      return ul::KeyCodes::GK_S;
    case GLFW_KEY_T:
      return ul::KeyCodes::GK_T;
    case GLFW_KEY_U:
      return ul::KeyCodes::GK_U;
    case GLFW_KEY_V:
      return ul::KeyCodes::GK_V;
    case GLFW_KEY_W:
      return ul::KeyCodes::GK_W;
    case GLFW_KEY_X:
      return ul::KeyCodes::GK_X;
    case GLFW_KEY_Y:
      return ul::KeyCodes::GK_Y;
    case GLFW_KEY_Z:
      return ul::KeyCodes::GK_Z;
    case GLFW_KEY_LEFT_BRACKET:
      return ul::KeyCodes::GK_OEM_4;
    case GLFW_KEY_BACKSLASH:
      return ul::KeyCodes::GK_OEM_5;
    case GLFW_KEY_RIGHT_BRACKET:
      return ul::KeyCodes::GK_OEM_6;
    case GLFW_KEY_GRAVE_ACCENT:
      return ul::KeyCodes::GK_OEM_3;
    case GLFW_KEY_WORLD_1:
      return ul::KeyCodes::GK_UNKNOWN;
    case GLFW_KEY_WORLD_2:
      return ul::KeyCodes::GK_UNKNOWN;
    case GLFW_KEY_ESCAPE:
      return ul::KeyCodes::GK_ESCAPE;
    case GLFW_KEY_ENTER:
      return ul::KeyCodes::GK_RETURN;
    case GLFW_KEY_TAB:
      return ul::KeyCodes::GK_TAB;
    case GLFW_KEY_BACKSPACE:
      return ul::KeyCodes::GK_BACK;
    case GLFW_KEY_INSERT:
      return ul::KeyCodes::GK_INSERT;
    case GLFW_KEY_DELETE:
      return ul::KeyCodes::GK_DELETE;
    case GLFW_KEY_RIGHT:
      return ul::KeyCodes::GK_RIGHT;
    case GLFW_KEY_LEFT:
      return ul::KeyCodes::GK_LEFT;
    case GLFW_KEY_DOWN:
      return ul::KeyCodes::GK_DOWN;
    case GLFW_KEY_UP:
      return ul::KeyCodes::GK_UP;
    case GLFW_KEY_PAGE_UP:
      return ul::KeyCodes::GK_PRIOR;
    case GLFW_KEY_PAGE_DOWN:
      return ul::KeyCodes::GK_NEXT;
    case GLFW_KEY_HOME:
      return ul::KeyCodes::GK_HOME;
    case GLFW_KEY_END:
      return ul::KeyCodes::GK_END;
    case GLFW_KEY_CAPS_LOCK:
      return ul::KeyCodes::GK_CAPITAL;
    case GLFW_KEY_SCROLL_LOCK:
      return ul::KeyCodes::GK_SCROLL;
    case GLFW_KEY_NUM_LOCK:
      return ul::KeyCodes::GK_NUMLOCK;
    case GLFW_KEY_PRINT_SCREEN:
      return ul::KeyCodes::GK_SNAPSHOT;
    case GLFW_KEY_PAUSE:
      return ul::KeyCodes::GK_PAUSE;
    case GLFW_KEY_F1:
      return ul::KeyCodes::GK_F1;
    case GLFW_KEY_F2:
      return ul::KeyCodes::GK_F2;
    case GLFW_KEY_F3:
      return ul::KeyCodes::GK_F3;
    case GLFW_KEY_F4:
      return ul::KeyCodes::GK_F4;
    case GLFW_KEY_F5:
      return ul::KeyCodes::GK_F5;
    case GLFW_KEY_F6:
      return ul::KeyCodes::GK_F6;
    case GLFW_KEY_F7:
      return ul::KeyCodes::GK_F7;
    case GLFW_KEY_F8:
      return ul::KeyCodes::GK_F8;
    case GLFW_KEY_F9:
      return ul::KeyCodes::GK_F9;
    case GLFW_KEY_F10:
      return ul::KeyCodes::GK_F10;
    case GLFW_KEY_F11:
      return ul::KeyCodes::GK_F11;
    case GLFW_KEY_F12:
      return ul::KeyCodes::GK_F12;
    case GLFW_KEY_F13:
      return ul::KeyCodes::GK_F13;
    case GLFW_KEY_F14:
      return ul::KeyCodes::GK_F14;
    case GLFW_KEY_F15:
      return ul::KeyCodes::GK_F15;
    case GLFW_KEY_F16:
      return ul::KeyCodes::GK_F16;
    case GLFW_KEY_F17:
      return ul::KeyCodes::GK_F17;
    case GLFW_KEY_F18:
      return ul::KeyCodes::GK_F18;
    case GLFW_KEY_F19:
      return ul::KeyCodes::GK_F19;
    case GLFW_KEY_F20:
      return ul::KeyCodes::GK_F20;
    case GLFW_KEY_F21:
      return ul::KeyCodes::GK_F21;
    case GLFW_KEY_F22:
      return ul::KeyCodes::GK_F22;
    case GLFW_KEY_F23:
      return ul::KeyCodes::GK_F23;
    case GLFW_KEY_F24:
      return ul::KeyCodes::GK_F24;
    case GLFW_KEY_F25:
      return ul::KeyCodes::GK_UNKNOWN;
    case GLFW_KEY_KP_0:
      return ul::KeyCodes::GK_NUMPAD0;
    case GLFW_KEY_KP_1:
      return ul::KeyCodes::GK_NUMPAD1;
    case GLFW_KEY_KP_2:
      return ul::KeyCodes::GK_NUMPAD2;
    case GLFW_KEY_KP_3:
      return ul::KeyCodes::GK_NUMPAD3;
    case GLFW_KEY_KP_4:
      return ul::KeyCodes::GK_NUMPAD4;
    case GLFW_KEY_KP_5:
      return ul::KeyCodes::GK_NUMPAD5;
    case GLFW_KEY_KP_6:
      return ul::KeyCodes::GK_NUMPAD6;
    case GLFW_KEY_KP_7:
      return ul::KeyCodes::GK_NUMPAD7;
    case GLFW_KEY_KP_8:
      return ul::KeyCodes::GK_NUMPAD8;
    case GLFW_KEY_KP_9:
      return ul::KeyCodes::GK_NUMPAD9;
    case GLFW_KEY_KP_DECIMAL:
      return ul::KeyCodes::GK_DECIMAL;
    case GLFW_KEY_KP_DIVIDE:
      return ul::KeyCodes::GK_DIVIDE;
    case GLFW_KEY_KP_MULTIPLY:
      return ul::KeyCodes::GK_MULTIPLY;
    case GLFW_KEY_KP_SUBTRACT:
      return ul::KeyCodes::GK_SUBTRACT;
    case GLFW_KEY_KP_ADD:
      return ul::KeyCodes::GK_ADD;
    case GLFW_KEY_KP_ENTER:
      return ul::KeyCodes::GK_RETURN;
    case GLFW_KEY_KP_EQUAL:
      return ul::KeyCodes::GK_OEM_PLUS;
    case GLFW_KEY_LEFT_SHIFT:
      return ul::KeyCodes::GK_SHIFT;
    case GLFW_KEY_LEFT_CONTROL:
      return ul::KeyCodes::GK_CONTROL;
    case GLFW_KEY_LEFT_ALT:
      return ul::KeyCodes::GK_MENU;
    case GLFW_KEY_LEFT_SUPER:
      return ul::KeyCodes::GK_LWIN;
    case GLFW_KEY_RIGHT_SHIFT:
      return ul::KeyCodes::GK_SHIFT;
    case GLFW_KEY_RIGHT_CONTROL:
      return ul::KeyCodes::GK_CONTROL;
    case GLFW_KEY_RIGHT_ALT:
      return ul::KeyCodes::GK_MENU;
    case GLFW_KEY_RIGHT_SUPER:
      return ul::KeyCodes::GK_RWIN;
    case GLFW_KEY_MENU:
      return ul::KeyCodes::GK_UNKNOWN;
    default:
      return ul::KeyCodes::GK_UNKNOWN;
    }
  }

}

#endif
