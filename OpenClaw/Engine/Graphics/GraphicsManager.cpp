#include "GraphicsManager.h"
#include "IRenderer.h"

       #ifdef __EMSCRIPTEN__
       // WASM builds: Use pure WebGL renderer
       #include "WASM/PureWebGLRenderer.h"
       #include "WASM/TextureManager.h"
       #include "WASM/SpriteRenderer.h"
       #include "WASM/PostProcessor.h"
       #include "WASM/ParticleSystem.h"
       #else
       // Non-WASM builds: Use existing SDL-based renderers
       #include "WebGL/WebGLRenderer.h"
       #endif

#include <iostream>
#include <sstream>

GraphicsManager::GraphicsManager()
    : m_currentRendererType(RendererType::None)
    , m_frameTime(0.0f)
    , m_drawCalls(0)
{
}

GraphicsManager::~GraphicsManager() {
    Shutdown();
}

bool GraphicsManager::Initialize() {
    return InitializeInternal();
}

void GraphicsManager::Shutdown() {
    m_currentRenderer.reset();
    m_currentRendererType = RendererType::None;
    
#ifdef __EMSCRIPTEN__
               // WASM: Clean up texture manager and sprite renderer
           if (m_spriteRenderer) {
               m_spriteRenderer->Shutdown();
               m_spriteRenderer.reset();
           }

           if (m_postProcessor) {
               m_postProcessor->Shutdown();
               m_postProcessor.reset();
           }

           if (m_particleSystem) {
               m_particleSystem->Shutdown();
               m_particleSystem.reset();
           }

           if (m_textureManager) {
               m_textureManager->Shutdown();
               m_textureManager.reset();
           }
#endif
}

bool GraphicsManager::InitializeInternal() {
#ifdef __EMSCRIPTEN__
    // WASM: Initialize texture manager first
    m_textureManager = std::unique_ptr<TextureManager>(new TextureManager());
    if (!m_textureManager->Initialize()) {
        std::cerr << "Failed to initialize texture manager" << std::endl;
        return false;
    }
    
               // WASM: Initialize sprite renderer
           m_spriteRenderer = std::unique_ptr<SpriteRenderer>(new SpriteRenderer(m_textureManager.get()));
           if (!m_spriteRenderer->Initialize()) {
               std::cerr << "Failed to initialize sprite renderer" << std::endl;
               return false;
           }

           // WASM: Initialize post processor
           m_postProcessor = std::unique_ptr<PostProcessor>(new PostProcessor(m_textureManager.get()));
           if (!m_postProcessor->Initialize()) {
               std::cerr << "Failed to initialize post processor" << std::endl;
               return false;
           }

           // WASM: Initialize particle system
           m_particleSystem = std::unique_ptr<ParticleSystem>(new ParticleSystem(m_textureManager.get(), 5000));
           if (!m_particleSystem->Initialize()) {
               std::cerr << "Failed to initialize particle system" << std::endl;
               return false;
           }
    
    // WASM: Use WebGL renderer
    if (TryInitializePureWebGL()) {
        m_currentRendererType = RendererType::WebGL;
        std::cout << "Initialized pure WebGL renderer" << std::endl;
    } else {
        std::cerr << "Failed to initialize WebGL renderer" << std::endl;
        return false;
    }
#else
    // Non-WASM: Use existing SDL-based renderers (unchanged)
    if (TryInitializeWebGL2()) {
        m_currentRendererType = RendererType::WebGL2;
        std::cout << "Initialized WebGL2 renderer" << std::endl;
    } else if (TryInitializeWebGL1()) {
        m_currentRendererType = RendererType::WebGL;
        std::cout << "Initialized WebGL renderer" << std::endl;
    } else {
        std::cerr << "Failed to initialize any renderer" << std::endl;
        return false;
    }
#endif
    
    return true;
}

