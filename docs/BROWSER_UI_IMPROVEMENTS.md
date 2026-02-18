# Browser UI Improvements - Ideation & Implementation Plan

**Status:** Phase 1 - Browser UI Cleanup (HTML/CSS/JS)
**Branch:** `feat/improve-browser-ui`
**Date:** 2026-02-18

---

## Current State Analysis

### Existing UI Elements (openclaw.html)

1. **Canvas** - Game rendering area (640x480 default)
2. **Fullscreen Button** - Requests fullscreen mode
3. **Resolution Selector** - Dropdown with 6 preset resolutions (640x480 to 2000x1500)
4. **Scale Selector** - Dropdown with scale options (-1 to 2)
5. **Debug Panel** (hidden) - Cheat codes and console
6. **Loading Indicator** - Shows during initial load

### Issues Identified

- ❌ No visible styling - buttons use browser defaults
- ❌ Resolution/scale controls may not be working correctly
- ❌ No audio controls (mute/unmute)
- ❌ No help/controls information for players
- ❌ Cheat codes visible in source (should be hidden better)
- ❌ Not responsive - doesn't adapt to different screen sizes
- ❌ No visual feedback on button states
- ❌ Controls are cramped and unclear

---

## Phase 1: Browser UI Cleanup (Current Phase)

### Objectives
- Clean, modern, professional appearance
- Test and fix resolution/scale functionality
- Hide debug/cheat features properly
- Add essential missing controls (audio, help)
- Make UI responsive and user-friendly

### Tasks

#### 1. **Audit Existing Functionality** ✅ NEXT
- [ ] Test resolution selector - does it actually change the game resolution?
- [ ] Test scale selector - does it scale the canvas/graphics?
- [ ] Check if changes persist (localStorage)
- [ ] Document what each control actually does

#### 2. **Design Modern UI Layout**
- [ ] Create mockup/wireframe for new layout
- [ ] Design control panel (collapsible/overlay?)
- [ ] Choose color scheme (dark theme? match game aesthetic?)
- [ ] Select icon library (Font Awesome? SVG icons?)

#### 3. **Implement Core Styling**
- [ ] Add modern CSS with CSS variables for theming
- [ ] Style buttons with hover/active states
- [ ] Style dropdowns and inputs
- [ ] Add smooth transitions and animations
- [ ] Make canvas border/container look professional
- [ ] Add loading screen with progress animation

#### 4. **Add Missing Controls**
- [ ] **Audio Toggle** - Mute/unmute button with icon
- [ ] **Help/Controls Button** - Shows modal with keyboard controls
- [ ] **Settings Panel** - Collapsible panel with resolution/scale/audio
- [ ] **Debug Toggle** - Hidden keyboard shortcut (Ctrl+Shift+D)

#### 5. **Improve Existing Controls**
- [ ] Fix resolution selector (if broken)
- [ ] Fix scale selector (if broken)
- [ ] Add visual feedback when settings change
- [ ] Show current resolution/scale values
- [ ] Add tooltips to explain what each control does

#### 6. **Responsive Design**
- [ ] Make UI work on mobile (touch controls?)
- [ ] Adapt layout for different screen sizes
- [ ] Ensure canvas scales appropriately
- [ ] Test on various devices/browsers

---

## Phase 2: In-Game Integration (Future)

### Objectives
- Add browser features to in-game menus
- Save/load game state to localStorage
- Deeper integration between browser UI and game

### Potential Features

#### 1. **Save/Load System**
- Add "Save Progress" to in-game menu
- Add "Load Progress" to in-game menu
- Store save data in browser localStorage
- Implement via Emscripten JS bindings

#### 2. **Custom Menu Items**
- Modify XML menu definitions to add new items
- Wire up menu items to JavaScript functions
- Examples:
  - "Browser Settings" - Opens settings overlay
  - "Keyboard Controls" - Shows help modal
  - "Full Screen" - Toggles fullscreen

#### 3. **Enhanced Browser Features**
- Screenshot button (capture canvas to image)
- Recording button (capture gameplay video)
- Share score/progress to social media
- Achievements/progress tracking
- Cloud save sync

---

## Design Recommendations

### Visual Style
- **Theme:** Dark mode with game-appropriate colors (match Captain Claw aesthetic)
- **Typography:** Clean, readable fonts (not pixelated - save that for the game)
- **Icons:** Modern, minimal icons for buttons
- **Layout:** Controls at bottom or collapsible side panel
- **Effects:** Subtle animations, smooth transitions

