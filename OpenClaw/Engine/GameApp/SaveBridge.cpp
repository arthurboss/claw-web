#include "SaveBridge.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <cstring>

namespace SaveBridge {

bool SaveToIndexedDB(const std::string& jsonData) {
    int result = EM_ASM_INT({
        try {
            localStorage.setItem('openclaw:saves', UTF8ToString($0));
            return 1;
        } catch (e) {
            console.error('[SaveBridge] Save error:', e);
            return 0;
        }
    }, jsonData.c_str());

    return result == 1;
}

std::string LoadFromIndexedDB() {
    char* result = (char*)EM_ASM_INT({
        try {
            var jsonStr = localStorage.getItem('openclaw:saves');
            if (!jsonStr) return 0;
            var lengthBytes = lengthBytesUTF8(jsonStr) + 1;
            var ptr = _malloc(lengthBytes);
            stringToUTF8(jsonStr, ptr, lengthBytes);
            return ptr;
        } catch (e) {
            console.warn('[SaveBridge] Load error:', e);
            return 0;
        }
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
            return localStorage.getItem('openclaw:saves') ? 1 : 0;
        } catch (e) {
            return 0;
        }
    });

    return result == 1;
}

bool DeleteSaveData() {
    int result = EM_ASM_INT({
        try {
            localStorage.removeItem('openclaw:saves');
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
