#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#include <vector>

// Forward declarations
struct TextureData;

// Texture information structure
struct TextureInfo {
    int id;                     // Unique texture ID
    std::string name;           // Texture name/path
    int width, height;          // Texture dimensions
    int format;                 // Texture format (RGBA, RGB, etc.)
    bool hasTransparency;       // Whether texture has alpha channel
    bool isLoaded;              // Loading status
    int memoryUsage;            // Memory usage in bytes
    float lastUsed;             // Last access time for LRU cache
};

// WASM-Only Texture Manager (No SDL Dependencies)
// This system handles texture loading, caching, and management
// for WASM builds without requiring any SDL abstraction layers
class TextureManager {
private:
    // Texture storage
    std::unordered_map<int, std::unique_ptr<TextureInfo>> m_textures;
    std::unordered_map<std::string, int> m_nameToId;
    
    // Memory management
    int m_totalMemoryUsage;
    int m_maxMemoryUsage;
    int m_nextTextureId;
    
    // Performance tracking
    int m_texturesLoaded;
    int m_texturesCached;
    int m_cacheHits;
    int m_cacheMisses;
    
    // Helper methods
    int GenerateTextureId();
    void UpdateLRU(int textureId);
    void EvictLRUTextures(int requiredMemory);
    bool LoadTextureInternal(const std::string& name, int textureId);

public:
    TextureManager();
    ~TextureManager();
    
    // Initialization and shutdown
    bool Initialize();
    void Shutdown();
    
    // Texture loading and management
    int LoadTexture(const std::string& name);
    bool UnloadTexture(int textureId);
    bool UnloadTexture(const std::string& name);
    bool IsTextureLoaded(int textureId) const;
    bool IsTextureLoaded(const std::string& name) const;
    
    // Texture information
    const TextureInfo* GetTextureInfo(int textureId) const;
    const TextureInfo* GetTextureInfo(const std::string& name) const;
    int GetTextureId(const std::string& name) const;
    
    // Memory management
    void SetMaxMemoryUsage(int maxBytes);
    int GetTotalMemoryUsage() const { return m_totalMemoryUsage; }
    int GetMaxMemoryUsage() const { return m_maxMemoryUsage; }
    void ClearCache();
    
    // Performance monitoring
    int GetTexturesLoaded() const { return m_texturesLoaded; }
    int GetTexturesCached() const { return m_texturesCached; }
    int GetCacheHits() const { return m_cacheHits; }
    int GetCacheMisses() const { return m_cacheMisses; }
    float GetCacheHitRate() const;
    
    // Utility methods
    void PreloadTextures(const std::vector<std::string>& textureNames);
    void ReloadAllTextures();
    std::vector<std::string> GetLoadedTextureNames() const;
};
