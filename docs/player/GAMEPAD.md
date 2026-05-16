# Gamepad Support

OpenClaw supports Xbox, PlayStation, and most standard game controllers.

## Browser Support

- Chrome 105+, Firefox 121+, Edge 105+ (full support)
- Safari has limited controller support

## Controls (Xbox Layout)

| Button | Action |
|--------|--------|
| A | Jump |
| B | Fire (ranged weapon) |
| X | Attack (sword) |
| LB | Previous weapon |
| RB | Next weapon |
| Start | Pause menu |
| D-pad | Move / Look up/down |
| Left Stick | Move / Look up/down |

**PlayStation equivalents:** Cross=A, Circle=B, Square=X, Triangle=Y, L1=LB, R1=RB

## Vibration Feedback

The controller vibrates for various game events:

| Event | Vibration |
|-------|-----------|
| Menu navigation | Light pulse |
| Menu select/back | Medium pulse |
| Taking damage | Strong pulse |
| Player death | Long strong pulse |
| Hitting enemies | Strong pulse |
| Collecting items | Very light pulse |
| Sword attack | Light pulse |
| Firing projectile | Light pulse |
| Landing from jump | Light to medium pulse |
| Explosions | Strong pulse |

## Troubleshooting

**Controller not detected:**

1. Connect controller before opening the game
2. Ensure the game tab is selected on the browser
3. Press any button to activate it
4. Check browser permissions for gamepad access

**No vibration:**

1. Check if your controller supports vibration
2. Safari has limited vibration support - try Chrome or Firefox
3. Open browser console (F12) and run `gamepadDebug()` to check status

**Wrong button mapping:**

- The game uses standard Xbox-style mapping
- Third-party controllers may have different layouts
