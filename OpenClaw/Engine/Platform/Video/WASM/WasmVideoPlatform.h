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
  // Returns: 0=unknown, 1=menu, 2=in-game, 3=paused, 4=cutscene
  EMSCRIPTEN_KEEPALIVE int GetJSGameState();
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
