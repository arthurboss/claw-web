#ifdef __EMSCRIPTEN__

#include "WasmVideoPlatform.h"

#include "../../Events/AppEvent.h"
#include "../../Events/AppEventQueue.h"

#include <SDL2/SDL_scancode.h>
#include <cmath>
#include <cstring>
#include <emscripten/emscripten.h>
#include <emscripten/html5.h>

namespace {

int32_t MapDomCodeToSDLScancode(const char *code) {
  if (!code || !code[0]) {
    return SDL_SCANCODE_UNKNOWN;
  }

  // Letters: "KeyA".."KeyZ"
  if (std::strncmp(code, "Key", 3) == 0 && std::strlen(code) == 4) {
    char c = code[3];
    if (c >= 'A' && c <= 'Z') {
      return SDL_SCANCODE_A + (c - 'A');
    }
  }

  // Digits: "Digit0".."Digit9"
  if (std::strncmp(code, "Digit", 5) == 0 && std::strlen(code) == 6) {
    char c = code[5];
    if (c >= '0' && c <= '9') {
      return SDL_SCANCODE_0 + (c - '0');
    }
  }

  // Common named keys
  if (std::strcmp(code, "ArrowLeft") == 0)
    return SDL_SCANCODE_LEFT;
  if (std::strcmp(code, "ArrowRight") == 0)
    return SDL_SCANCODE_RIGHT;
  if (std::strcmp(code, "ArrowUp") == 0)
    return SDL_SCANCODE_UP;
  if (std::strcmp(code, "ArrowDown") == 0)
    return SDL_SCANCODE_DOWN;

  if (std::strcmp(code, "Space") == 0)
    return SDL_SCANCODE_SPACE;
  if (std::strcmp(code, "Enter") == 0)
    return SDL_SCANCODE_RETURN;
  if (std::strcmp(code, "Escape") == 0)
    return SDL_SCANCODE_ESCAPE;

  if (std::strcmp(code, "ShiftLeft") == 0)
    return SDL_SCANCODE_LSHIFT;
  if (std::strcmp(code, "ShiftRight") == 0)
    return SDL_SCANCODE_RSHIFT;
  if (std::strcmp(code, "ControlLeft") == 0)
    return SDL_SCANCODE_LCTRL;
  if (std::strcmp(code, "ControlRight") == 0)
    return SDL_SCANCODE_RCTRL;
  if (std::strcmp(code, "AltLeft") == 0)
    return SDL_SCANCODE_LALT;
  if (std::strcmp(code, "AltRight") == 0)
    return SDL_SCANCODE_RALT;

  if (std::strcmp(code, "Tab") == 0)
    return SDL_SCANCODE_TAB;
  if (std::strcmp(code, "Backspace") == 0)
    return SDL_SCANCODE_BACKSPACE;

  return SDL_SCANCODE_UNKNOWN;
}

EM_BOOL OnKey(int eventType, const EmscriptenKeyboardEvent *e, void *userData) {
  auto *self = static_cast<WasmVideoPlatform *>(userData);
  if (!self) {
    return EM_FALSE;
  }

  AppEvent evt;
  evt.type = (eventType == EMSCRIPTEN_EVENT_KEYDOWN) ? AppEventType::KeyDown
                                                     : AppEventType::KeyUp;
  evt.key.scancode = MapDomCodeToSDLScancode(e->code);
  // BaseGameApp will compute a canonical SDL_Keycode from scancode.
  evt.key.keycode = 0;
  evt.key.repeat = (e->repeat != 0);

  AppEventQueue *q = self->GetEventQueue();
  if (q) {
    q->Push(evt);
  }

  return EM_FALSE;
}

EM_BOOL OnMouseMove(int /*eventType*/, const EmscriptenMouseEvent *e,
                    void *userData) {
  auto *self = static_cast<WasmVideoPlatform *>(userData);
  if (!self || !self->GetEventQueue()) {
    return EM_FALSE;
  }

  AppEvent evt;
  evt.type = AppEventType::MouseMove;
  evt.mouseMove.x = static_cast<float>(e->canvasX);
  evt.mouseMove.y = static_cast<float>(e->canvasY);

  self->GetEventQueue()->Push(evt);
  return EM_FALSE;
}

EM_BOOL OnMouseButton(int eventType, const EmscriptenMouseEvent *e,
                      void *userData) {
  auto *self = static_cast<WasmVideoPlatform *>(userData);
  if (!self || !self->GetEventQueue()) {
    return EM_FALSE;
  }

  AppEvent evt;
  evt.type = (eventType == EMSCRIPTEN_EVENT_MOUSEDOWN) ? AppEventType::MouseDown
                                                       : AppEventType::MouseUp;
  // Emscripten: 0=left,1=middle,2=right. SDL: 1=left,2=middle,3=right.
  uint8_t sdlButton = 0;
  if (e->button == 0)
    sdlButton = 1;
  else if (e->button == 1)
    sdlButton = 2;
  else if (e->button == 2)
    sdlButton = 3;
  else
    sdlButton = static_cast<uint8_t>(e->button);

  evt.mouseButton.button = sdlButton;
  evt.mouseButton.x = static_cast<float>(e->canvasX);
  evt.mouseButton.y = static_cast<float>(e->canvasY);

  self->GetEventQueue()->Push(evt);
  return EM_FALSE;
}

EM_BOOL OnResize(int /*eventType*/, const EmscriptenUiEvent * /*e*/,
                 void *userData) {
  auto *self = static_cast<WasmVideoPlatform *>(userData);
  if (!self || !self->GetEventQueue()) {
    return EM_FALSE;
  }

  double cssW = 0.0;
  double cssH = 0.0;
  emscripten_get_element_css_size("#canvas", &cssW, &cssH);

  double dpi = EM_ASM_DOUBLE(
      { return window.devicePixelRatio ? window.devicePixelRatio : 1.0; });
  if (dpi <= 0.0) {
    dpi = 1.0;
  }

  int fbW = static_cast<int>(std::lround(cssW * dpi));
  int fbH = static_cast<int>(std::lround(cssH * dpi));

  AppEvent evt;
  evt.type = AppEventType::Resize;
  evt.resize.width = fbW;
  evt.resize.height = fbH;
  evt.resize.dpiScale = static_cast<float>(dpi);

  self->GetEventQueue()->Push(evt);
  return EM_FALSE;
}

} // namespace

