#pragma once

#include <vector>
#include <memory>
#include <string>

// Forward declarations
class IRenderer;
class TextureManager;

#ifdef __EMSCRIPTEN__
// WASM builds only - no SDL dependencies

/**
 * @brief Post-processing effect types
 */
enum class PostProcessEffect {
    None,
    Blur,
    Bloom,
    ColorCorrection,
    Vignette,
    ChromaticAberration,
    MotionBlur
};

/**
 * @brief Post-processing effect parameters
 */
struct PostProcessParams {
    float intensity = 1.0f;
    float radius = 5.0f;
    float threshold = 0.8f;
    float saturation = 1.0f;
    float contrast = 1.0f;
    float brightness = 1.0f;
    bool enabled = true;
};

/**
 * @brief PostProcessor for advanced visual effects
 *
 * This class manages post-processing effects including blur, bloom,
 * color correction, and other visual enhancements. It works with
 * both WebGL and WebGPU renderers.
 */
class PostProcessor {
public:
    /**
     * @brief Constructor
     * @param textureManager Pointer to the texture manager
     */
    explicit PostProcessor(TextureManager* textureManager);

    /**
     * @brief Destructor
     */
    ~PostProcessor();

    /**
     * @brief Initialize the post processor
     * @return True if initialization successful, false otherwise
     */
    bool Initialize();

    /**
     * @brief Shutdown the post processor
     */
    void Shutdown();

    /**
     * @brief Begin post-processing pass
     * @param renderer The renderer to use
     * @param sourceTexture Source texture to process
     * @return True if successful, false otherwise
     */
    bool BeginPass(IRenderer* renderer, int sourceTexture);

    /**
     * @brief End post-processing pass
     * @return The processed texture ID
     */
    int EndPass();

    /**
     * @brief Add a post-processing effect
     * @param effect The effect to add
     * @param params Effect parameters
     */
    void AddEffect(PostProcessEffect effect, const PostProcessParams& params = {});

    /**
     * @brief Remove a post-processing effect
     * @param effect The effect to remove
     */
    void RemoveEffect(PostProcessEffect effect);

    /**
     * @brief Clear all effects
     */
    void ClearEffects();

    /**
     * @brief Update effect parameters
     * @param effect The effect to update
     * @param params New parameters
     */
    void UpdateEffect(PostProcessEffect effect, const PostProcessParams& params);

    /**
     * @brief Enable/disable an effect
     * @param effect The effect to toggle
     * @param enabled Whether to enable the effect
     */
    void SetEffectEnabled(PostProcessEffect effect, bool enabled);

    /**
     * @brief Check if an effect is enabled
     * @param effect The effect to check
     * @return True if enabled, false otherwise
     */
    bool IsEffectEnabled(PostProcessEffect effect) const;

    /**
     * @brief Get the number of active effects
     * @return The number of effects
     */
    size_t GetEffectCount() const { return m_effects.size(); }

    /**
     * @brief Check if the post processor is initialized
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
     * @brief Apply a specific effect
     * @param effect The effect to apply
     * @param sourceTexture Source texture ID
     * @param targetTexture Target texture ID
     * @param params Effect parameters
     * @return True if successful, false otherwise
     */
    bool ApplyEffect(PostProcessEffect effect, int sourceTexture, int targetTexture, const PostProcessParams& params);

    /**
     * @brief Create framebuffer for post-processing
     * @return True if successful, false otherwise
     */
    bool CreateFramebuffers();

    /**
     * @brief Clean up framebuffers
     */
    void CleanupFramebuffers();

    /**
     * @brief Update performance metrics
     */
    void UpdatePerformanceMetrics();

private:
    TextureManager* m_textureManager;                    ///< Pointer to texture manager
    std::vector<std::pair<PostProcessEffect, PostProcessParams>> m_effects; ///< Active effects and parameters
    bool m_isInitialized;                               ///< Initialization status

    // Framebuffers for post-processing
    int m_sourceFramebuffer;                            ///< Source framebuffer ID
    int m_targetFramebuffer;                            ///< Target framebuffer ID
    int m_tempTexture;                                  ///< Temporary texture for processing

    // Performance tracking
    size_t m_totalEffectsApplied;                       ///< Total effects applied this frame
    double m_lastFrameTime;                             ///< Time of last frame
    double m_averageFrameTime;                          ///< Average frame time
    size_t m_frameCount;                                ///< Frame counter for averaging
};

#endif // __EMSCRIPTEN__
