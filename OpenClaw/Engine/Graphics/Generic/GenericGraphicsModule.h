#pragma once

#include "IGenericRenderer.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <sstream>

// Forward declarations for platform-specific implementations
class PureWebGLRenderer;

/**
 * @brief Generic Graphics Module - Cross-platform graphics solution
 * 
 * This module automatically selects the appropriate graphics renderer based on the build target:
 * - WASM: Pure WebGL/WebGPU renderers (no SDL dependencies)
 * - Windows/Linux: SDL2-based renderers
 * - Android: OpenGL ES renderers
 * 
 * Usage:
 * ```cpp
 * GenericGraphicsModule graphics;
 * if (graphics.Initialize(800, 600, "My Game")) {
 *     graphics.BeginFrame();
 *     graphics.Clear(0.1f, 0.1f, 0.1f);
 *     graphics.RenderSprite(mySprite, 100, 100);
 *     graphics.EndFrame();
 * }
 * ```
 */
class GenericGraphicsModule {
public:
    /**
     * @brief Constructor - automatically detects platform and capabilities
     */
    GenericGraphicsModule();
    
    /**
     * @brief Destructor - automatically cleans up resources
     */
    ~GenericGraphicsModule();

    // ===== Core Module Interface =====
    
    /**
     * @brief Initialize the graphics module
     * @param width Screen width
     * @param height Screen height
     * @param title Window title (for desktop platforms)
     * @return true if initialization successful
     */
    bool Initialize(int width, int height, const std::string& title = "");
    
    /**
     * @brief Shutdown and cleanup the graphics module
     */
    void Shutdown();
    
    /**
     * @brief Check if the module is currently initialized
     * @return true if initialized and ready
     */
    bool IsInitialized() const;

    // ===== Platform & Renderer Information =====
    
    /**
     * @brief Get the current platform this module is running on
     * @return Platform string (e.g., "WASM", "Windows", "Linux", "Android")
     */
    std::string GetPlatform() const;
    
    /**
     * @brief Get the current renderer type being used
     * @return Renderer type string (e.g., "WebGL", "WebGPU", "OpenGL", "Vulkan")
     */
    std::string GetRendererType() const;
    
    /**
     * @brief Get the renderer that was automatically selected
     * @return Pointer to the generic renderer interface
     */
    IGenericRenderer* GetRenderer() const;
    
    /**
     * @brief Get a list of available renderers for this platform
     * @return Vector of available renderer names
     */
    std::vector<std::string> GetAvailableRenderers() const;

    // ===== Feature Detection & Capabilities =====
    
    /**
     * @brief Check if a specific graphics feature is supported
     * @param feature Feature name (e.g., "shaders", "textures", "particles", "post_processing")
     * @return true if feature is supported
     */
    bool IsFeatureSupported(const std::string& feature) const;
    
    /**
     * @brief Get the maximum supported texture size
     * @return Maximum texture dimension (width/height)
     */
    int GetMaxTextureSize() const;
    
    /**
     * @brief Get detailed capability information
     * @return String containing detailed capability info
     */
    std::string GetCapabilityInfo() const;

    // ===== Configuration & Customization =====
    
    /**
     * @brief Set preferred renderer type (if multiple are available)
     * @param rendererType Preferred renderer (e.g., "WebGPU", "WebGL", "OpenGL")
     * @return true if renderer type was set successfully
     */
    bool SetPreferredRenderer(const std::string& rendererType);
    
    /**
     * @brief Enable/disable specific features
     * @param feature Feature name
     * @param enabled true to enable, false to disable
     */
    void SetFeatureEnabled(const std::string& feature, bool enabled);
    
    /**
     * @brief Get module configuration
     * @return String containing current configuration
     */
    std::string GetConfiguration() const;

    // ===== Performance & Monitoring =====
    
    /**
     * @brief Enable/disable performance monitoring
     * @param enabled true to enable monitoring
     */
    void SetPerformanceMonitoring(bool enabled);
    
    /**
     * @brief Get current FPS
     * @return Frames per second
     */
    float GetFPS() const;
    
    /**
     * @brief Get memory usage statistics
     * @return String containing memory usage info
     */
    std::string GetMemoryStats() const;
    
    /**
     * @brief Get performance statistics
     * @return String containing performance info
     */
    std::string GetPerformanceStats() const;

    // ===== Convenience Methods (Delegate to current renderer) =====
    
    /**
     * @brief Begin a new frame for rendering
     */
    void BeginFrame();
    
    /**
     * @brief End the current frame and present to screen
     */
    void EndFrame();
    
    /**
     * @brief Clear the screen with the specified color
     * @param r Red component (0.0 - 1.0)
     * @param g Green component (0.0 - 1.0)
     * @param b Blue component (0.0 - 1.0)
     * @param a Alpha component (0.0 - 1.0)
     */
    void Clear(float r, float g, float b, float a = 1.0f);
    
