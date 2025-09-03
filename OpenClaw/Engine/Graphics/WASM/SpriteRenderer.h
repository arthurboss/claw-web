#pragma once

#include <vector>
#include <memory>
#include <unordered_map>

// Forward declarations
class Sprite;
class IRenderer;
class TextureManager;

#ifdef __EMSCRIPTEN__
// WASM builds only - no SDL dependencies

/**
 * @brief SpriteRenderer for efficient batch rendering of sprites
 * 
 * This class manages the rendering of multiple sprites with performance
 * optimizations including batching, texture binding, and state management.
 */
class SpriteRenderer {
public:
    /**
     * @brief Constructor
     * @param textureManager Pointer to the texture manager
     */
    explicit SpriteRenderer(TextureManager* textureManager);
    
    /**
     * @brief Destructor
     */
    ~SpriteRenderer();
    
    /**
     * @brief Initialize the sprite renderer
     * @return True if initialization successful, false otherwise
     */
    bool Initialize();
    
    /**
     * @brief Shutdown the sprite renderer
     */
    void Shutdown();
    
    /**
     * @brief Add a sprite to the render queue
     * @param sprite The sprite to add
     */
    void AddSprite(const Sprite* sprite);
    
    /**
     * @brief Remove a sprite from the render queue
     * @param sprite The sprite to remove
     */
    void RemoveSprite(const Sprite* sprite);
    
    /**
     * @brief Clear all sprites from the render queue
     */
    void ClearSprites();
    
    /**
     * @brief Render all sprites in the queue
     * @param renderer The renderer to use for rendering
     */
    void RenderSprites(IRenderer* renderer);
    
    /**
     * @brief Set the maximum number of sprites that can be batched together
     * @param maxBatchSize The maximum batch size
     */
    void SetMaxBatchSize(size_t maxBatchSize);
    
    /**
     * @brief Get the current number of sprites in the queue
     * @return The number of sprites
     */
    size_t GetSpriteCount() const { return m_sprites.size(); }
    
    /**
     * @brief Get the maximum batch size
     * @return The maximum batch size
     */
    size_t GetMaxBatchSize() const { return m_maxBatchSize; }
    
    /**
     * @brief Check if the sprite renderer is initialized
     * @return True if initialized, false otherwise
     */
    bool IsInitialized() const { return m_isInitialized; }
    
    /**
     * @brief Get performance statistics
     * @return String containing performance stats
     */
    std::string GetPerformanceStats() const;

private:
    /**
     * @brief Sort sprites by texture ID for efficient batching
     */
    void SortSpritesByTexture();
    
    /**
     * @brief Render a batch of sprites with the same texture
     * @param renderer The renderer to use
     * @param textureId The texture ID for this batch
     * @param startIndex Start index of sprites in this batch
     * @param endIndex End index of sprites in this batch
     */
    void RenderSpriteBatch(IRenderer* renderer, int textureId, size_t startIndex, size_t endIndex);
    
    /**
     * @brief Update performance metrics
     */
    void UpdatePerformanceMetrics();

private:
    TextureManager* m_textureManager;                    ///< Pointer to texture manager
    std::vector<const Sprite*> m_sprites;               ///< Sprites to render
    std::unordered_map<int, std::vector<size_t>> m_textureBatches; ///< Texture ID to sprite indices mapping
    size_t m_maxBatchSize;                              ///< Maximum sprites per batch
    bool m_isInitialized;                               ///< Initialization status
    
    // Performance tracking
    size_t m_totalSpritesRendered;                      ///< Total sprites rendered this frame
    size_t m_totalBatchesRendered;                      ///< Total batches rendered this frame
    double m_lastFrameTime;                             ///< Time of last frame
    double m_averageFrameTime;                          ///< Average frame time
    size_t m_frameCount;                                ///< Frame counter for averaging
};

#endif // __EMSCRIPTEN__