WasmVideoPlatform::WasmVideoPlatform()
    : m_queue(nullptr), m_installedCallbacks(false) {}

WasmVideoPlatform::~WasmVideoPlatform() { Shutdown(); }

bool WasmVideoPlatform::Init(int32_t /*initialWidth*/,
                             int32_t /*initialHeight*/) {
  if (m_installedCallbacks) {
    return true;
  }

  // Keyboard on window, mouse on canvas.
  emscripten_set_keydown_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, this, true,
                                  OnKey);
  emscripten_set_keyup_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, this, true,
                                OnKey);

  emscripten_set_mousemove_callback("#canvas", this, true, OnMouseMove);
  emscripten_set_mousedown_callback("#canvas", this, true, OnMouseButton);
  emscripten_set_mouseup_callback("#canvas", this, true, OnMouseButton);

  emscripten_set_resize_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, this, true,
                                 OnResize);

  m_installedCallbacks = true;

  // Emit an initial resize so engine picks up correct size.
  OnResize(0, nullptr, this);
  return true;
}

void WasmVideoPlatform::Shutdown() {
  if (!m_installedCallbacks) {
    return;
  }

  // Best-effort: unregister by setting nullptr callbacks.
  emscripten_set_keydown_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, nullptr, true,
                                  nullptr);
  emscripten_set_keyup_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, nullptr, true,
                                nullptr);
  emscripten_set_mousemove_callback("#canvas", nullptr, true, nullptr);
  emscripten_set_mousedown_callback("#canvas", nullptr, true, nullptr);
  emscripten_set_mouseup_callback("#canvas", nullptr, true, nullptr);
  emscripten_set_resize_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, nullptr, true,
                                 nullptr);

  m_installedCallbacks = false;
}

void WasmVideoPlatform::SetEventQueue(AppEventQueue *queue) { m_queue = queue; }

void WasmVideoPlatform::PumpEvents() {
  // No-op; events are pushed via browser callbacks.
}

double WasmVideoPlatform::NowSeconds() const {
  return emscripten_get_now() / 1000.0;
}

#endif // __EMSCRIPTEN__
