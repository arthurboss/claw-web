#pragma once

#include <cstdint>

/**
 * @brief Particle emitter configuration for particle systems
 * 
 * This struct defines the parameters for creating and controlling
 * particle emitters in the generic graphics module.
 */
struct ParticleEmitter {
    // ===== Position & Movement =====
    float x = 0.0f;                    // X position
    float y = 0.0f;                    // Y position
    float velocityX = 0.0f;            // X velocity
    float velocityY = 0.0f;            // Y velocity
    float velocityVariation = 0.1f;    // Velocity variation factor
    
    // ===== Particle Properties =====
    float size = 1.0f;                 // Particle size
    float sizeVariation = 0.2f;        // Size variation factor
    float life = 1.0f;                 // Particle lifetime in seconds
    float lifeVariation = 0.2f;        // Life variation factor
    
    // ===== Emission Control =====
    float emissionRate = 10.0f;        // Particles per second
    float emissionRadius = 0.0f;       // Emission area radius
    float emissionAngle = 0.0f;        // Emission direction angle
    float emissionAngleSpread = 360.0f; // Emission angle spread
    
    // ===== Visual Properties =====
    uint32_t color = 0xFFFFFFFF;       // Particle color (RGBA)
    float alpha = 1.0f;                // Alpha transparency
    float alphaVariation = 0.1f;       // Alpha variation factor
    
    // ===== Physics =====
    float gravity = 0.0f;              // Gravity effect
    float friction = 0.98f;            // Air resistance
    float bounce = 0.0f;               // Bounce factor
    
    // ===== Behavior =====
    bool active = true;                 // Whether emitter is active
    bool loop = true;                   // Whether to loop emission
    float burstCount = 0.0f;           // Burst emission count (0 = continuous)
    
    // ===== Advanced =====
    float scaleOverLife = 1.0f;        // Size scaling over lifetime
    float alphaOverLife = 1.0f;        // Alpha scaling over lifetime
    float colorOverLife = 1.0f;        // Color interpolation over lifetime
};
