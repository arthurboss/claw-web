#ifndef HAPTICFEEDBACK_H_
#define HAPTICFEEDBACK_H_

#include "../SharedDefines.h"

enum class HapticPreset : uint8_t
{
    Light,
    Medium,
    Heavy,
    Damage,
    Death,
    Explosion,
    Pickup,
    Attack,
    Jump
};

class HapticFeedback
{
public:
    static void Trigger(HapticPreset preset, int gamepadIndex = 0);
    static void TriggerCustom(int durationMs, float weakMagnitude, float strongMagnitude, int gamepadIndex = 0);
    static void SetEnabled(bool enabled);
    static bool IsEnabled();

private:
    static bool s_Enabled;
};

#endif // HAPTICFEEDBACK_H_
