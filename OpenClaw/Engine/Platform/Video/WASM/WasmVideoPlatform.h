#pragma once

#ifdef __EMSCRIPTEN__

#include "../VideoPlatform.h"

class AppEventQueue;

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
