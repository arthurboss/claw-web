#include "PostProcessor.h"
#include "../IRenderer.h"
#include "TextureManager.h"
#include <iostream>
#include <sstream>
#include <algorithm>

#ifdef __EMSCRIPTEN__

PostProcessor::PostProcessor(TextureManager* textureManager)
    : m_textureManager(textureManager)
    , m_isInitialized(false)
    , m_sourceFramebuffer(0)
    , m_targetFramebuffer(0)
    , m_tempTexture(0)
    , m_totalEffectsApplied(0)
    , m_lastFrameTime(0.0)
    , m_averageFrameTime(0.0)
    , m_frameCount(0)
{
}

PostProcessor::~PostProcessor() {
    Shutdown();
}

bool PostProcessor::Initialize() {
    if (!m_textureManager) {
        std::cerr << "PostProcessor: No texture manager provided" << std::endl;
        return false;
    }

    // Create framebuffers for post-processing
    if (!CreateFramebuffers()) {
        std::cerr << "PostProcessor: Failed to create framebuffers" << std::endl;
        return false;
    }

    m_isInitialized = true;
    m_effects.clear();
    std::cout << "PostProcessor: Initialized successfully" << std::endl;

    return true;
}

void PostProcessor::Shutdown() {
    m_isInitialized = false;
    CleanupFramebuffers();
    m_effects.clear();
}

bool PostProcessor::BeginPass(IRenderer* renderer, int sourceTexture) {
    if (!m_isInitialized || !renderer) {
        return false;
    }

    // For Phase 5, we'll implement the actual post-processing pipeline
    // For now, this is a placeholder that will be enhanced in future phases
    m_totalEffectsApplied = 0;
    
    return true;
}

int PostProcessor::EndPass() {
    if (!m_isInitialized) {
        return 0;
    }

    // For Phase 5, we'll return the processed texture
    // For now, return the source texture (no processing applied)
    UpdatePerformanceMetrics();
    
    return m_tempTexture;
}

void PostProcessor::AddEffect(PostProcessEffect effect, const PostProcessParams& params) {
    if (!m_isInitialized) {
        return;
    }

    // Check if effect already exists
    auto it = std::find_if(m_effects.begin(), m_effects.end(),
        [effect](const std::pair<PostProcessEffect, PostProcessParams>& pair) { return pair.first == effect; });

    if (it != m_effects.end()) {
        // Update existing effect
        it->second = params;
    } else {
        // Add new effect
        m_effects.emplace_back(effect, params);
    }

    std::cout << "PostProcessor: Added effect " << static_cast<int>(effect) << std::endl;
}

void PostProcessor::RemoveEffect(PostProcessEffect effect) {
    if (!m_isInitialized) {
        return;
    }

    auto it = std::find_if(m_effects.begin(), m_effects.end(),
        [effect](const std::pair<PostProcessEffect, PostProcessParams>& pair) { return pair.first == effect; });

    if (it != m_effects.end()) {
        m_effects.erase(it);
        std::cout << "PostProcessor: Removed effect " << static_cast<int>(effect) << std::endl;
    }
}

void PostProcessor::ClearEffects() {
    if (!m_isInitialized) {
        return;
    }

    m_effects.clear();
    std::cout << "PostProcessor: Cleared all effects" << std::endl;
}

void PostProcessor::UpdateEffect(PostProcessEffect effect, const PostProcessParams& params) {
    if (!m_isInitialized) {
        return;
    }

    auto it = std::find_if(m_effects.begin(), m_effects.end(),
        [effect](const std::pair<PostProcessEffect, PostProcessParams>& pair) { return pair.first == effect; });

    if (it != m_effects.end()) {
        it->second = params;
    }
}

void PostProcessor::SetEffectEnabled(PostProcessEffect effect, bool enabled) {
    if (!m_isInitialized) {
        return;
    }

    auto it = std::find_if(m_effects.begin(), m_effects.end(),
        [effect](const std::pair<PostProcessEffect, PostProcessParams>& pair) { return pair.first == effect; });

    if (it != m_effects.end()) {
        it->second.enabled = enabled;
    }
}

bool PostProcessor::IsEffectEnabled(PostProcessEffect effect) const {
    if (!m_isInitialized) {
        return false;
    }

    auto it = std::find_if(m_effects.begin(), m_effects.end(),
        [effect](const std::pair<PostProcessEffect, PostProcessParams>& pair) { return pair.first == effect; });

    return (it != m_effects.end()) && it->second.enabled;
}

bool PostProcessor::ApplyEffect(PostProcessEffect effect, int sourceTexture, int targetTexture, const PostProcessParams& params) {
    if (!m_isInitialized || !params.enabled) {
        return false;
    }

    // For Phase 5, we'll implement actual effect rendering
    // For now, this is a placeholder that will be enhanced in future phases
    switch (effect) {
        case PostProcessEffect::Blur:
            // TODO: Implement blur effect
            break;
        case PostProcessEffect::Bloom:
            // TODO: Implement bloom effect
            break;
        case PostProcessEffect::ColorCorrection:
            // TODO: Implement color correction
            break;
        case PostProcessEffect::Vignette:
            // TODO: Implement vignette effect
            break;
        case PostProcessEffect::ChromaticAberration:
            // TODO: Implement chromatic aberration
            break;
        case PostProcessEffect::MotionBlur:
            // TODO: Implement motion blur
            break;
        default:
            break;
    }

    m_totalEffectsApplied++;
    return true;
}

bool PostProcessor::CreateFramebuffers() {
    // For Phase 5, we'll create actual WebGL/WebGPU framebuffers
    // For now, this is a placeholder that will be enhanced in future phases
    
    // Create temporary texture for post-processing
    // For Phase 5, we'll implement actual texture creation
    // For now, use a placeholder ID
    m_tempTexture = 999; // Placeholder texture ID
    
    return true;
}

void PostProcessor::CleanupFramebuffers() {
    // For Phase 5, we'll clean up actual WebGL/WebGPU resources
    // For now, this is a placeholder that will be enhanced in future phases
    
    if (m_tempTexture > 0) {
        // TODO: Clean up texture
        m_tempTexture = 0;
    }
}

void PostProcessor::UpdatePerformanceMetrics() {
    // Simple frame time calculation
    // In a real implementation, you'd use a proper timer
    m_frameCount++;

    // For now, we'll use a simple counter
    // This will be enhanced in future phases
}

std::string PostProcessor::GetPerformanceStats() const {
    std::stringstream ss;
    ss << "PostProcessor Performance:\n";
    ss << "  Effects Applied: " << m_totalEffectsApplied << "\n";
    ss << "  Active Effects: " << m_effects.size() << "\n";
    ss << "  Initialized: " << (m_isInitialized ? "Yes" : "No") << "\n";
    ss << "  Frame Count: " << m_frameCount << "\n";

    return ss.str();
}

#endif // __EMSCRIPTEN__
