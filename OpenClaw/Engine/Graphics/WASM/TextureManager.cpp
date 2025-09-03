#include "TextureManager.h"

// Emscripten includes for JavaScript interop
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include <algorithm>
#include <chrono>

TextureManager::TextureManager()
    : m_totalMemoryUsage(0)
    , m_maxMemoryUsage(100 * 1024 * 1024) // 100MB default
    , m_nextTextureId(1)
    , m_texturesLoaded(0)
    , m_texturesCached(0)
    , m_cacheHits(0)
    , m_cacheMisses(0)
{
}

TextureManager::~TextureManager() {
    Shutdown();
}

bool TextureManager::Initialize() {
    // Initialize JavaScript texture bridge
#ifdef __EMSCRIPTEN__
    EM_ASM({
        if (!window.textureBridge) {
            window.textureBridge = new TextureBridge();
        }
        window.textureBridge.initialize();
    });
    
    EM_ASM({ console.log("TextureManager initialized via JS bridge."); });
#endif
    return true;
}

void TextureManager::Shutdown() {
    // Clear all textures
    ClearCache();
    
    // Shutdown JavaScript bridge
#ifdef __EMSCRIPTEN__
    EM_ASM({
        if (window.textureBridge) {
            window.textureBridge.shutdown();
            delete window.textureBridge;
            window.textureBridge = null;
        }
    });
    
    EM_ASM({ console.log("TextureManager shutdown complete."); });
#endif
}

int TextureManager::GenerateTextureId() {
    return m_nextTextureId++;
}

void TextureManager::UpdateLRU(int textureId) {
    auto it = m_textures.find(textureId);
    if (it != m_textures.end()) {
        auto now = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());
        it->second->lastUsed = duration.count() / 1000.0f;
    }
}

void TextureManager::EvictLRUTextures(int requiredMemory) {
    if (m_totalMemoryUsage + requiredMemory <= m_maxMemoryUsage) {
        return; // No need to evict
    }
    
    // Sort textures by last used time (oldest first)
    std::vector<std::pair<int, float>> textureUsage;
    for (const auto& pair : m_textures) {
        textureUsage.emplace_back(pair.first, pair.second->lastUsed);
    }
    
            std::sort(textureUsage.begin(), textureUsage.end(),
                  [](const std::pair<int, float>& a, const std::pair<int, float>& b) { return a.second < b.second; });
    
    // Evict textures until we have enough memory
    for (const auto& pair : textureUsage) {
        if (m_totalMemoryUsage + requiredMemory <= m_maxMemoryUsage) {
            break;
        }
        
        int textureId = pair.first;
        UnloadTexture(textureId);
    }
}

bool TextureManager::LoadTextureInternal(const std::string& name, int textureId) {
#ifdef __EMSCRIPTEN__
    // Load texture via JavaScript bridge
    bool success = EM_ASM_INT({
        if (window.textureBridge) {
            return window.textureBridge.loadTexture(UTF8ToString($0), $1) ? 1 : 0;
        }
        return 0;
    }, name.c_str(), textureId);
    
    if (success) {
        // Get texture information from JavaScript
        int width = EM_ASM_INT({
            return window.textureBridge ? window.textureBridge.getTextureWidth($0) : 0;
        }, textureId);
        
        int height = EM_ASM_INT({
            return window.textureBridge ? window.textureBridge.getTextureHeight($0) : 0;
        }, textureId);
        
        int format = EM_ASM_INT({
            return window.textureBridge ? window.textureBridge.getTextureFormat($0) : 0;
        }, textureId);
        
        bool hasTransparency = EM_ASM_INT({
            return window.textureBridge ? window.textureBridge.hasTransparency($0) : 0;
        }, textureId);
        
        int memoryUsage = width * height * 4; // Assume RGBA for now
        
        // Create texture info
        std::unique_ptr<TextureInfo> textureInfo(new TextureInfo());
        textureInfo->id = textureId;
        textureInfo->name = name;
        textureInfo->width = width;
        textureInfo->height = height;
        textureInfo->format = format;
        textureInfo->hasTransparency = hasTransparency;
        textureInfo->isLoaded = true;
        textureInfo->memoryUsage = memoryUsage;
        
        auto now = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());
        textureInfo->lastUsed = duration.count() / 1000.0f;
        
        // Store texture
        m_textures[textureId] = std::move(textureInfo);
        m_nameToId[name] = textureId;
        m_totalMemoryUsage += memoryUsage;
        m_texturesLoaded++;
        
        EM_ASM_({ console.log("Texture loaded:", UTF8ToString($0), "ID:", $1, "Size:", $2, "x", $3); },
                name.c_str(), textureId, width, height);
        
        return true;
    }
    
    EM_ASM_({ console.error("Failed to load texture:", UTF8ToString($0)); }, name.c_str());
    return false;
