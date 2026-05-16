# Save System Documentation

## Overview

OpenClaw uses IndexedDB for persistent game saves in the browser, enabling save data to persist across browser sessions.

## Architecture

```
┌─────────────────┐     EM_ASM      ┌──────────────────┐    IndexedDB API   ┌─────────────┐
│   C++ Engine    │ ──────────────► │  save-storage.js │ ─────────────────► │  IndexedDB  │
│ (BaseGameLogic) │                 │   (JS Bridge)    │                    │ (Browser)   │
└─────────────────┘                 └──────────────────┘                    └─────────────┘
        │                                   │
        │                                   ▼
        │                           ┌──────────────────┐
        │                           │  localStorage    │
        │                           │ (sync backup)    │
        └───────────────────────────┴──────────────────┘
```

## Components

### 1. SaveBridge (C++)

**Files:** `OpenClaw/Engine/GameApp/SaveBridge.h`, `SaveBridge.cpp`

C++ interface for JavaScript save operations using Emscripten's `EM_ASM` macro.

```cpp
namespace SaveBridge {
    bool SaveToIndexedDB(const std::string& jsonData);
    std::string LoadFromIndexedDB();
    bool HasSaveData();
    bool DeleteSaveData();
}
```

### 2. save-storage.js (JavaScript)

**File:** `Build_Release/save-storage.js`

ES6 module providing IndexedDB operations with global window functions for C++ bridge.

**Global Functions (called from C++):**
- `window.saveGameToIndexedDB(jsonString)` - Save game data
- `window.loadGameFromIndexedDB()` - Load game data
- `window.hasSaveDataInIndexedDB()` - Check if save exists
- `window.deleteSaveDataFromIndexedDB()` - Delete save data

### 3. GameSaves JSON Serialization (C++)

**File:** `OpenClaw/Engine/GameApp/GameSaves.h`

Added JSON serialization methods to existing save structures:

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
3. `SaveBridge::SaveToIndexedDB()` persists to browser storage

### 2. Level Completion

When a level is completed:

1. `BaseGameLogic::FinishedLevelDelegate()` creates save for next level
2. Next level entry added with checkpoint 0 (start position)
3. Save persisted to IndexedDB

### 3. Game Initialization

On startup (`BaseGameLogic::Initialize()`):

1. Attempts to load save from IndexedDB
2. If no save exists, creates initial Level 1 save
3. Player starts with only unlocked levels available

## IndexedDB API Usage

### Database Structure

- **Database Name:** `OpenClawSaves`
- **Object Store:** `saves`
- **Key:** `gameSaves` (single record for all save data)

### Storage Operations

```javascript
// Opening database
const request = indexedDB.open('OpenClawSaves', 1);

// Creating object store (on upgrade)
db.createObjectStore('saves', { keyPath: 'key' });

// Writing data
const transaction = db.transaction(['saves'], 'readwrite');
const store = transaction.objectStore('saves');
store.put({ key: 'gameSaves', data: saveData, timestamp: Date.now() });

// Reading data
const transaction = db.transaction(['saves'], 'readonly');
const store = transaction.objectStore('saves');
const request = store.get('gameSaves');
```

### Sync/Async Handling

IndexedDB is asynchronous, but C++ needs synchronous access during initialization. Solution:

1. **localStorage backup** - Immediate sync write on every save
2. **IndexedDB primary** - Async write for persistence
3. **Load priority** - Read from localStorage (sync) on startup

## Browser Storage Limits

- **IndexedDB quota:** Typically 50% of free disk space (varies by browser)
- **Save data size:** ~1-5KB typical (JSON text)
- **No practical limit** for game saves

## Debugging

### Browser Console Logs

```
[SaveStorage] Game saved successfully
[SaveStorage] Loaded save data from 5/16/2026, 10:30:45 PM
[SaveBridge] Save initiated
[SaveBridge] Saved to IndexedDB
```

### Inspecting Save Data

1. Open DevTools (F12)
2. Go to **Application** tab
3. Expand **IndexedDB** > **OpenClawSaves** > **saves**
4. Click on `gameSaves` record to view JSON data

### Clearing Save Data

```javascript
// In browser console
window.deleteSaveDataFromIndexedDB();
localStorage.removeItem('OpenClawSaves_backup');
```

## References

- [IndexedDB API (MDN)](https://developer.mozilla.org/en-US/docs/Web/API/IndexedDB_API)
- [Emscripten EM_ASM](https://emscripten.org/docs/api_reference/emscripten.h.html#c.EM_ASM)
- [Web Storage API](https://developer.mozilla.org/en-US/docs/Web/API/Web_Storage_API)
