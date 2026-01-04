#pragma once

#include <cstdint>

class AppEventQueue;

class VideoPlatform {
public:
  virtual ~VideoPlatform() = default;

  virtual bool Init(int32_t initialWidth, int32_t initialHeight) = 0;
  virtual void Shutdown() = 0;

  virtual void SetEventQueue(AppEventQueue *queue) = 0;

  // Desktop backends may poll native event loops; WASM implementation is a
  // no-op.
  virtual void PumpEvents() = 0;

  // Monotonic time source.
  virtual double NowSeconds() const = 0;
};
