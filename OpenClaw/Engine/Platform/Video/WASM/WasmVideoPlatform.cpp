#ifdef __EMSCRIPTEN__

#include "WasmVideoPlatform.h"

#include "../../Events/AppEvent.h"
#include "../../Events/AppEventQueue.h"
#include "../../../GameApp/BaseGameApp.h"
#include "../../../GameApp/BaseGameLogic.h"
#include "../../../UserInterface/HumanView.h"

#include <SDL2/SDL_scancode.h>
#include <cmath>
#include <cstring>
#include <emscripten/emscripten.h>
#include <emscripten/html5.h>

// Global pointer for JavaScript callbacks to access the event queue
static AppEventQueue* g_jsGamepadEventQueue = nullptr;

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

// --- Pointer Events bridge helpers (used by the OnJSPointer* callbacks) ---

constexpr int kPointerTypeMouse = 0; // pointer-bridge: 0=mouse, 1=touch, 2=pen

// DOM/PointerEvent button (0=left,1=middle,2=right) -> SDL button (1,2,3).
uint8_t PointerButtonToSDL(int button) {
  if (button == 0) return 1;
  if (button == 1) return 2;
  if (button == 2) return 3;
  return static_cast<uint8_t>(button + 1);
}

// The on-screen touch controls overlay (touch-controls.js) is the sole source
// of touch input for BOTH gameplay and menus — it dispatches keyboard events
// from its own DOM buttons. So raw canvas touch/pen must not generate any game
// input: it neither feeds the legacy SDL_FINGER recognizers nor synthesizes
// mouse events (which would otherwise let a tap select a menu item directly).
// Only real mouse input produces game events. Touch still updates the cursor
// position + last-input flag (handled in the callbacks below).
bool PointerEmitsGameInput(int ptype) {
  return ptype == kPointerTypeMouse;
}

} // namespace

// ============================================================================
// JavaScript Gamepad Bridge Callbacks
// These functions are called from gamepad-bridge.js via Module.ccall()
// ============================================================================

