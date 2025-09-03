#pragma once

#include "IRenderer.h"
#include <memory>
#include <string>

// Conditional includes based on platform
#ifdef __EMSCRIPTEN__
    // WASM: Use pure WebGL and WebGPU renderers (no SDL dependencies)
    class PureWebGLRenderer;
    class PureWebGPURenderer;
    class TextureManager;
    #define USE_WASM_RENDERER 1
#else
    // Non-WASM: Use SDL-dependent renderers
    #include <SDL2/SDL.h>
    class WebGPURenderer;
    class WebGLRenderer;
    #define USE_WASM_RENDERER 0
#endif

// Renderer type enumeration
enum class RendererType {
    WebGPU,
    WebGL2,
    WebGL1,
    None
};

// Graphics manager class
class GraphicsManager {
private:
    std::unique_ptr<IRenderer> renderer;
    RendererType currentType;
    bool isInitialized;
    #if USE_WASM_RENDERER
        // WASM: No SDL renderer needed
        void* existingSdlRenderer; // Placeholder for compatibility
        std::unique_ptr<TextureManager> m_textureManager; // WASM texture management
    #else
        // Non-WASM: Store existing SDL renderer for WebGL fallback
        SDL_Renderer* existingSdlRenderer;
    #endif
    
    // Performance tracking
    float frameTime;
    int drawCalls;
    
public:
    
    // Constructor/Destructor
    GraphicsManager();
    ~GraphicsManager();
    
    // Initialization and shutdown
    bool Initialize();
    #if !USE_WASM_RENDERER
        bool Initialize(SDL_Renderer* existingRenderer); // Overload to use existing renderer (non-WASM only)
    #endif
    void Shutdown();
    
    // Renderer access
    IRenderer* GetRenderer() { return renderer.get(); }
    const IRenderer* GetRenderer() const { return renderer.get(); }
    RendererType GetCurrentType() const { return currentType; }
    
    // Status queries
    bool IsInitialized() const { return isInitialized; }
    std::string GetRendererName() const;
    bool SupportsFeature(RendererFeature feature) const;
    
    // Renderer status and information
    std::string GetRendererStatus() const;
    bool IsUsingWebGPU() const { return currentType == RendererType::WebGPU; }
    bool IsUsingWebGL() const { return currentType == RendererType::WebGL2 || currentType == RendererType::WebGL1; }
    
    // Performance queries
    float GetFrameTime() const { return frameTime; }
    int GetDrawCalls() const { return drawCalls; }
    void ResetStats();
    
    // Texture management (WASM only)
    #if USE_WASM_RENDERER
        TextureManager* GetTextureManager() { return m_textureManager.get(); }
        const TextureManager* GetTextureManager() const { return m_textureManager.get(); }
    #endif
    
    // Frame management
    void BeginFrame();
    void EndFrame();
    
private:
    // Renderer detection
    #if USE_WASM_RENDERER
        bool InitializeInternal(void* existingRenderer); // Common initialization logic (WASM)
        bool TryInitializePureWebGPU();
        bool TryInitializePureWebGL();
    #else
        bool InitializeInternal(SDL_Renderer* existingRenderer); // Common initialization logic (non-WASM)
        bool TryInitializeWebGPU();
        bool TryInitializeWebGL2();
        bool TryInitializeWebGL1();
    #endif
    
    // Helper methods
    void LogRendererInfo() const;
};
