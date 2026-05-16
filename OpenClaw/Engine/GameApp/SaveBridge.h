#ifndef __SAVE_BRIDGE_H__
#define __SAVE_BRIDGE_H__

#include <string>

#ifdef __EMSCRIPTEN__

namespace SaveBridge {
    // Save game data to IndexedDB (JSON format)
    // Returns true on success
    bool SaveToIndexedDB(const std::string& jsonData);

    // Load game data from IndexedDB
    // Returns JSON string, or empty string if no save exists
    std::string LoadFromIndexedDB();

    // Check if save data exists in IndexedDB
    bool HasSaveData();

    // Delete save data from IndexedDB
    bool DeleteSaveData();
}

#endif // __EMSCRIPTEN__

#endif // __SAVE_BRIDGE_H__
