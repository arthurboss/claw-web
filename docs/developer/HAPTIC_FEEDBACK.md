# Haptic Feedback (Gamepad Vibration)

Haptic feedback provides tactile responses through gamepad vibration for various game events.

## Browser Support

- Chrome 105+, Firefox 121+, Edge 105+ (full support)
- Safari has limited vibration support

## Haptic Presets

| Preset | Duration | Intensity | Used For |
|--------|----------|-----------|----------|
| `light` | 50ms | Low | Menu navigation, short landings |
| `medium` | 100ms | Medium | Menu select/back, hard landings |
| `heavy` | 150ms | High | (Available for future use) |
| `damage` | 200ms | High | Taking damage, dealing damage to enemies |
| `death` | 500ms | Maximum | Player death |
| `explosion` | 300ms | High | Crate/barrel explosions |
| `pickup` | 40ms | Very low | Collecting coins and items |
| `attack` | 60ms | Low | Sword attacks, firing projectiles |

## Where Haptics Trigger

### Menu (JavaScript - gamepad-bridge.js)

| Action | Preset |
|--------|--------|
| D-pad navigation | `light` |
| Analog stick navigation | `light` |
| A/Start (select) | `medium` |
| B (back) | `medium` |

### Gameplay (C++)

| Action | Preset | File |
|--------|--------|------|
| Player takes damage | `damage` | HumanView.cpp |
| Player dies | `death` | HumanView.cpp |
| Player hits enemy | `damage` | HealthComponent.cpp |
| Collect item/coin | `pickup` | BaseGameLogic.cpp |
| Sword attack | `attack` | ActorController.cpp |
| Fire projectile | `attack` | ActorController.cpp |
| Land (short jump) | `light` | ControllableComponent.cpp |
| Land (high fall) | `medium` | ControllableComponent.cpp |
| Crate explosion | `explosion` | ExplodeableComponent.cpp |

## Debug Commands (Browser Console)

```javascript
gamepadDebug()        // Show gamepad and vibration support status
testHaptic('damage')  // Test a specific preset
setHapticEnabled(false) // Disable haptic feedback
```

## Files

- `Build_Release/gamepad-bridge.js` - JavaScript haptic functions and presets
- `OpenClaw/Engine/GameApp/HapticFeedback.h` - C++ API
- `OpenClaw/Engine/GameApp/HapticFeedback.cpp` - C++ implementation (calls JS via EM_ASM)
