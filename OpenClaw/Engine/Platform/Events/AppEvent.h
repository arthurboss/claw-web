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

  Resize,

  // Gamepad events (using browser Gamepad API)
  GamepadConnected,
  GamepadDisconnected,
  GamepadButtonDown,
  GamepadButtonUp,
  GamepadAxis
};

// Standard gamepad button mapping (matches W3C "standard" mapping)
// https://w3c.github.io/gamepad/#remapping
enum class GamepadButton : uint8_t {
  A = 0,              // Bottom button (Xbox A, PS Cross)
  B = 1,              // Right button (Xbox B, PS Circle)
  X = 2,              // Left button (Xbox X, PS Square)
  Y = 3,              // Top button (Xbox Y, PS Triangle)
  LeftBumper = 4,     // LB / L1
  RightBumper = 5,    // RB / R1
  LeftTrigger = 6,    // LT / L2 (as button)
  RightTrigger = 7,   // RT / R2 (as button)
  Back = 8,           // Back / Select / View
  Start = 9,          // Start / Menu
  LeftStick = 10,     // L3 (stick press)
  RightStick = 11,    // R3 (stick press)
  DPadUp = 12,
  DPadDown = 13,
  DPadLeft = 14,
  DPadRight = 15,
  Guide = 16,         // Xbox button / PS button
  Count = 17
};

enum class GamepadAxis : uint8_t {
  LeftStickX = 0,     // -1.0 (left) to 1.0 (right)
  LeftStickY = 1,     // -1.0 (up) to 1.0 (down)
  RightStickX = 2,
  RightStickY = 3,
  Count = 4
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

struct AppGamepadConnectionEvent {
  int32_t gamepadIndex = 0;  // Which gamepad (0-3)
};

struct AppGamepadButtonEvent {
  int32_t gamepadIndex = 0;
  GamepadButton button = GamepadButton::A;
  float value = 0.0f;        // Analog value (0.0-1.0 for triggers)
};

struct AppGamepadAxisEvent {
  int32_t gamepadIndex = 0;
  GamepadAxis axis = GamepadAxis::LeftStickX;
  float value = 0.0f;        // -1.0 to 1.0
};

struct AppEvent {
  AppEventType type = AppEventType::None;

  // Payload (only one is meaningful depending on type).
  AppKeyEvent key;
  AppMouseMoveEvent mouseMove;
  AppMouseButtonEvent mouseButton;
  AppResizeEvent resize;
  AppGamepadConnectionEvent gamepadConnection;
  AppGamepadButtonEvent gamepadButton;
  AppGamepadAxisEvent gamepadAxis;

  static AppEvent MakeQuit() {
    AppEvent e;
    e.type = AppEventType::Quit;
    return e;
  }

  static AppEvent MakeGamepadConnected(int32_t index) {
    AppEvent e;
    e.type = AppEventType::GamepadConnected;
    e.gamepadConnection.gamepadIndex = index;
    return e;
  }

  static AppEvent MakeGamepadDisconnected(int32_t index) {
    AppEvent e;
    e.type = AppEventType::GamepadDisconnected;
    e.gamepadConnection.gamepadIndex = index;
    return e;
  }
};
