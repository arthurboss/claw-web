#pragma once

#ifdef __EMSCRIPTEN__

#include "../VideoPlatform.h"
#include <emscripten/emscripten.h>

class AppEventQueue;

// JavaScript Gamepad Bridge callbacks (called from gamepad-bridge.js via Module.ccall)
// These are exported to JavaScript and receive events from the browser's Gamepad API
extern "C" {
  EMSCRIPTEN_KEEPALIVE void OnJSGamepadConnected(int index);
  EMSCRIPTEN_KEEPALIVE void OnJSGamepadDisconnected(int index);
  EMSCRIPTEN_KEEPALIVE void OnJSGamepadButtonDown(int index, int button, float value);
  EMSCRIPTEN_KEEPALIVE void OnJSGamepadButtonUp(int index, int button);
  EMSCRIPTEN_KEEPALIVE void OnJSGamepadAxis(int index, int axis, float value);

  // Pointer Events bridge callbacks (called from pointer-bridge.js).
  // Coordinates are in window (device-pixel canvas) space. ptype: 0=mouse,1=touch,2=pen.
  EMSCRIPTEN_KEEPALIVE void OnJSPointerDown(int pointerId, int x, int y, int ptype, int button);
  EMSCRIPTEN_KEEPALIVE void OnJSPointerMove(int pointerId, int x, int y, int ptype);
  EMSCRIPTEN_KEEPALIVE void OnJSPointerUp(int pointerId, int x, int y, int ptype, int button);

  // Called once from pointer-bridge.js with navigator.maxTouchPoints > 0.
  EMSCRIPTEN_KEEPALIVE void OnJSTouchCapability(int isTouchDevice);

  // Returns: 0=unknown, 1=menu, 2=in-game, 3=paused, 4=cutscene
  EMSCRIPTEN_KEEPALIVE int GetJSGameState();
  // Dynamic resolution support for fullscreen mode
  EMSCRIPTEN_KEEPALIVE void OnJSResolutionChange(int width, int height);
  EMSCRIPTEN_KEEPALIVE int GetGameWidth();
  EMSCRIPTEN_KEEPALIVE int GetGameHeight();
}

class WasmVideoPlatform final : public VideoPlatform {
public:
  WasmVideoPlatform();
  ~WasmVideoPlatform() override;

  bool Init(int32_t initialWidth, int32_t initialHeight) override;
  void Shutdown() override;

  void SetEventQueue(AppEventQueue *queue) override;
  void PumpEvents() override;
  double NowSeconds() const override;

  AppEventQueue *GetEventQueue() const { return m_queue; }

private:
  AppEventQueue *m_queue;
  bool m_installedCallbacks;
};

#endif // __EMSCRIPTEN__