#else
    return false;
#endif
}

int TextureManager::LoadTexture(const std::string& name) {
    // Check if texture is already loaded
    auto it = m_nameToId.find(name);
    if (it != m_nameToId.end()) {
        int textureId = it->second;
        UpdateLRU(textureId);
        m_cacheHits++;
        return textureId;
    }
    
    m_cacheMisses++;
    
    // Check if we need to evict textures
    int estimatedMemory = 1024 * 1024; // Assume 1MB per texture
    EvictLRUTextures(estimatedMemory);
    
    // Generate new texture ID
    int textureId = GenerateTextureId();
    
    // Load the texture
    if (LoadTextureInternal(name, textureId)) {
        return textureId;
    }
    
    return -1; // Failed to load
}

bool TextureManager::UnloadTexture(int textureId) {
    auto it = m_textures.find(textureId);
    if (it == m_textures.end()) {
        return false;
    }
    
    // Unload from JavaScript bridge
#ifdef __EMSCRIPTEN__
    EM_ASM_({
        if (window.textureBridge) {
            window.textureBridge.unloadTexture($0);
        }
    }, textureId);
#endif
    
    // Update memory usage
    m_totalMemoryUsage -= it->second->memoryUsage;
    m_texturesLoaded--;
    
    // Remove from name mapping
    m_nameToId.erase(it->second->name);
    
    // Remove texture
    m_textures.erase(it);
    
    return true;
}

bool TextureManager::UnloadTexture(const std::string& name) {
    auto it = m_nameToId.find(name);
    if (it == m_nameToId.end()) {
        return false;
    }
    
    return UnloadTexture(it->second);
}

bool TextureManager::IsTextureLoaded(int textureId) const {
    auto it = m_textures.find(textureId);
    return it != m_textures.end() && it->second->isLoaded;
}

bool TextureManager::IsTextureLoaded(const std::string& name) const {
    auto it = m_nameToId.find(name);
    return it != m_nameToId.end();
}

const TextureInfo* TextureManager::GetTextureInfo(int textureId) const {
    auto it = m_textures.find(textureId);
    return it != m_textures.end() ? it->second.get() : nullptr;
}

const TextureInfo* TextureManager::GetTextureInfo(const std::string& name) const {
    auto it = m_nameToId.find(name);
    if (it == m_nameToId.end()) {
        return nullptr;
    }
    
    return GetTextureInfo(it->second);
}

int TextureManager::GetTextureId(const std::string& name) const {
    auto it = m_nameToId.find(name);
    return it != m_nameToId.end() ? it->second : -1;
}

void TextureManager::SetMaxMemoryUsage(int maxBytes) {
    m_maxMemoryUsage = maxBytes;
    if (m_totalMemoryUsage > m_maxMemoryUsage) {
        EvictLRUTextures(0); // Evict until we're under the limit
    }
}

void TextureManager::ClearCache() {
    // Unload all textures
    std::vector<int> textureIds;
    for (const auto& pair : m_textures) {
        textureIds.push_back(pair.first);
    }
    
    for (int textureId : textureIds) {
        UnloadTexture(textureId);
    }
    
    m_textures.clear();
    m_nameToId.clear();
    m_totalMemoryUsage = 0;
    m_texturesLoaded = 0;
}

float TextureManager::GetCacheHitRate() const {
    int totalAccesses = m_cacheHits + m_cacheMisses;
    if (totalAccesses == 0) {
        return 0.0f;
    }
    
    return static_cast<float>(m_cacheHits) / static_cast<float>(totalAccesses);
}

void TextureManager::PreloadTextures(const std::vector<std::string>& textureNames) {
    for (const auto& name : textureNames) {
        LoadTexture(name);
    }
}

void TextureManager::ReloadAllTextures() {
    std::vector<std::string> textureNames = GetLoadedTextureNames();
    ClearCache();
    PreloadTextures(textureNames);
}

std::vector<std::string> TextureManager::GetLoadedTextureNames() const {
    std::vector<std::string> names;
    names.reserve(m_textures.size());
    
    for (const auto& pair : m_textures) {
        names.push_back(pair.second->name);
    }
    
    return names;
}
