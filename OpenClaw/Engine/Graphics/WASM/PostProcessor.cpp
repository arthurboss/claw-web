#include "PostProcessor.h"
#include "../IRenderer.h"
#include "TextureManager.h"
#include "PostProcessShader.h"
#include "PostProcessFramebuffer.h"
#include <GLES3/gl3.h>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <memory>

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
#ifdef __EMSCRIPTEN__
    , m_screenWidth(800)
    , m_screenHeight(600)
#endif
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

#ifdef __EMSCRIPTEN__
    // Initialize WASM-specific components
    m_shader = std::unique_ptr<PostProcessShader>(new PostProcessShader());
    if (!m_shader->Initialize()) {
        std::cerr << "PostProcessor: Failed to initialize shader manager" << std::endl;
        return false;
    }
    
    m_framebuffer = std::unique_ptr<PostProcessFramebuffer>(new PostProcessFramebuffer());
    if (!m_framebuffer->Initialize(m_screenWidth, m_screenHeight)) {
        std::cerr << "PostProcessor: Failed to initialize framebuffer manager" << std::endl;
        return false;
    }
    
    // Create framebuffers for post-processing
    if (!CreateFramebuffers()) {
        std::cerr << "PostProcessor: Failed to create framebuffers" << std::endl;
        return false;
    }
#endif

    m_isInitialized = true;
    m_effects.clear();

    return true;
}

void PostProcessor::Shutdown() {
    m_isInitialized = false;
    
#ifdef __EMSCRIPTEN__
    if (m_shader) {
        m_shader->Shutdown();
        m_shader.reset();
    }
    if (m_framebuffer) {
        m_framebuffer->Shutdown();
        m_framebuffer.reset();
    }
#endif

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

#ifdef __EMSCRIPTEN__
    if (!m_shader || !m_framebuffer) {
        return false;
    }
    
    // Bind target framebuffer for rendering
    const Framebuffer* fb = m_framebuffer->GetFirstFramebuffer();
    if (fb) {
        m_framebuffer->BindFramebuffer(*fb);
    }
    
    // Clear the framebuffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // Apply the effect based on type
    switch (effect) {
        case PostProcessEffect::Blur:
            if (m_shader->UseShader("blur")) {
                m_shader->SetUniform2f("u_resolution", static_cast<float>(m_screenWidth), static_cast<float>(m_screenHeight));
                m_shader->SetUniform1f("u_intensity", params.intensity);
                m_shader->SetUniform1f("u_radius", params.radius);
                m_shader->BindTexture("u_texture", sourceTexture, 0);
                
                // Render the effect
                m_framebuffer->RenderScreenQuad();
            }
            break;
            
        case PostProcessEffect::Bloom:
            if (m_shader->UseShader("bloom")) {
                m_shader->SetUniform1f("u_intensity", params.intensity);
                m_shader->SetUniform1f("u_threshold", params.threshold);
                m_shader->BindTexture("u_texture", sourceTexture, 0);
                
                // Render the effect
                m_framebuffer->RenderScreenQuad();
            }
            break;
            
        case PostProcessEffect::ColorCorrection:
            if (m_shader->UseShader("color")) {
                m_shader->SetUniform1f("u_brightness", params.brightness);
                m_shader->SetUniform1f("u_contrast", params.contrast);
                m_shader->SetUniform1f("u_saturation", params.saturation);
                m_shader->BindTexture("u_texture", sourceTexture, 0);
                
                // Render the effect
                m_framebuffer->RenderScreenQuad();
            }
            break;
            
        case PostProcessEffect::Vignette:
        case PostProcessEffect::ChromaticAberration:
        case PostProcessEffect::MotionBlur:
            // TODO: Implement these effects in future phases
            break;
            
        default:
            break;
    }
    
    // Restore default framebuffer
    m_framebuffer->BindDefault();
#else
    // Non-WASM builds: placeholder implementation
    switch (effect) {
        case PostProcessEffect::Blur:
        case PostProcessEffect::Bloom:
        case PostProcessEffect::ColorCorrection:
        case PostProcessEffect::Vignette:
        case PostProcessEffect::ChromaticAberration:
        case PostProcessEffect::MotionBlur:
            // TODO: Implement these effects for non-WASM builds
            break;
        default:
            break;
    }
#endif

    m_totalEffectsApplied++;
    return true;
}

bool PostProcessor::CreateFramebuffers() {
#ifdef __EMSCRIPTEN__
    if (!m_framebuffer) {
        return false;
    }
    
    // Create framebuffers for post-processing
    if (!m_framebuffer->CreateFramebuffer(m_screenWidth, m_screenHeight)) {
        std::cerr << "PostProcessor: Failed to create framebuffer" << std::endl;
        return false;
    }
    
    // Create temporary texture for post-processing
    // For now, use a placeholder ID (will be replaced with actual texture)
    m_tempTexture = 999; // Placeholder texture ID
#else
    // Non-WASM builds: placeholder implementation
    m_tempTexture = 999; // Placeholder texture ID
#endif
    
    return true;
}

void PostProcessor::CleanupFramebuffers() {
    // For Phase 5, we'll clean up actual WebGL resources
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
