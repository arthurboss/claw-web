#pragma once

#include "../VideoPlatform.h"

class NullVideoPlatform final : public VideoPlatform {
public:
  bool Init(int32_t /*initialWidth*/, int32_t /*initialHeight*/) override {
    return true;
  }
  void Shutdown() override {}

  void SetEventQueue(AppEventQueue * /*queue*/) override {}
  void PumpEvents() override {}
  double NowSeconds() const override { return 0.0; }
};
