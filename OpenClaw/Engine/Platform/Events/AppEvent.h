#pragma once

#include <cstdint>

enum class AppEventType : uint8_t {
  None = 0,
  Quit,

  KeyDown,
  KeyUp,

  MouseMove,
  MouseDown,
  MouseUp,

  Resize
};

// Keep this minimal for Stage 2a.
// For now we carry scancode/keycode values that BaseGameApp can translate back
// to SDL_Event.
struct AppKeyEvent {
  int32_t scancode = 0;
  int32_t keycode = 0;
  bool repeat = false;
};

struct AppMouseMoveEvent {
  float x = 0.0f;
  float y = 0.0f;
};

struct AppMouseButtonEvent {
  // SDL mouse button values (e.g. SDL_BUTTON_LEFT).
  uint8_t button = 0;
  float x = 0.0f;
  float y = 0.0f;
};

struct AppResizeEvent {
  int32_t width = 0;
  int32_t height = 0;
  float dpiScale = 1.0f;
};

struct AppEvent {
  AppEventType type = AppEventType::None;

  // Payload (only one is meaningful depending on type).
  AppKeyEvent key;
  AppMouseMoveEvent mouseMove;
  AppMouseButtonEvent mouseButton;
  AppResizeEvent resize;

  static AppEvent MakeQuit() {
    AppEvent e;
    e.type = AppEventType::Quit;
    return e;
  }
};
