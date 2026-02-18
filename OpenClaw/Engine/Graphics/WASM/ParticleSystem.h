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
 * @brief Particle properties
 */
struct Particle {
    float x, y;                    ///< Position
    float vx, vy;                  ///< Velocity
    float life;                     ///< Life remaining (0.0 to 1.0)
    float maxLife;                  ///< Maximum life
    float size;                     ///< Size
    float alpha;                    ///< Alpha transparency
    float rotation;                 ///< Rotation in radians
    float rotationSpeed;            ///< Rotation speed
    int textureId;                  ///< Texture ID for this particle
    bool active;                    ///< Whether particle is active
};

/**
 * @brief Particle emitter configuration
 */
struct ParticleEmitter {
    float x, y;                     ///< Emitter position
    float emissionRate;             ///< Particles per second
    float emissionRadius;           ///< Random emission radius
    float minVelocity, maxVelocity; ///< Velocity range
    float minSize, maxSize;         ///< Size range
    float minLife, maxLife;         ///< Life range
    float minRotation, maxRotation; ///< Rotation range
    int textureId;                  ///< Default texture ID
    bool active;                    ///< Whether emitter is active
    bool continuous;                ///< Whether to emit continuously
};

/**
 * @brief ParticleSystem for GPU-accelerated particle rendering
 *
 * This class manages particle emitters and renders particles efficiently
 * using WebGL for optimal performance. It supports multiple
 * emitter types and particle behaviors.
 */
class ParticleSystem {
public:
    /**
     * @brief Constructor
     * @param textureManager Pointer to the texture manager
     * @param maxParticles Maximum number of particles
     */
    explicit ParticleSystem(TextureManager* textureManager, size_t maxParticles = 10000);

    /**
     * @brief Destructor
     */
    ~ParticleSystem();

    /**
     * @brief Initialize the particle system
     * @return True if initialization successful, false otherwise
     */
    bool Initialize();

    /**
     * @brief Shutdown the particle system
     */
    void Shutdown();

    /**
     * @brief Update particle system (call each frame)
     * @param deltaTime Time since last frame in seconds
     */
    void Update(float deltaTime);

    /**
     * @brief Render all particles
     * @param renderer The renderer to use
     */
    void Render(IRenderer* renderer);

    /**
     * @brief Add a particle emitter
     * @param emitter The emitter configuration
     * @return Emitter ID (for later reference)
     */
    int AddEmitter(const ParticleEmitter& emitter);

    /**
     * @brief Remove an emitter
     * @param emitterId The emitter ID to remove
     */
    void RemoveEmitter(int emitterId);

    /**
     * @brief Update emitter configuration
     * @param emitterId The emitter ID to update
     * @param emitter New emitter configuration
     */
    void UpdateEmitter(int emitterId, const ParticleEmitter& emitter);

    /**
     * @brief Enable/disable an emitter
     * @param emitterId The emitter ID to toggle
     * @param active Whether to enable the emitter
     */
    void SetEmitterActive(int emitterId, bool active);

    /**
     * @brief Clear all emitters
     */
    void ClearEmitters();

    /**
     * @brief Get the number of active particles
     * @return The number of active particles
     */
    size_t GetActiveParticleCount() const;

    /**
     * @brief Get the number of emitters
     * @return The number of emitters
     */
    size_t GetEmitterCount() const { return m_emitters.size(); }

    /**
     * @brief Check if the particle system is initialized
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
     * @brief Emit particles from an emitter
     * @param emitter The emitter to use
     * @param deltaTime Time since last frame
     */
    void EmitParticles(const ParticleEmitter& emitter, float deltaTime);

    /**
     * @brief Update individual particle
     * @param particle The particle to update
     * @param deltaTime Time since last frame
     */
    void UpdateParticle(Particle& particle, float deltaTime);

    /**
     * @brief Find inactive particle slot
     * @return Index of inactive particle, or -1 if none available
     */
    int FindInactiveParticle() const;

    /**
     * @brief Create a new particle
     * @param emitter The emitter configuration
     * @return The created particle
     */
    Particle CreateParticle(const ParticleEmitter& emitter);

    /**
     * @brief Update performance metrics
     */
    void UpdatePerformanceMetrics();

private:
    TextureManager* m_textureManager;                    ///< Pointer to texture manager
    std::vector<Particle> m_particles;                   ///< Particle pool
    std::vector<ParticleEmitter> m_emitters;             ///< Particle emitters
    size_t m_maxParticles;                               ///< Maximum number of particles
    bool m_isInitialized;                                ///< Initialization status

    // Performance tracking
    size_t m_totalParticlesEmitted;                      ///< Total particles emitted
    size_t m_totalParticlesRendered;                     ///< Total particles rendered
    double m_lastFrameTime;                              ///< Time of last frame
    double m_averageFrameTime;                           ///< Average frame time
    size_t m_frameCount;                                 ///< Frame counter for averaging
};

#endif // __EMSCRIPTEN__
