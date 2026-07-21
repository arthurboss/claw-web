# Save System Documentation

## Overview

Claw Web stores game save data in localStorage under the key `claw-web:saves`. Save data is tiny (~1-5KB JSON).

## Architecture

```
┌─────────────────┐     EM_ASM      ┌────────────────┐
│   C++ Engine    │ ──────────────► │  SaveBridge    │ ──► localStorage
│ (BaseGameLogic) │                 │  (C++/JS glue) │
└─────────────────┘                 └────────────────┘
```

## Components

### 1. SaveBridge (C++)

**Files:** `OpenClaw/Engine/GameApp/SaveBridge.h`, `SaveBridge.cpp`

C++ interface for localStorage save operations using Emscripten's `EM_ASM` macro.

```cpp
namespace SaveBridge {
    bool SaveToIndexedDB(const std::string& jsonData);  // saves to localStorage
    std::string LoadFromIndexedDB();                    // loads from localStorage
    bool HasSaveData();
    bool DeleteSaveData();
}
```

Note: function names retain the original `IndexedDB` naming for C++ call-site compatibility, but the implementation uses localStorage exclusively.

### 2. GameSaves JSON Serialization (C++)

**File:** `OpenClaw/Engine/GameApp/GameSaves.h`

JSON serialization methods on save structures:

- `CheckpointSave::ToJson()` / `LoadFromJson()`
- `LevelSave::ToJson()` / `LoadFromJson()`
- `GameSaveMgr::ToJson()` / `InitializeFromJson()`

## Save Data Format

```json
{
  "version": 1,
  "levels": [
    {
      "levelNumber": 1,
      "levelName": "La Roca",
      "checkpoints": [
        {
          "checkpointIdx": 0,
          "score": 12500,
          "health": 85,
          "lives": 5,
          "bulletCount": 12,
          "magicCount": 3,
          "dynamiteCount": 2
        }
      ]
    }
  ]
}
```

## Save Triggers

### 1. Checkpoint Flags (In-Level)

When the player touches a checkpoint flag with `IsSaveCheckpoint=true`:

1. `CheckpointComponent::VOnApply()` fires `EventData_Checkpoint_Reached`
2. `BaseGameLogic::CheckpointReachedDelegate()` captures player state
3. `SaveBridge::SaveToIndexedDB()` persists to localStorage

### 2. Level Completion

When a level is completed:

1. `BaseGameLogic::FinishedLevelDelegate()` creates save for next level
2. Next level entry added with checkpoint 0 (start position)
3. Save persisted to localStorage

### 3. Game Initialization

On startup (`BaseGameLogic::Initialize()`):

1. Attempts to load save from localStorage
2. If no save exists, creates initial Level 1 save
3. Player starts with only unlocked levels available

## Debugging

### Inspecting Save Data

```javascript
// In browser console
JSON.parse(localStorage.getItem('claw-web:saves'));
```

### Clearing Save Data

```javascript
// In browser console
localStorage.removeItem('claw-web:saves');
```

## References

- [Web Storage API (MDN)](https://developer.mozilla.org/en-US/docs/Web/API/Web_Storage_API)
- [Emscripten EM_ASM](https://emscripten.org/docs/api_reference/emscripten.h.html#c.EM_ASM)
