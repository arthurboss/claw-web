#include "SpriteRenderer.h"
#include "Sprite.h"
#include "../IRenderer.h"
#include "TextureManager.h"
#include <algorithm>
#include <sstream>
#include <iomanip>

#ifdef __EMSCRIPTEN__

SpriteRenderer::SpriteRenderer(TextureManager* textureManager)
    : m_textureManager(textureManager)
    , m_maxBatchSize(100)
    , m_isInitialized(false)
    , m_totalSpritesRendered(0)
    , m_totalBatchesRendered(0)
    , m_lastFrameTime(0.0)
    , m_averageFrameTime(0.0)
    , m_frameCount(0)
{
}

SpriteRenderer::~SpriteRenderer() {
    Shutdown();
}

bool SpriteRenderer::Initialize() {
    if (!m_textureManager) {
        return false;
    }
    
    m_isInitialized = true;
    m_sprites.clear();
    m_textureBatches.clear();
    
    return true;
}

void SpriteRenderer::Shutdown() {
    m_isInitialized = false;
    ClearSprites();
}

void SpriteRenderer::AddSprite(const Sprite* sprite) {
    if (!sprite || !m_isInitialized) {
        return;
    }
    
    // Check if sprite is already in the list
    auto it = std::find(m_sprites.begin(), m_sprites.end(), sprite);
    if (it == m_sprites.end()) {
        m_sprites.push_back(sprite);
    }
}

void SpriteRenderer::RemoveSprite(const Sprite* sprite) {
    if (!sprite || !m_isInitialized) {
        return;
    }
    
    auto it = std::find(m_sprites.begin(), m_sprites.end(), sprite);
    if (it != m_sprites.end()) {
        m_sprites.erase(it);
    }
}

void SpriteRenderer::ClearSprites() {
    m_sprites.clear();
    m_textureBatches.clear();
}

void SpriteRenderer::RenderSprites(IRenderer* renderer) {
    if (!renderer || !m_isInitialized || m_sprites.empty()) {
        return;
    }
    
    // Sort sprites by texture ID for efficient batching
    SortSpritesByTexture();
    
    // Render sprites in batches
    size_t totalSpritesRendered = 0;
    size_t totalBatchesRendered = 0;
    
    for (const auto& batch : m_textureBatches) {
        int textureId = batch.first;
        const auto& spriteIndices = batch.second;
        
        // Split large batches if needed
        for (size_t i = 0; i < spriteIndices.size(); i += m_maxBatchSize) {
            size_t endIndex = std::min(i + m_maxBatchSize, spriteIndices.size());
            RenderSpriteBatch(renderer, textureId, i, endIndex);
            totalSpritesRendered += (endIndex - i);
            totalBatchesRendered++;
        }
    }
    
    // Update performance metrics
    m_totalSpritesRendered = totalSpritesRendered;
    m_totalBatchesRendered = totalBatchesRendered;
    UpdatePerformanceMetrics();
}

void SpriteRenderer::SetMaxBatchSize(size_t maxBatchSize) {
    if (maxBatchSize > 0) {
        m_maxBatchSize = maxBatchSize;
    }
}

void SpriteRenderer::SortSpritesByTexture() {
    // Clear previous batches
    m_textureBatches.clear();
    
    // Group sprites by texture ID
    for (size_t i = 0; i < m_sprites.size(); ++i) {
        const Sprite* sprite = m_sprites[i];
        if (sprite && sprite->HasValidTexture()) {
            int textureId = sprite->GetTextureId();
            m_textureBatches[textureId].push_back(i);
        }
    }
}

void SpriteRenderer::RenderSpriteBatch(IRenderer* renderer, int textureId, size_t startIndex, size_t endIndex) {
    if (!renderer || !m_textureManager) {
        return;
    }
    
    // For Phase 4, we'll integrate this with the existing renderer methods
    // In the future, we'll add dedicated batch rendering methods to IRenderer
    
    // For now, render each sprite individually
    // This will be optimized in future phases
    for (size_t i = startIndex; i < endIndex; ++i) {
        const auto& batch = m_textureBatches[textureId];
        if (i < batch.size()) {
            size_t spriteIndex = batch[i];
            if (spriteIndex < m_sprites.size()) {
                const Sprite* sprite = m_sprites[spriteIndex];
                if (sprite && sprite->IsVisible()) {
                    // TODO: Implement actual sprite rendering
                    // For now, this is a placeholder for the integration
                }
            }
        }
    }
}

void SpriteRenderer::UpdatePerformanceMetrics() {
    // Simple frame time calculation
    // In a real implementation, you'd use a proper timer
    m_frameCount++;
    
    // For now, we'll use a simple counter
    // This will be enhanced in future phases
}

std::string SpriteRenderer::GetPerformanceStats() const {
    std::stringstream ss;
    ss << "Sprite Renderer Performance:\n";
    ss << "  Sprites Rendered: " << m_totalSpritesRendered << "\n";
    ss << "  Batches Rendered: " << m_totalBatchesRendered << "\n";
    ss << "  Total Sprites: " << m_sprites.size() << "\n";
    ss << "  Max Batch Size: " << m_maxBatchSize << "\n";
    ss << "  Initialized: " << (m_isInitialized ? "Yes" : "No") << "\n";
    
    return ss.str();
}

#endif // __EMSCRIPTEN__
