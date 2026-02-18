#pragma once

#include <memory>
#include <string>
#include "IRenderer.h"

       // Forward declarations
       class SDL_Renderer;

       #ifdef __EMSCRIPTEN__
       // WASM builds: Use pure WebGL renderer
       class PureWebGLRenderer;
       class TextureManager;
       class SpriteRenderer;
       class PostProcessor;
       class ParticleSystem;
       #else
       // Non-WASM builds: Use existing SDL-based renderers
       class WebGLRenderer;
       #endif

enum class RendererType {
    None,
    WebGL,
    WebGL2,
    OpenGL,
    OpenGLES
};

class GraphicsManager {
public:
    GraphicsManager();
    ~GraphicsManager();

    // Initialize graphics system
    bool Initialize();
    void Shutdown();

    // Renderer management
    IRenderer* GetCurrentRenderer() const { return m_currentRenderer.get(); }
    IRenderer* GetRenderer() const { return m_currentRenderer.get(); } // Compatibility alias
    RendererType GetCurrentRendererType() const { return m_currentRendererType; }
    
    // Performance and stats
    void ResetStats();
    std::string GetPerformanceStats() const;
    
    // Feature support
    bool SupportsFeature(RendererFeature feature) const;
    
    // Frame management (for compatibility)
    void BeginFrame();
    void EndFrame();
    
    // Renderer information (for compatibility)
    std::string GetRendererName() const;
    bool IsUsingWebGL() const;
    std::string GetRendererStatus() const;
    float GetFrameTime() const;
    int GetDrawCalls() const;

       #ifdef __EMSCRIPTEN__
           // WASM: Get texture manager
           TextureManager* GetTextureManager() const { return m_textureManager.get(); }

           // WASM: Get sprite renderer
           SpriteRenderer* GetSpriteRenderer() const { return m_spriteRenderer.get(); }

           // WASM: Get post processor
           PostProcessor* GetPostProcessor() const { return m_postProcessor.get(); }

           // WASM: Get particle system
           ParticleSystem* GetParticleSystem() const { return m_particleSystem.get(); }
       #endif

private:
    // Initialize renderer based on platform
    bool InitializeInternal();
    
#ifdef __EMSCRIPTEN__
    // WASM: Try to initialize pure WebGL renderer
    bool TryInitializePureWebGL();
#else
    // Non-WASM: Try to initialize SDL-based renderers
    bool TryInitializeWebGL2();
    bool TryInitializeWebGL1();
#endif

private:
    std::unique_ptr<IRenderer> m_currentRenderer;
    RendererType m_currentRendererType;
    
    // Performance tracking (for compatibility)
    float m_frameTime;
    int m_drawCalls;
    
       #ifdef __EMSCRIPTEN__
           // WASM: Use pure WebGL renderer
           std::unique_ptr<TextureManager> m_textureManager;
           std::unique_ptr<SpriteRenderer> m_spriteRenderer;
           std::unique_ptr<PostProcessor> m_postProcessor;
           std::unique_ptr<ParticleSystem> m_particleSystem;
       #else
           // Non-WASM: Use existing SDL-based renderers (unchanged)
       #endif
};
