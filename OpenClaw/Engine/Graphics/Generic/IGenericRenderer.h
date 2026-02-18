#pragma once

#include <cstdint>
#include <string>
#include <memory>

// Forward declarations
class ITexture;
class ISprite;
class IShader;

/**
 * @brief Generic graphics renderer interface for cross-platform compatibility
 * 
 * This interface provides a platform-agnostic way to access graphics functionality
 * regardless of whether the project is built for WASM, Windows, Linux, or Android.
 * 
 * The actual implementation will be selected at build time:
 * - WASM: Pure WebGL renderer
 * - Windows/Linux: SDL2-based renderers
 * - Android: OpenGL ES renderers
 */
class IGenericRenderer {
public:
    virtual ~IGenericRenderer() = default;

    // ===== Core Initialization & Lifecycle =====
    
    /**
     * @brief Initialize the renderer with the given parameters
     * @param width Screen width
     * @param height Screen height
     * @param title Window title (for desktop platforms)
     * @return true if initialization successful
     */
    virtual bool Initialize(int width, int height, const std::string& title = "") = 0;
    
    /**
     * @brief Shutdown and cleanup the renderer
     */
    virtual void Shutdown() = 0;
    
    /**
     * @brief Check if the renderer is currently initialized
     * @return true if initialized and ready
     */
    virtual bool IsInitialized() const = 0;

    // ===== Platform & Capability Detection =====
    
    /**
     * @brief Get the renderer type/backend being used
     * @return String describing the renderer (e.g., "WebGL", "OpenGL", "Vulkan")
     */
    virtual std::string GetRendererType() const = 0;
    
    /**
     * @brief Get the platform this renderer is running on
     * @return String describing the platform (e.g., "WASM", "Windows", "Linux", "Android")
     */
    virtual std::string GetPlatform() const = 0;
    
    /**
     * @brief Check if a specific graphics feature is supported
     * @param feature Feature name (e.g., "shaders", "textures", "particles")
     * @return true if feature is supported
     */
    virtual bool IsFeatureSupported(const std::string& feature) const = 0;
    
    /**
     * @brief Get the maximum supported texture size
     * @return Maximum texture dimension (width/height)
     */
    virtual int GetMaxTextureSize() const = 0;

    // ===== Rendering & Display =====
    
    /**
     * @brief Begin a new frame for rendering
     */
    virtual void BeginFrame() = 0;
    
    /**
     * @brief End the current frame and present to screen
     */
    virtual void EndFrame() = 0;
    
    /**
     * @brief Clear the screen with the specified color
     * @param r Red component (0.0 - 1.0)
     * @param g Green component (0.0 - 1.0) 
     * @param b Blue component (0.0 - 1.0)
     * @param a Alpha component (0.0 - 1.0)
     */
    virtual void Clear(float r, float g, float b, float a = 1.0f) = 0;
    
    /**
     * @brief Set the viewport dimensions
     * @param x Viewport X position
     * @param y Viewport Y position
     * @param width Viewport width
     * @param height Viewport height
     */
    virtual void SetViewport(int x, int y, int width, int height) = 0;

    // ===== Resource Management =====
    
    /**
     * @brief Create a new texture
     * @param width Texture width
     * @param height Texture height
     * @param data Pixel data (RGBA format)
     * @return Shared pointer to the created texture
     */
    virtual std::shared_ptr<ITexture> CreateTexture(int width, int height, const uint8_t* data) = 0;
    
    /**
     * @brief Load a texture from a file
     * @param filepath Path to the texture file
     * @return Shared pointer to the loaded texture
     */
    virtual std::shared_ptr<ITexture> LoadTexture(const std::string& filepath) = 0;
    
    /**
     * @brief Create a new shader program
     * @param vertexSource Vertex shader source code
     * @param fragmentSource Fragment shader source code
     * @return Shared pointer to the created shader
     */
    virtual std::shared_ptr<IShader> CreateShader(const std::string& vertexSource, const std::string& fragmentSource) = 0;

    // ===== Rendering Operations =====
    
