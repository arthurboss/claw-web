#include "HapticFeedback.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

bool HapticFeedback::s_Enabled = true;

void HapticFeedback::Trigger(HapticPreset preset, int gamepadIndex)
{
#ifdef __EMSCRIPTEN__
    if (!s_Enabled) return;

    const char* presetName = nullptr;
    switch (preset)
    {
        case HapticPreset::Light:     presetName = "light"; break;
        case HapticPreset::Medium:    presetName = "medium"; break;
        case HapticPreset::Heavy:     presetName = "heavy"; break;
        case HapticPreset::Damage:    presetName = "damage"; break;
        case HapticPreset::Death:     presetName = "death"; break;
        case HapticPreset::Explosion: presetName = "explosion"; break;
        case HapticPreset::Pickup:    presetName = "pickup"; break;
        case HapticPreset::Attack:    presetName = "attack"; break;
        case HapticPreset::Jump:      presetName = "jump"; break;
        default: return;
    }

    EM_ASM({
        if (typeof triggerHaptic === 'function') {
            triggerHaptic($0, UTF8ToString($1));
        }
    }, gamepadIndex, presetName);
#endif
}

void HapticFeedback::TriggerCustom(int durationMs, float weakMagnitude, float strongMagnitude, int gamepadIndex)
{
#ifdef __EMSCRIPTEN__
    if (!s_Enabled) return;

    EM_ASM({
        if (typeof triggerHapticCustom === 'function') {
            triggerHapticCustom($0, $1, $2, $3);
        }
    }, gamepadIndex, durationMs, weakMagnitude, strongMagnitude);
#endif
}

void HapticFeedback::SetEnabled(bool enabled)
{
    s_Enabled = enabled;

#ifdef __EMSCRIPTEN__
    EM_ASM({
        if (typeof setHapticEnabled === 'function') {
            setHapticEnabled($0);
        }
    }, enabled ? 1 : 0);
#endif
}

bool HapticFeedback::IsEnabled()
{
    return s_Enabled;
}
