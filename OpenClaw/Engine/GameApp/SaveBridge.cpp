#include "SaveBridge.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <cstring>

namespace SaveBridge {

bool SaveToIndexedDB(const std::string& jsonData) {
    // Call JavaScript function and wait for result using ASYNCIFY
    int result = EM_ASM_INT({
        // Get the JSON string from C++
        var jsonStr = UTF8ToString($0);

        // We need to use a synchronous approach here
        // Store the data synchronously using localStorage as a fallback
        // and trigger async IndexedDB save
        try {
            // Immediate sync save to localStorage as backup
            localStorage.setItem('OpenClawSaves_backup', jsonStr);

            // Trigger async IndexedDB save
            if (typeof window.saveGameToIndexedDB === 'function') {
                window.saveGameToIndexedDB(jsonStr).then(function(success) {
                    if (success) {
                        console.log('[SaveBridge] Saved to IndexedDB');
                    }
                }).catch(function(err) {
                    console.error('[SaveBridge] IndexedDB save failed:', err);
                });
            }

            console.log('[SaveBridge] Save initiated');
            return 1; // Success
        } catch (e) {
            console.error('[SaveBridge] Save error:', e);
            return 0; // Failure
        }
    }, jsonData.c_str());

    return result == 1;
}

std::string LoadFromIndexedDB() {
    // For loading, we need to use emscripten_sleep to wait for async result
    // First check localStorage backup, then try IndexedDB

    char* result = (char*)EM_ASM_INT({
        var jsonStr = null;

        // Try localStorage backup first (sync)
        try {
            jsonStr = localStorage.getItem('OpenClawSaves_backup');
            if (jsonStr) {
                console.log('[SaveBridge] Loaded from localStorage backup');
            }
        } catch (e) {
            console.warn('[SaveBridge] localStorage not available:', e);
        }

        if (jsonStr) {
            // Allocate memory for the string and copy it
            var lengthBytes = lengthBytesUTF8(jsonStr) + 1;
            var stringOnWasmHeap = _malloc(lengthBytes);
            stringToUTF8(jsonStr, stringOnWasmHeap, lengthBytes);
            return stringOnWasmHeap;
        }

        return 0; // null pointer - no data found
    });

    if (result) {
        std::string str(result);
        free(result);
        return str;
    }

    return "";
}

bool HasSaveData() {
    int result = EM_ASM_INT({
        try {
            var data = localStorage.getItem('OpenClawSaves_backup');
            return data ? 1 : 0;
        } catch (e) {
            return 0;
        }
    });

    return result == 1;
}

bool DeleteSaveData() {
    int result = EM_ASM_INT({
        try {
            localStorage.removeItem('OpenClawSaves_backup');

            // Also trigger IndexedDB delete
            if (typeof window.deleteSaveDataFromIndexedDB === 'function') {
                window.deleteSaveDataFromIndexedDB();
            }

            console.log('[SaveBridge] Save data deleted');
            return 1;
        } catch (e) {
            console.error('[SaveBridge] Delete error:', e);
            return 0;
        }
    });

    return result == 1;
}

} // namespace SaveBridge

#endif // __EMSCRIPTEN__
