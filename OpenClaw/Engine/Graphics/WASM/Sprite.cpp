#include "Sprite.h"
#include "../IRenderer.h"
#include <algorithm>

#ifdef __EMSCRIPTEN__

Sprite::Sprite(int textureId, float x, float y, float width, float height)
    : m_textureId(textureId)
    , m_x(x)
    , m_y(y)
    , m_width(width)
    , m_height(height)
    , m_u1(0.0f)
    , m_v1(0.0f)
    , m_u2(1.0f)
    , m_v2(1.0f)
    , m_alpha(1.0f)
    , m_visible(true)
{
    // Validate input parameters
    if (width <= 0.0f || height <= 0.0f) {
        // Set default size if invalid
        m_width = 1.0f;
        m_height = 1.0f;
    }
    
    if (textureId <= 0) {
        // Set default texture ID if invalid
        m_textureId = 1; // Default texture
    }
}

void Sprite::Render(IRenderer* renderer) {
    if (!renderer || !m_visible || !HasValidTexture()) {
        return;
    }
    
    // For now, we'll use the existing renderer methods
    // In the future, we'll add a dedicated RenderSprite method to IRenderer
    // For Phase 4, we'll integrate this with the existing menu rendering system
}

void Sprite::SetTexture(int textureId) {
    if (textureId > 0) {
        m_textureId = textureId;
    }
}

void Sprite::SetPosition(float x, float y) {
    m_x = x;
    m_y = y;
}

void Sprite::SetSize(float width, float height) {
    if (width > 0.0f && height > 0.0f) {
        m_width = width;
        m_height = height;
    }
}

void Sprite::SetUV(float u1, float v1, float u2, float v2) {
    // Validate UV coordinates are within [0,1] range
    m_u1 = std::max(0.0f, std::min(1.0f, u1));
    m_v1 = std::max(0.0f, std::min(1.0f, v1));
    m_u2 = std::max(0.0f, std::min(1.0f, u2));
    m_v2 = std::max(0.0f, std::min(1.0f, v2));
    
    // Ensure u2 > u1 and v2 > v1
    if (m_u2 <= m_u1) {
        m_u2 = m_u1 + 0.001f; // Small offset to prevent zero width
    }
    if (m_v2 <= m_v1) {
        m_v2 = m_v1 + 0.001f; // Small offset to prevent zero height
    }
}

void Sprite::SetAlpha(float alpha) {
    // Clamp alpha to [0,1] range
    m_alpha = std::max(0.0f, std::min(1.0f, alpha));
}

void Sprite::SetVisible(bool visible) {
    m_visible = visible;
}

#endif // __EMSCRIPTEN__