#ifdef __EMSCRIPTEN__
// WASM: Pure WebGL renderer initialization
bool GraphicsManager::TryInitializePureWebGL() {
    auto renderer = std::unique_ptr<PureWebGLRenderer>(new PureWebGLRenderer());
    if (renderer->Initialize()) {
        m_currentRenderer = std::move(renderer);
        return true;
    }
    return false;
}
#else
// Non-WASM: SDL-based renderer initialization (unchanged)
bool GraphicsManager::TryInitializeWebGL2() {
    // Implementation for SDL-based WebGL2 renderer
    return false;
}

bool GraphicsManager::TryInitializeWebGL1() {
    // Implementation for SDL-based WebGL1 renderer
    return false;
}
#endif

void GraphicsManager::ResetStats() {
    m_frameTime = 0.0f;
    m_drawCalls = 0;
    
    if (m_currentRenderer) {
        m_currentRenderer->ResetStats();
    }
}

// Frame management (for compatibility)
void GraphicsManager::BeginFrame() {
    if (m_currentRenderer) {
        m_currentRenderer->BeginFrame();
    }
}

void GraphicsManager::EndFrame() {
    if (m_currentRenderer) {
        m_currentRenderer->EndFrame();
        
        // Update performance metrics (simplified for now)
        m_frameTime = 16.67f; // Assume 60 FPS for now
        m_drawCalls = 0; // Will be updated by renderer
    }
}

// Renderer information (for compatibility)
std::string GraphicsManager::GetRendererName() const {
    if (m_currentRenderer) {
        switch (m_currentRendererType) {
            case RendererType::WebGL: return "Pure WebGL (WASM)";
            case RendererType::WebGL2: return "WebGL2 (SDL)";
            case RendererType::OpenGL: return "OpenGL (SDL)";
            case RendererType::OpenGLES: return "OpenGL ES (SDL)";
            case RendererType::None: return "None";
        }
    }
    return "None";
}

bool GraphicsManager::IsUsingWebGL() const {
    return m_currentRendererType == RendererType::WebGL || m_currentRendererType == RendererType::WebGL2;
}

std::string GraphicsManager::GetRendererStatus() const {
    std::stringstream ss;
    ss << "Graphics System Status:\n";
    ss << "  Initialized: " << (m_currentRenderer ? "Yes" : "No") << "\n";
    ss << "  Renderer Type: " << GetRendererName() << "\n";
    ss << "  WebGL Support: " << (IsUsingWebGL() ? "Yes" : "No") << "\n";

    if (m_currentRenderer) {
        ss << "  Shader Support: " << (SupportsFeature(RendererFeature::ShaderSupport) ? "Yes" : "No") << "\n";
        ss << "  Texture Compression: " << (SupportsFeature(RendererFeature::TextureCompression) ? "Yes" : "No") << "\n";
    }

    return ss.str();
}

float GraphicsManager::GetFrameTime() const {
    return m_frameTime;
}

int GraphicsManager::GetDrawCalls() const {
    return m_drawCalls;
}

std::string GraphicsManager::GetPerformanceStats() const {
    std::stringstream ss;
    ss << "Graphics Manager Performance:\n";
    ss << "  Renderer Type: ";
    
    switch (m_currentRendererType) {
        case RendererType::WebGL2: ss << "WebGL2"; break;
        case RendererType::WebGL: ss << "WebGL"; break;
        case RendererType::OpenGL: ss << "OpenGL"; break;
        case RendererType::OpenGLES: ss << "OpenGL ES"; break;
        case RendererType::None: ss << "None"; break;
    }
    ss << "\n";
    
    // Note: IRenderer doesn't have GetPerformanceStats, so we'll skip that for now
    
#ifdef __EMSCRIPTEN__
    if (m_spriteRenderer) {
        ss << "\n" << m_spriteRenderer->GetPerformanceStats();
    }
#endif
    
    return ss.str();
}

bool GraphicsManager::SupportsFeature(RendererFeature feature) const {
    if (m_currentRenderer) {
        return m_currentRenderer->SupportsFeature(feature);
    }
    return false;
}
