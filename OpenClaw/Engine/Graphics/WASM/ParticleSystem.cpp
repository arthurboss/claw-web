#include "ParticleSystem.h"
#include "../IRenderer.h"
#include "TextureManager.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <cmath>
#include <cstdlib>

#ifdef __EMSCRIPTEN__

// Helper function for random float generation
float RandomFloat(float min, float max) {
    float random = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
    return min + random * (max - min);
}

// Helper function for random angle generation
float RandomAngle() {
    return RandomFloat(0.0f, 2.0f * M_PI);
}

ParticleSystem::ParticleSystem(TextureManager* textureManager, size_t maxParticles)
    : m_textureManager(textureManager)
    , m_maxParticles(maxParticles)
    , m_isInitialized(false)
    , m_totalParticlesEmitted(0)
    , m_totalParticlesRendered(0)
    , m_lastFrameTime(0.0)
    , m_averageFrameTime(0.0)
    , m_frameCount(0)
{
}

ParticleSystem::~ParticleSystem() {
    Shutdown();
}

bool ParticleSystem::Initialize() {
    if (!m_textureManager) {
        std::cerr << "ParticleSystem: No texture manager provided" << std::endl;
        return false;
    }

    // Initialize particle pool
    m_particles.resize(m_maxParticles);
    for (auto& particle : m_particles) {
        particle.active = false;
        particle.life = 0.0f;
    }

    // Clear emitters
    m_emitters.clear();

    m_isInitialized = true;
    std::cout << "ParticleSystem: Initialized with " << m_maxParticles << " particles" << std::endl;

    return true;
}

void ParticleSystem::Shutdown() {
    m_isInitialized = false;
    m_particles.clear();
    m_emitters.clear();
}

void ParticleSystem::Update(float deltaTime) {
    if (!m_isInitialized) {
        return;
    }

    // Update all emitters
    for (const auto& emitter : m_emitters) {
        if (emitter.active) {
            EmitParticles(emitter, deltaTime);
        }
    }

    // Update all active particles
    for (auto& particle : m_particles) {
        if (particle.active) {
            UpdateParticle(particle, deltaTime);
        }
    }
}

void ParticleSystem::Render(IRenderer* renderer) {
    if (!m_isInitialized || !renderer) {
        return;
    }

    // For Phase 5, we'll implement actual particle rendering
    // For now, this is a placeholder that will be enhanced in future phases
    
    size_t activeCount = 0;
    for (const auto& particle : m_particles) {
        if (particle.active) {
            activeCount++;
            // TODO: Render particle using renderer
        }
    }

    m_totalParticlesRendered = activeCount;
    UpdatePerformanceMetrics();
}

int ParticleSystem::AddEmitter(const ParticleEmitter& emitter) {
    if (!m_isInitialized) {
        return -1;
    }

    m_emitters.push_back(emitter);
    int emitterId = static_cast<int>(m_emitters.size()) - 1;
    
    std::cout << "ParticleSystem: Added emitter " << emitterId << " at (" 
              << emitter.x << ", " << emitter.y << ")" << std::endl;
    
    return emitterId;
}

void ParticleSystem::RemoveEmitter(int emitterId) {
    if (!m_isInitialized || emitterId < 0 || emitterId >= static_cast<int>(m_emitters.size())) {
        return;
    }

    m_emitters.erase(m_emitters.begin() + emitterId);
    std::cout << "ParticleSystem: Removed emitter " << emitterId << std::endl;
}

void ParticleSystem::UpdateEmitter(int emitterId, const ParticleEmitter& emitter) {
    if (!m_isInitialized || emitterId < 0 || emitterId >= static_cast<int>(m_emitters.size())) {
        return;
    }

    m_emitters[emitterId] = emitter;
}

void ParticleSystem::SetEmitterActive(int emitterId, bool active) {
    if (!m_isInitialized || emitterId < 0 || emitterId >= static_cast<int>(m_emitters.size())) {
        return;
    }

    m_emitters[emitterId].active = active;
}

