#pragma once

#include <cstdint>

// Forward declarations
class IRenderer;

#ifdef __EMSCRIPTEN__
// WASM builds only - no SDL dependencies

/**
 * @brief Sprite class for texture-based rendering in WASM builds
 * 
 * This class represents a textured sprite that can be rendered using
 * the WebGL renderer. It supports UV coordinates, alpha blending,
 * and visibility controls.
 */
class Sprite {
public:
    /**
     * @brief Constructor for creating a sprite with texture and position
     * @param textureId The ID of the texture to use for this sprite
     * @param x X position of the sprite
     * @param y Y position of the sprite
     * @param width Width of the sprite
     * @param height Height of the sprite
     */
    Sprite(int textureId, float x, float y, float width, float height);
    
    /**
     * @brief Destructor
     */
    ~Sprite() = default;
    
    // Rendering methods
    /**
     * @brief Render the sprite using the specified renderer
     * @param renderer The renderer to use for rendering
     */
    void Render(IRenderer* renderer);
    
    // Texture management
    /**
     * @brief Set the texture ID for this sprite
     * @param textureId The new texture ID
     */
    void SetTexture(int textureId);
    
    // Position and size
    /**
     * @brief Set the position of the sprite
     * @param x X position
     * @param y Y position
     */
    void SetPosition(float x, float y);
    
    /**
     * @brief Set the size of the sprite
     * @param width Width
     * @param height Height
     */
    void SetSize(float width, float height);
    
    // UV coordinates for texture mapping
    /**
     * @brief Set the UV coordinates for texture mapping
     * @param u1 Left U coordinate (0.0 to 1.0)
     * @param v1 Top V coordinate (0.0 to 1.0)
     * @param u2 Right U coordinate (0.0 to 1.0)
     * @param v2 Bottom V coordinate (0.0 to 1.0)
     */
    void SetUV(float u1, float v1, float u2, float v2);
    
    // Visual properties
    /**
     * @brief Set the alpha transparency of the sprite
     * @param alpha Alpha value (0.0 = transparent, 1.0 = opaque)
     */
    void SetAlpha(float alpha);
    
    /**
     * @brief Set the visibility of the sprite
     * @param visible Whether the sprite should be visible
     */
    void SetVisible(bool visible);
    
    // Getters
    /**
     * @brief Get the texture ID
     * @return The texture ID
     */
    int GetTextureId() const { return m_textureId; }
    
    /**
     * @brief Get the X position
     * @return The X position
     */
    float GetX() const { return m_x; }
    
    /**
     * @brief Get the Y position
     * @return The Y position
     */
    float GetY() const { return m_y; }
    
    /**
     * @brief Get the width
     * @return The width
     */
    float GetWidth() const { return m_width; }
    
    /**
     * @brief Get the height
     * @return The height
     */
    float GetHeight() const { return m_height; }
    
    /**
     * @brief Get the left U coordinate
     * @return The left U coordinate
     */
    float GetU1() const { return m_u1; }
    
    /**
     * @brief Get the top V coordinate
     * @return The top V coordinate
     */
    float GetV1() const { return m_v1; }
    
    /**
     * @brief Get the right U coordinate
     * @return The right U coordinate
     */
    float GetU2() const { return m_u2; }
    
    /**
     * @brief Get the bottom V coordinate
     * @return The bottom V coordinate
     */
    float GetV2() const { return m_v2; }
    
    /**
     * @brief Get the alpha value
     * @return The alpha value
     */
    float GetAlpha() const { return m_alpha; }
    
    /**
     * @brief Check if the sprite is visible
     * @return True if visible, false otherwise
     */
    bool IsVisible() const { return m_visible; }
    
    /**
     * @brief Check if the sprite has valid UV coordinates
     * @return True if UV coordinates are valid
     */
    bool HasValidUV() const { return m_u1 >= 0.0f && m_v1 >= 0.0f && m_u2 <= 1.0f && m_v2 <= 1.0f; }
    
    /**
     * @brief Check if the sprite has a valid texture
     * @return True if texture ID is valid
     */
    bool HasValidTexture() const { return m_textureId > 0; }

private:
    int m_textureId;           ///< ID of the texture to render
    float m_x, m_y;            ///< Position of the sprite
    float m_width, m_height;   ///< Size of the sprite
    float m_u1, m_v1;          ///< Top-left UV coordinates
    float m_u2, m_v2;          ///< Bottom-right UV coordinates
    float m_alpha;             ///< Alpha transparency (0.0 = transparent, 1.0 = opaque)
    bool m_visible;            ///< Whether the sprite is visible
};

#endif // __EMSCRIPTEN__
