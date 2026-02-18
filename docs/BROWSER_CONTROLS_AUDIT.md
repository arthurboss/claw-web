# Browser Controls Audit

**Date:** 2026-02-18
**Status:** Analyzed existing functionality

---

## Current Controls Analysis

### 1. Resolution Selector

**HTML:**
```html
<select name="res" id="res" onchange="CheatCodeManager.winsize = this.value; CheatCodeManager.resize()">
    <option value="640 480">640x480</option>
    <option value="800 600">800x600</option>
    <!-- ... more options -->
</select>
```

**How it works:**
- Changes `CheatCodeManager.winsize` variable
- Calls `CheatCodeManager.resize()` which:
  - Sends `WINRESIZE <width> <height> <scale>` console command to game
  - Saves width/height to localStorage
- On page load: Restores from localStorage and sets dropdown value

**Status:** ⚠️ **WORKING BUT HAS ISSUES** - Sends command to game
**Notes:**
- Game must handle `WINRESIZE` console command in C++
- Changes may require page reload to fully take effect
- May not be visually obvious if game doesn't respond to command
- **🐛 KNOWN ISSUE:** Movies/cutscenes do not rescale when resolution changes
  - Videos play at original size regardless of game resolution
  - Need to fix video scaling in the video player system
  - See: `OpenClaw/Engine/Video/Generic/WASM/WasmVideoPlayer.cpp`

---

### 2. Scale Selector

**HTML:**
```html
<select name="scale" id="scale" onchange="CheatCodeManager.scale = this.value; CheatCodeManager.resize()">
    <option value="-1">Original</option>
    <option value="1">1</option>
    <!-- ... more options -->
</select>
```

**How it works:**
- Changes `CheatCodeManager.scale` variable
- Calls same `CheatCodeManager.resize()` function
- Saves scale to localStorage
- Restored on page load

**Status:** ✅ **WORKING** - Sends command to game
**Notes:**
- Scale affects game rendering scale (internal pixels per screen pixel)
- `-1` = "Original" means use game's default scale
- May not be visually obvious in browser

---

### 3. Fullscreen Button

**HTML:**
```html
<button onclick="Module.canvas.requestFullscreen()">Enable Fullscreen Mode</button>
```

**How it works:**
- Uses browser's native Fullscreen API
- Requests fullscreen for the canvas element only
- No localStorage persistence

**Status:** ✅ **WORKING** - Browser native feature
**Issues:**
- Button text doesn't change when fullscreen is active
- No exit fullscreen button (must press ESC)
- No indication that ESC exits fullscreen

---

### 4. Debug Panel (Hidden)

**HTML:**
```html
<div class="debug_panel" style="display:none">
    <!-- Cheat codes -->
    <button onclick="CheatCodeManager.infiniteJumps()">Infinite Jumps</button>
    <!-- ... more cheats -->
</div>
```

**How it works:**
- Hidden by default (`display:none`)
- ~~No keyboard shortcut to toggle~~ (can be added)
- Sends console commands like `INFINITEJUMP ON/OFF`
- Simulates keyboard input to open console and type commands

**Status:** ⚠️ **HIDDEN BUT FUNCTIONAL**
**Issues:**
- No way to access without editing HTML
- Should have keyboard shortcut (Ctrl+Shift+D)
- Cheat code logic is exposed in source code

---

### 5. Loading Indicator

**HTML:**
```html
<span id="loading"></span>
```

**JavaScript:**
```javascript
Module.setStatus: function (text) {
    // Updates loading indicator with download progress
    loadingElement.innerHTML = "Game resources are downloading... " + progress + "%";
}
```

**Status:** ✅ **WORKING**
**Notes:**
- Shows download progress for `openclaw.data` file
- Removed after game loads
- Could be styled better

---

## Missing Controls

### 1. Audio Controls ❌
- No mute/unmute button
- No volume slider
- Game has audio but no browser control for it

### 2. Help/Controls ❌
- No keyboard controls documentation
- No "How to Play" button
- Players don't know which keys do what

### 3. Settings Persistence ⚠️
- Resolution/scale persist to localStorage ✅
- Fullscreen preference doesn't persist ❌
- Audio settings can't persist (no audio control) ❌