void ParticleSystem::ClearEmitters() {
    if (!m_isInitialized) {
        return;
    }

    m_emitters.clear();
    std::cout << "ParticleSystem: Cleared all emitters" << std::endl;
}

size_t ParticleSystem::GetActiveParticleCount() const {
    if (!m_isInitialized) {
        return 0;
    }

    size_t count = 0;
    for (const auto& particle : m_particles) {
        if (particle.active) {
            count++;
        }
    }
    return count;
}

void ParticleSystem::EmitParticles(const ParticleEmitter& emitter, float deltaTime) {
    if (!emitter.active) {
        return;
    }

    // Calculate how many particles to emit this frame
    float particlesToEmit = emitter.emissionRate * deltaTime;
    int particleCount = static_cast<int>(particlesToEmit);

    // Emit particles
    for (int i = 0; i < particleCount; ++i) {
        int particleIndex = FindInactiveParticle();
        if (particleIndex >= 0) {
            m_particles[particleIndex] = CreateParticle(emitter);
            m_totalParticlesEmitted++;
        }
    }
}

void ParticleSystem::UpdateParticle(Particle& particle, float deltaTime) {
    if (!particle.active) {
        return;
    }

    // Update position
    particle.x += particle.vx * deltaTime;
    particle.y += particle.vy * deltaTime;

    // Update rotation
    particle.rotation += particle.rotationSpeed * deltaTime;

    // Update life
    particle.life -= deltaTime / particle.maxLife;

    // Check if particle should die
    if (particle.life <= 0.0f) {
        particle.active = false;
        return;
    }

    // Update alpha based on life
    particle.alpha = particle.life;
}

int ParticleSystem::FindInactiveParticle() const {
    for (size_t i = 0; i < m_particles.size(); ++i) {
        if (!m_particles[i].active) {
            return static_cast<int>(i);
        }
    }
    return -1; // No inactive particles available
}

Particle ParticleSystem::CreateParticle(const ParticleEmitter& emitter) {
    Particle particle;
    
    // Random position within emission radius
    float angle = RandomAngle();
    float radius = RandomFloat(0.0f, emitter.emissionRadius);
    particle.x = emitter.x + radius * cos(angle);
    particle.y = emitter.y + radius * sin(angle);

    // Random velocity
    float velocity = RandomFloat(emitter.minVelocity, emitter.maxVelocity);
    float velocityAngle = RandomAngle();
    particle.vx = velocity * cos(velocityAngle);
    particle.vy = velocity * sin(velocityAngle);

    // Random size
    particle.size = RandomFloat(emitter.minSize, emitter.maxSize);

    // Random life
    particle.maxLife = RandomFloat(emitter.minLife, emitter.maxLife);
    particle.life = 1.0f; // Start with full life

    // Random rotation
    particle.rotation = RandomFloat(emitter.minRotation, emitter.maxRotation);
    particle.rotationSpeed = RandomFloat(-2.0f, 2.0f); // Random rotation speed

    // Set texture and alpha
    particle.textureId = emitter.textureId;
    particle.alpha = 1.0f;
    particle.active = true;

    return particle;
}

void ParticleSystem::UpdatePerformanceMetrics() {
    // Simple frame time calculation
    // In a real implementation, you'd use a proper timer
    m_frameCount++;

    // For now, we'll use a simple counter
    // This will be enhanced in future phases
}

std::string ParticleSystem::GetPerformanceStats() const {
    std::stringstream ss;
    ss << "ParticleSystem Performance:\n";
    ss << "  Active Particles: " << GetActiveParticleCount() << "\n";
    ss << "  Total Emitted: " << m_totalParticlesEmitted << "\n";
    ss << "  Total Rendered: " << m_totalParticlesRendered << "\n";
    ss << "  Emitters: " << m_emitters.size() << "\n";
    ss << "  Max Particles: " << m_maxParticles << "\n";
    ss << "  Initialized: " << (m_isInitialized ? "Yes" : "No") << "\n";
    ss << "  Frame Count: " << m_frameCount << "\n";

    return ss.str();
}

#endif // __EMSCRIPTEN__