    /**
     * @brief Render a sprite at the specified position
     * @param sprite Sprite to render
     * @param x X position
     * @param y Y position
     * @param scale Scale factor (1.0 = normal size)
     * @param rotation Rotation in radians
     */
    virtual void RenderSprite(const std::shared_ptr<ISprite>& sprite, float x, float y, float scale = 1.0f, float rotation = 0.0f) = 0;
    
    /**
     * @brief Render a textured quad
     * @param texture Texture to use
     * @param x X position
     * @param y Y position
     * @param width Quad width
     * @param height Quad height
     */
    virtual void RenderQuad(const std::shared_ptr<ITexture>& texture, float x, float y, float width, float height) = 0;
    
    /**
     * @brief Render text at the specified position
     * @param text Text string to render
     * @param x X position
     * @param y Y position
     * @param color Text color (RGBA)
     * @param fontSize Font size in pixels
     */
    virtual void RenderText(const std::string& text, float x, float y, uint32_t color = 0xFFFFFFFF, int fontSize = 16) = 0;

    // ===== Performance & Monitoring =====
    
    /**
     * @brief Get current FPS
     * @return Frames per second
     */
    virtual float GetFPS() const = 0;
    
    /**
     * @brief Get memory usage statistics
     * @return String containing memory usage info
     */
    virtual std::string GetMemoryStats() const = 0;
    
    /**
     * @brief Get renderer performance statistics
     * @return String containing performance info
     */
    virtual std::string GetPerformanceStats() const = 0;
    
    /**
     * @brief Enable/disable performance monitoring
     * @param enabled true to enable monitoring
     */
    virtual void SetPerformanceMonitoring(bool enabled) = 0;

    // ===== Advanced Features =====
    
    /**
     * @brief Enable/disable post-processing effects
     * @param effect Effect name (e.g., "blur", "bloom", "color_correction")
     * @param enabled true to enable the effect
     */
    virtual void SetPostProcessEffect(const std::string& effect, bool enabled) = 0;
    
    /**
     * @brief Set post-processing effect parameter
     * @param effect Effect name
     * @param param Parameter name
     * @param value Parameter value
     */
    virtual void SetPostProcessParameter(const std::string& effect, const std::string& param, float value) = 0;
    
    /**
     * @brief Create a particle system
     * @param maxParticles Maximum number of particles
     * @return Shared pointer to the particle system
     */
    virtual std::shared_ptr<class IParticleSystem> CreateParticleSystem(size_t maxParticles) = 0;
};

// ===== Supporting Interface Classes =====

/**
 * @brief Generic texture interface
 */
class ITexture {
public:
    virtual ~ITexture() = default;
    virtual int GetWidth() const = 0;
    virtual int GetHeight() const = 0;
    virtual bool IsValid() const = 0;
    virtual void Bind(int unit = 0) = 0;
    virtual void Unbind() = 0;
};

/**
 * @brief Generic sprite interface
 */
class ISprite {
public:
    virtual ~ISprite() = default;
    virtual void SetTexture(const std::shared_ptr<ITexture>& texture) = 0;
    virtual std::shared_ptr<ITexture> GetTexture() const = 0;
    virtual void SetUV(float u1, float v1, float u2, float v2) = 0;
    virtual void GetUV(float& u1, float& v1, float& u2, float& v2) const = 0;
};

/**
 * @brief Generic shader interface
 */
class IShader {
public:
    virtual ~IShader() = default;
    virtual bool IsValid() const = 0;
    virtual void Use() = 0;
    virtual void SetUniform(const std::string& name, float value) = 0;
    virtual void SetUniform(const std::string& name, int value) = 0;
    virtual void SetUniform(const std::string& name, float x, float y) = 0;
};

/**
 * @brief Generic particle system interface
 */
class IParticleSystem {
public:
    virtual ~IParticleSystem() = default;
    virtual bool Initialize() = 0;
    virtual void Shutdown() = 0;
    virtual void Update(float deltaTime) = 0;
    virtual void Render() = 0;
    virtual void AddEmitter(const class ParticleEmitter& emitter) = 0;
    virtual void RemoveEmitter(size_t index) = 0;
    virtual size_t GetActiveParticleCount() const = 0;
};