### 4. Visual Feedback ❌
- No indication when settings change
- No "Settings saved" message
- No active state for buttons

---

## Issues & Problems

### User Experience Issues

1. **Resolution/Scale don't appear to work**
   - Reason: Changes may be subtle or require reload
   - Fix: Add visual confirmation message
   - Fix: Document what these controls do

2. **No feedback on button clicks**
   - Fix: Add visual button states (active, hover)
   - Fix: Add toast notifications for actions

3. **Cramped layout**
   - Controls are all inline, hard to read
   - Fix: Better spacing and organization

4. **No styling**
   - Uses browser default button styles
   - Looks unprofessional
   - Fix: Add modern CSS styling

5. **Cheat codes visible in source**
   - Anyone can see cheat commands
   - Fix: Obfuscate or move to separate file
   - Fix: Add proper debug mode toggle

### Technical Issues

1. **Resolution/Scale timing**
   - localStorage is read on load
   - Changes might not apply until reload
   - Fix: Apply settings immediately AND on load

2. **No error handling**
   - If game doesn't support `WINRESIZE` command, no feedback
   - Fix: Add error handling or validation

3. **Hard-coded console commands**
   - CheatCodeManager directly types into console
   - Brittle if console key changes
   - Fix: Use Emscripten bindings instead

4. **🐛 Video scaling bug**
   - Movies/cutscenes don't rescale when resolution changes
   - Videos play at original resolution regardless of game resolution setting
   - Affects: Menu background videos, cutscenes between levels
   - Root cause: Video player doesn't listen to resolution change events
   - Fix location: `OpenClaw/Engine/Video/Generic/WASM/WasmVideoPlayer.cpp`
   - Priority: Medium (doesn't break gameplay, just looks off)

---

## Recommendations

### High Priority (Phase 1)

1. **Add visual feedback**
   - Style buttons with CSS
   - Add hover/active states
   - Show toast/notification when settings change

2. **Add audio controls**
   - Mute/unmute button
   - Volume slider (if supported by game)

3. **Add help button**
   - Shows modal with keyboard controls
   - Links to GitHub/documentation

4. **Hide debug panel properly**
   - Add Ctrl+Shift+D keyboard shortcut to toggle
   - Remove from default view

5. **Improve layout**
   - Organize controls into logical groups
   - Add spacing and proper alignment
   - Make it responsive

### Medium Priority

6. **Better loading screen**
   - Styled progress bar instead of text
   - Game logo or animation
   - Tips or instructions while loading

7. **Settings panel**
   - Collapsible settings group
   - Clear labels and descriptions
   - "Apply" button to confirm changes

8. **Persist fullscreen preference**
   - Remember if user prefers fullscreen
   - Auto-enter fullscreen on load (if preferred)

### Low Priority (Phase 2)

9. **In-game menu integration**
   - Add browser controls to in-game menu
   - Save/load via localStorage
   - Deeper integration with game

---

## Testing Plan

### Manual Testing

1. ✅ Test resolution selector
   - Change resolution dropdown
   - Check if `WINRESIZE` command is sent (check console)
   - Reload page and verify settings persist

2. ✅ Test scale selector
   - Change scale dropdown
   - Check if command is sent
   - Verify localStorage saves

3. ✅ Test fullscreen button
   - Click fullscreen
   - Verify canvas goes fullscreen
   - Press ESC and verify exit

4. ❌ Test audio (when controls added)
   - Toggle mute button
   - Verify game audio mutes
   - Check persistence

### Browser Testing

- Chrome/Edge (latest)
- Firefox (latest)
- Safari (if available)
- Mobile browsers (optional)

---

## Next Steps

1. ✅ Document current controls (this file)
2. Create mockup for new UI layout
3. Implement modern CSS styling
4. Add audio toggle button
5. Add help modal
6. Add debug panel keyboard shortcut
7. Test all controls thoroughly
8. Iterate based on feedback

---

## References

- HTML file: `Build_Release/openclaw.html`
- CheatCodeManager: Lines 121-184
- Console commands: Game must handle in C++ console system
- localStorage: Browser API, persists across page loads