    /**
     * @brief Set the viewport dimensions
     * @param x Viewport X position
     * @param y Viewport Y position
     * @param width Viewport width
     * @param height Viewport height
     */
    void SetViewport(int x, int y, int width, int height);

    // ===== Resource Management =====
    
    /**
     * @brief Create a new texture
     * @param width Texture width
     * @param height Texture height
     * @param data Pixel data (RGBA format)
     * @return Shared pointer to the created texture
     */
    std::shared_ptr<ITexture> CreateTexture(int width, int height, const uint8_t* data);
    
    /**
     * @brief Load a texture from a file
     * @param filepath Path to the texture file
     * @return Shared pointer to the loaded texture
     */
    std::shared_ptr<ITexture> LoadTexture(const std::string& filepath);
    
    /**
     * @brief Create a new shader program
     * @param vertexSource Vertex shader source code
     * @param fragmentSource Fragment shader source code
     * @return Shared pointer to the created shader
     */
    std::shared_ptr<IShader> CreateShader(const std::string& vertexSource, const std::string& fragmentSource);

    // ===== Rendering Operations =====
    
    /**
     * @brief Render a sprite at the specified position
     * @param sprite Sprite to render
     * @param x X position
     * @param y Y position
     * @param scale Scale factor (1.0 = normal size)
     * @param rotation Rotation in radians
     */
    void RenderSprite(const std::shared_ptr<ISprite>& sprite, float x, float y, float scale = 1.0f, float rotation = 0.0f);
    
    /**
     * @brief Render a textured quad
     * @param texture Texture to use
     * @param x X position
     * @param y Y position
     * @param width Quad width
     * @param height Quad height
     */
    void RenderQuad(const std::shared_ptr<ITexture>& texture, float x, float y, float width, float height);
    
    /**
     * @brief Render text at the specified position
     * @param text Text string to render
     * @param x X position
     * @param y Y position
     * @param color Text color (RGBA)
     * @param fontSize Font size in pixels
     */
    void RenderText(const std::string& text, float x, float y, uint32_t color = 0xFFFFFFFF, int fontSize = 16);

    // ===== Advanced Features =====
    
    /**
     * @brief Enable/disable post-processing effects
     * @param effect Effect name (e.g., "blur", "bloom", "color_correction")
     * @param enabled true to enable the effect
     */
    void SetPostProcessEffect(const std::string& effect, bool enabled);
    
    /**
     * @brief Set post-processing effect parameter
     * @param effect Effect name
     * @param param Parameter name
     * @param value Parameter value
     */
    void SetPostProcessParameter(const std::string& effect, const std::string& param, float value);
    
    /**
     * @brief Create a particle system
     * @param maxParticles Maximum number of particles
     * @return Shared pointer to the particle system
     */
    std::shared_ptr<IParticleSystem> CreateParticleSystem(size_t maxParticles);

private:
    // ===== Private Implementation Details =====
    
    /**
     * @brief Detect the current platform
     * @return Platform string
     */
    std::string DetectPlatform() const;
    
    /**
     * @brief Get available renderers for the current platform
     * @return Vector of available renderer names
     */
    std::vector<std::string> GetPlatformRenderers() const;
    
    /**
     * @brief Create the appropriate renderer for the current platform
     * @param rendererType Type of renderer to create
     * @return Pointer to the created renderer
     */
    std::unique_ptr<IGenericRenderer> CreateRenderer(const std::string& rendererType);
    
    /**
     * @brief Initialize WASM-specific renderers
     * @param width Screen width
     * @param height Screen height
     * @return true if initialization successful
     */
    bool InitializeWASMRenderers(int width, int height);
    
    /**
     * @brief Initialize desktop-specific renderers
     * @param width Screen width
     * @param height Screen height
     * @param title Window title
     * @return true if initialization successful
     */
    bool InitializeDesktopRenderers(int width, int height, const std::string& title);
    
    /**
     * @brief Initialize mobile-specific renderers
     * @param width Screen width
     * @param height Screen height
     * @return true if initialization successful
     */
    bool InitializeMobileRenderers(int width, int height);

    // ===== Member Variables =====
    
    std::string m_platform;                           // Current platform
    std::string m_rendererType;                       // Current renderer type
    std::string m_preferredRenderer;                  // User's preferred renderer
    std::unique_ptr<IGenericRenderer> m_renderer;     // Current renderer instance
    std::vector<std::string> m_availableRenderers;    // Available renderers for this platform
    std::unordered_map<std::string, bool> m_features; // Feature support map
    bool m_initialized;                               // Initialization state
    bool m_performanceMonitoring;                     // Performance monitoring state
    
    // Platform-specific renderer instances (for fallback)
    std::unique_ptr<PureWebGLRenderer> m_wasmWebGLRenderer;
};