extern "C" {

EMSCRIPTEN_KEEPALIVE void OnJSGamepadConnected(int index) {
  if (!g_jsGamepadEventQueue) return;

  EM_ASM({
    console.log('[Gamepad C++] Controller ' + $0 + ' connected');
  }, index);

  g_jsGamepadEventQueue->Push(AppEvent::MakeGamepadConnected(index));
}

EMSCRIPTEN_KEEPALIVE void OnJSGamepadDisconnected(int index) {
  if (!g_jsGamepadEventQueue) return;

  EM_ASM({
    console.log('[Gamepad C++] Controller ' + $0 + ' disconnected');
  }, index);

  g_jsGamepadEventQueue->Push(AppEvent::MakeGamepadDisconnected(index));
}

EMSCRIPTEN_KEEPALIVE void OnJSGamepadButtonDown(int index, int button, float value) {
  if (g_pApp) g_pApp->SetLastInputSource(BaseGameApp::LastInput_Gamepad);
  if (!g_jsGamepadEventQueue) return;

  AppEvent evt;
  evt.type = AppEventType::GamepadButtonDown;
  evt.gamepadButton.gamepadIndex = index;
  evt.gamepadButton.button = static_cast<GamepadButton>(button);
  evt.gamepadButton.value = value;
  g_jsGamepadEventQueue->Push(evt);
}

EMSCRIPTEN_KEEPALIVE void OnJSGamepadButtonUp(int index, int button) {
  if (!g_jsGamepadEventQueue) return;

  AppEvent evt;
  evt.type = AppEventType::GamepadButtonUp;
  evt.gamepadButton.gamepadIndex = index;
  evt.gamepadButton.button = static_cast<GamepadButton>(button);
  evt.gamepadButton.value = 0.0f;
  g_jsGamepadEventQueue->Push(evt);
}

EMSCRIPTEN_KEEPALIVE void OnJSGamepadAxis(int index, int axis, float value) {
  // Meaningful stick deflection counts as gamepad activity (JS already applies
  // a deadzone, but guard here too so idle drift doesn't grab the input focus).
  if (g_pApp && (value > 0.2f || value < -0.2f))
    g_pApp->SetLastInputSource(BaseGameApp::LastInput_Gamepad);
  if (!g_jsGamepadEventQueue) return;

  AppEvent evt;
  evt.type = AppEventType::GamepadAxis;
  evt.gamepadAxis.gamepadIndex = index;
  evt.gamepadAxis.axis = static_cast<GamepadAxis>(axis);
  evt.gamepadAxis.value = value;
  g_jsGamepadEventQueue->Push(evt);
}

// Marks the gamepad as the active input source. Called from gamepad-bridge.js
// on any gamepad activity — including menu navigation, which is handled JS-side
// via synthesized keyboard events and so never reaches OnJSGamepadButtonDown.
EMSCRIPTEN_KEEPALIVE void OnJSGamepadActivity() {
  if (g_pApp) g_pApp->SetLastInputSource(BaseGameApp::LastInput_Gamepad);
}

// ============================================================================
// Pointer Events Bridge Callbacks (called from pointer-bridge.js)
// One unified stream for mouse, touch and pen. Coordinates arrive already
// converted to window (device-pixel canvas) space by the JS shim.
// Helpers live in the anonymous namespace above (C++ linkage).
// ============================================================================

EMSCRIPTEN_KEEPALIVE void OnJSPointerDown(int /*pointerId*/, int x, int y,
                                          int ptype, int button) {
  if (g_pApp) {
    g_pApp->SetPointerPosition(x, y);
    g_pApp->SetLastInputWasTouch(ptype != 0); // 0=mouse, 1=touch, 2=pen
  }
  // Touch/pen: cursor position updated above, but no game event — the overlay
  // is the sole touch input source. Only mouse produces menu/game input.
  if (!g_jsGamepadEventQueue || !PointerEmitsGameInput(ptype)) return;

  AppEvent evt;
  evt.type = AppEventType::MouseDown;
  evt.mouseButton.button = PointerButtonToSDL(button);
  evt.mouseButton.x = static_cast<float>(x);
  evt.mouseButton.y = static_cast<float>(y);
  g_jsGamepadEventQueue->Push(evt);
}

EMSCRIPTEN_KEEPALIVE void OnJSPointerMove(int /*pointerId*/, int x, int y,
                                          int ptype) {
  if (g_pApp) {
    g_pApp->SetPointerPosition(x, y);
    g_pApp->SetLastInputWasTouch(ptype != 0); // 0=mouse, 1=touch, 2=pen
  }
  if (!g_jsGamepadEventQueue || !PointerEmitsGameInput(ptype)) return;

  AppEvent evt;
  evt.type = AppEventType::MouseMove;
  evt.mouseMove.x = static_cast<float>(x);
  evt.mouseMove.y = static_cast<float>(y);
  g_jsGamepadEventQueue->Push(evt);
}

EMSCRIPTEN_KEEPALIVE void OnJSPointerUp(int /*pointerId*/, int x, int y,
                                        int ptype, int button) {
  if (g_pApp) {
    g_pApp->SetPointerPosition(x, y);
    g_pApp->SetLastInputWasTouch(ptype != 0); // 0=mouse, 1=touch, 2=pen
  }
  if (!g_jsGamepadEventQueue || !PointerEmitsGameInput(ptype)) return;

  AppEvent evt;
  evt.type = AppEventType::MouseUp;
  evt.mouseButton.button = PointerButtonToSDL(button);
  evt.mouseButton.x = static_cast<float>(x);
  evt.mouseButton.y = static_cast<float>(y);
  g_jsGamepadEventQueue->Push(evt);
}

EMSCRIPTEN_KEEPALIVE void OnJSTouchCapability(int isTouchDevice) {
  if (g_pApp) {
    g_pApp->SetTouchDevice(isTouchDevice != 0);
    // Seed the last-input state so a pure-touch device hides the cursor from
    // the first frame; a real mouse event flips it back if one arrives.
    g_pApp->SetLastInputWasTouch(isTouchDevice != 0);
  }
}

// Returns game state for JS to determine input mode:
// 0 = unknown/invalid, 1 = menu, 2 = in-game running, 3 = in-game paused
EMSCRIPTEN_KEEPALIVE int GetJSGameState() {
  if (!g_pApp || !g_pApp->GetGameLogic()) return 0;

  GameState state = g_pApp->GetGameLogic()->GetGameState();
  switch (state) {
    case GameState_Menu:
    case GameState_LoadingMenu:
      return 1; // Menu
    case GameState_IngameRunning:
      return 2; // In-game running
    case GameState_IngamePaused:
      return 3; // In-game paused (quick menu open)
    case GameState_Cutscene:
      return 4; // Cutscene
    default:
      return 0; // Loading or other
  }
}

// 1 if a menu (main menu or in-game quick menu) is currently visible, or if a score screen is visible.
// Lets JS tell "playing" from "quick menu open" / "score screen", which GetJSGameState can't (both are
// IngameRunning). Mirrors the check used by TouchDrivesGameplay.
EMSCRIPTEN_KEEPALIVE int IsScoreScreenVisibleJS() {
  if (!g_pApp) return 0;
  HumanView* pView = g_pApp->GetHumanView();
  if (pView && pView->IsScoreScreenVisible()) return 1;
  return 0;
}

EMSCRIPTEN_KEEPALIVE int IsMenuVisibleJS() {
  if (!g_pApp) return 0;
  if (g_pApp->IsMenuActive()) return 1;
  return IsScoreScreenVisibleJS();
}

// Called from JavaScript when entering/exiting fullscreen to adjust game resolution
// Width is calculated based on screen aspect ratio, height is always 480
EMSCRIPTEN_KEEPALIVE void OnJSResolutionChange(int width, int height) {
  if (!g_pApp || !g_pApp->GetRenderer()) return;

  // Update the game's internal resolution
  g_pApp->SetWindowSize(width, height, 1.0);
}

// Returns current game width for JS to query
EMSCRIPTEN_KEEPALIVE int GetGameWidth() {
  if (!g_pApp) return 640;
  return (int)g_pApp->GetWindowSize().x;
}

// Returns current game height for JS to query
EMSCRIPTEN_KEEPALIVE int GetGameHeight() {
  if (!g_pApp) return 480;
  return (int)g_pApp->GetWindowSize().y;
}

EMSCRIPTEN_KEEPALIVE void OnJSFullscreenChange() {
  if (!g_pApp || !g_pApp->GetHumanView()) return;
  auto pMenu = g_pApp->GetHumanView()->GetActiveMenu();
  if (pMenu) pMenu->RefreshActivePageVisibility();
}

} // extern "C"

// ============================================================================

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

  // Mouse/touch/pen input arrives via the Pointer Events bridge
  // (pointer-bridge.js -> OnJSPointer*), so no html5 mouse callbacks here.

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
  emscripten_set_resize_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, nullptr, true,
                                 nullptr);

  m_installedCallbacks = false;
}

void WasmVideoPlatform::SetEventQueue(AppEventQueue *queue) {
  m_queue = queue;
  // Also set global pointer for JavaScript gamepad bridge callbacks
  g_jsGamepadEventQueue = queue;
}

void WasmVideoPlatform::PumpEvents() {
  // Gamepad events now come from JavaScript bridge (gamepad-bridge.js)
  // via the exported OnJSGamepad* functions, so no C++ polling needed here.
  //
  // The JavaScript bridge polls navigator.getGamepads() on each animation
  // frame and calls Module.ccall() to push events to the C++ event queue.
}

double WasmVideoPlatform::NowSeconds() const {
  return emscripten_get_now() / 1000.0;
}

#endif // __EMSCRIPTEN__