### Control Panel Layout (Suggested)

```
┌─────────────────────────────────────────┐
│         OPENCLAW - Captain Claw          │
│                                          │
│  ┌────────────────────────────────────┐  │
│  │                                    │  │
│  │         GAME CANVAS (640x480)      │  │
│  │                                    │  │
│  └────────────────────────────────────┘  │
│                                          │
│  [⛶ Fullscreen] [🔊 Audio] [? Help]      │
│  [⚙ Settings ▼]                          │
│    └─ Resolution: [800x600 ▼]            │
│    └─ Scale: [1.0 ▼]                     │
│    └─ Volume: [█████░░░░░] 50%           │
└─────────────────────────────────────────┘
```

### Alternative: Floating Control Bar

```
┌─────────────────────────────────────────┐
│  ┌────────────────────────────────────┐  │
│  │                                    │  │
│  │         GAME CANVAS                │  │
│  │                                    │  │
│  │  [⛶] [🔊] [?] [⚙]  ← Floating bar │  │
│  │                      (bottom-right)│  │
│  └────────────────────────────────────┘  │
└─────────────────────────────────────────┘
```

---

## Technical Implementation Notes

### Browser Controls Testing
Before implementing new UI, need to verify:
1. Does resolution selector actually resize the game?
2. Does scale selector affect rendering scale?
3. How does the game handle resolution changes?
4. Is there lag when changing settings?

### Audio Control Implementation
- Emscripten provides audio context access
- Can mute via `SDL_mixer` or Web Audio API
- Need to check if game exposes audio control functions
- May need to add Emscripten binding for mute toggle

### Keyboard Shortcuts
- Fullscreen: F11 (browser native) or F key
- Mute: M key
- Help: H or F1 key
- Debug panel: Ctrl+Shift+D
- Pause: P or ESC (if not already used)

### LocalStorage Structure
```javascript
{
  "openclaw": {
    "settings": {
      "resolution": "800 600",
      "scale": 1.0,
      "volume": 0.5,
      "muted": false
    },
    "preferences": {
      "fullscreenOnStart": false,
      "showHelp": true
    }
  }
}
```

---

## Success Criteria

### Phase 1 Complete When:
- ✅ All controls have modern, polished styling
- ✅ Resolution and scale selectors work correctly
- ✅ Audio mute/unmute button implemented
- ✅ Help modal shows keyboard controls
- ✅ Debug panel hidden behind keyboard shortcut
- ✅ UI is responsive on desktop browsers
- ✅ Loading screen looks professional
- ✅ Settings persist across page reloads

### Phase 2 Complete When:
- ✅ Save/load works from in-game menu
- ✅ Browser settings accessible from game menu
- ✅ Cloud save sync implemented (optional)
- ✅ Screenshots/recording features added (optional)

---

## Known Issues to Fix Later

### Video Scaling Bug 🐛
**Issue:** Movies/cutscenes do not rescale when game resolution changes
- Videos play at original size regardless of resolution setting
- Affects menu background videos and level cutscenes
- Root cause: Video player doesn't respond to resolution change events
- Fix location: `OpenClaw/Engine/Video/Generic/WASM/WasmVideoPlayer.cpp`
- Priority: Medium (cosmetic issue, doesn't break gameplay)

**Technical details:**
- Resolution changes send `WINRESIZE` command to game
- Video player needs to listen for this event and rescale video canvas
- May need to add resolution change callback system

---

## Open Questions

1. ✅ **Resolution/Scale**: Confirmed working - sends `WINRESIZE` console command
2. **Audio API**: Does the game expose audio control via Emscripten?
3. **Save System**: Does the game already have a save system we can tap into?
4. **Mobile Support**: Should we support touch controls in Phase 1?
5. **Theming**: Should we match the game's pirate/claw aesthetic or keep it modern/minimal?

---

## Next Steps

1. **Immediate**: Test existing resolution/scale controls
2. Create basic HTML structure with improved layout
3. Add modern CSS styling
4. Implement audio toggle
5. Add help modal
6. Test and iterate

---

## References

- Current HTML: `Build_Release/openclaw.html`
- Game Menu System: `OpenClaw/Engine/UserInterface/UserInterface.cpp`
- Emscripten Bindings: Need to check for audio/save bindings
- Similar Projects: Look at other Emscripten game UIs for inspiration
