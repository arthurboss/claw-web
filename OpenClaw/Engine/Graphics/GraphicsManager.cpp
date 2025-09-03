#include "GraphicsManager.h"
#include "../Logger/Logger.h"
#include <iostream>
#include <memory>
#include <stdexcept>

// Conditional includes based on platform
#ifdef __EMSCRIPTEN__
    // WASM: Use pure WebGL and WebGPU renderers (no SDL dependencies)
    #include "WASM/PureWebGLRenderer.h"
    #include "WASM/PureWebGPURenderer.h"
#else
    // Non-WASM: Use SDL-dependent renderers
    #include "WebGL/WebGLRenderer.h"
    #include "WebGPU/WebGPURenderer.h"
    #include <emscripten.h>
#endif

// Constructor
GraphicsManager::GraphicsManager()
    : currentType(RendererType::None)
    , isInitialized(false)
    , existingSdlRenderer(nullptr)
    , frameTime(0.0f)
    , drawCalls(0)
{
    LOG("GraphicsManager constructor called");
}

// Destructor
GraphicsManager::~GraphicsManager() {
    LOG("GraphicsManager destructor called");
    Shutdown();
}

// Initialize graphics system
bool GraphicsManager::Initialize() {
    LOG("=== GraphicsManager::Initialize() called ===");
    
    return InitializeInternal(nullptr);
}

#if !USE_WASM_RENDERER
// Initialize with existing SDL renderer
bool GraphicsManager::Initialize(SDL_Renderer* existingRenderer) {
    LOG("=== GraphicsManager::Initialize() called with existing renderer ===");
    
    return InitializeInternal(existingRenderer);
}
#endif

#ifdef __EMSCRIPTEN__
// Common initialization logic (WASM)
bool GraphicsManager::InitializeInternal(void* existingRenderer) {
    if (isInitialized) {
        LOG_WARNING("GraphicsManager already initialized");
        return true;
    }
    
    // WASM: No SDL renderer needed
    existingSdlRenderer = existingRenderer;
    
    // Try to initialize WebGPU renderer first (best performance)
    if (TryInitializePureWebGPU()) {
        currentType = RendererType::WebGPU;
        isInitialized = true;
        LogRendererInfo();
        return true;
    }
    
    // Fall back to pure WebGL renderer
    if (TryInitializePureWebGL()) {
        currentType = RendererType::WebGL2; // Treat as WebGL2 for compatibility
        isInitialized = true;
        LogRendererInfo();
        return true;
    }
    
    LOG_ERROR("No graphics renderer available");
    return false;
}
#else
// Common initialization logic (non-WASM)
bool GraphicsManager::InitializeInternal(SDL_Renderer* existingRenderer) {
    if (isInitialized) {
        LOG_WARNING("GraphicsManager already initialized");
        return true;
    }
    
    // Store the existing renderer for WebGL fallback
    existingSdlRenderer = existingRenderer;
    
    // Try to initialize renderers in order of preference
    if (TryInitializeWebGPU()) {
        currentType = RendererType::WebGPU;
        isInitialized = true;
        LogRendererInfo();
        return true;
    }
    
    if (TryInitializeWebGL2()) {
        currentType = RendererType::WebGL2;
        isInitialized = true;
        LogRendererInfo();
        return true;
    }
    
    if (TryInitializeWebGL1()) {
        currentType = RendererType::WebGL1;
        isInitialized = true;
        LogRendererInfo();
        return true;
    }
    
    LOG_ERROR("No graphics renderer available");
    return false;
}
#endif

// Shutdown graphics system
void GraphicsManager::Shutdown() {
    LOG("GraphicsManager::Shutdown() called");
    if (renderer) {
        renderer->Shutdown();
        renderer.reset();
    }
    
    currentType = RendererType::None;
    isInitialized = false;
    
    LOG("Graphics Manager shutdown complete");
}

// Get renderer name
std::string GraphicsManager::GetRendererName() const {
    if (renderer) {
        return renderer->GetRendererName();
    }
    return "None";
}

// Get detailed renderer status
std::string GraphicsManager::GetRendererStatus() const {
    std::string status = "Graphics System Status:\n";
    status += "  Initialized: " + std::string(isInitialized ? "Yes" : "No") + "\n";
    status += "  Renderer Type: ";
    
    switch (currentType) {
        case RendererType::WebGPU:
            status += "WebGPU (Modern, High Performance)";
            break;
        case RendererType::WebGL2:
            status += "WebGL2 (Fallback, Good Performance)";
            break;
        case RendererType::WebGL1:
            status += "WebGL1 (Legacy Fallback)";
            break;
        case RendererType::None:
            status += "None (No Graphics Available)";
            break;
    }
    
    status += "\n  Renderer Name: " + GetRendererName() + "\n";
    
    if (renderer) {
        status += "  WebGPU Support: " + std::string(SupportsFeature(RendererFeature::WebGPU) ? "Yes" : "No") + "\n";
        status += "  WebGL2 Support: " + std::string(SupportsFeature(RendererFeature::WebGL2) ? "Yes" : "No") + "\n";
        status += "  WebGL1 Support: " + std::string(SupportsFeature(RendererFeature::WebGL1) ? "Yes" : "No") + "\n";
        status += "  Shader Support: " + std::string(SupportsFeature(RendererFeature::ShaderSupport) ? "Yes" : "No") + "\n";
        status += "  Texture Compression: " + std::string(SupportsFeature(RendererFeature::TextureCompression) ? "Yes" : "No") + "\n";
    }
    
    return status;
}

// Check feature support
bool GraphicsManager::SupportsFeature(RendererFeature feature) const {
    if (renderer) {
        return renderer->SupportsFeature(feature);
    }
    return false;
}

// Reset performance stats
void GraphicsManager::ResetStats() {
    frameTime = 0.0f;
    drawCalls = 0;
    
    if (renderer) {
        renderer->ResetStats();
    }
}

// Begin frame
void GraphicsManager::BeginFrame() {
    if (renderer && isInitialized) {
        renderer->BeginFrame();
    }
}

// End frame
void GraphicsManager::EndFrame() {
    if (renderer && isInitialized) {
        renderer->EndFrame();
        
        // Update performance stats
        frameTime = renderer->GetFrameTime();
        drawCalls = renderer->GetDrawCalls();
    }
}

#if !USE_WASM_RENDERER
// Try to initialize WebGPU
bool GraphicsManager::TryInitializeWebGPU() {
    LOG("GraphicsManager::TryInitializeWebGPU() called");
    
    // Check if WebGPU is available via JavaScript
    int webgpuAvailable = EM_ASM_INT({
        return Module.detectWebGPU();
    });
    
    if (webgpuAvailable) {
        LOG("WebGPU detected, attempting to initialize renderer");
        
        // Get detailed WebGPU info from JavaScript
        char* webgpuInfo = (char*)EM_ASM_PTR({
            var info = Module.getWebGPUInfo();
            var length = lengthBytesUTF8(info) + 1;
            var buffer = _malloc(length);
            stringToUTF8(info, buffer, length);
            return buffer;
        });
        
        LOG("WebGPU Info: " + std::string(webgpuInfo));
        free(webgpuInfo);
        
        try {
            renderer.reset(new WebGPURenderer());
            if (renderer->Initialize()) {
                LOG("WebGPU renderer initialized successfully");
                LOG("🎉 WebGPU is now active! Better performance expected.");
                return true;
            } else {
                LOG("WebGPU renderer initialization failed");
                renderer.reset();
            }
        } catch (const std::exception& e) {
            LOG_ERROR("Exception during WebGPU initialization: " + std::string(e.what()));
            renderer.reset();
        }
    } else {
        LOG("WebGPU not available in this browser");
    }
    
    return false;
}
#endif

#if !USE_WASM_RENDERER
// Try to initialize WebGL2
bool GraphicsManager::TryInitializeWebGL2() {
    LOG("GraphicsManager::TryInitializeWebGL2() called");
    
    // Check if WebGL2 is available via JavaScript
    int webgl2Available = EM_ASM_INT({
        try {
            var canvas = document.createElement('canvas');
            var gl = canvas.getContext('webgl2');
            return gl ? 1 : 0;
        } catch (e) {
            return 0;
        }
    });
    
    if (webgl2Available) {
        LOG("WebGL2 detected, attempting to initialize renderer");
        try {
            WebGLRenderer* webglRenderer = new WebGLRenderer();
            bool initSuccess = false;
            
            if (existingSdlRenderer) {
                LOG("Using existing SDL renderer for WebGL2");
                initSuccess = webglRenderer->Initialize(existingSdlRenderer);
            } else {
                LOG("Creating new SDL renderer for WebGL2");
                initSuccess = webglRenderer->Initialize();
            }
            
            if (initSuccess) {
                renderer.reset(webglRenderer);
                LOG("WebGL2 renderer initialized successfully");
                return true;
            } else {
                LOG("WebGL2 renderer initialization failed");
                delete webglRenderer;
            }
        } catch (const std::exception& e) {
            LOG_ERROR("Exception during WebGL2 initialization: " + std::string(e.what()));
        }
    } else {
        LOG("WebGL2 not available");
    }
    
    return false;
}
#endif

#if !USE_WASM_RENDERER
// Try to initialize WebGL1
bool GraphicsManager::TryInitializeWebGL1() {
    LOG("GraphicsManager::TryInitializeWebGL1() called");
    
    // Check if WebGL1 is available via JavaScript
    int webgl1Available = EM_ASM_INT({
        try {
            var canvas = document.createElement('canvas');
            var gl = canvas.getContext('webgl') || canvas.getContext('experimental-webgl');
            return gl ? 1 : 0;
        } catch (e) {
            return 0;
        }
    });
    
    if (webgl1Available) {
        LOG("WebGL1 detected, attempting to initialize renderer");
        try {
            WebGLRenderer* webglRenderer = new WebGLRenderer();
            bool initSuccess = false;
            
            if (existingSdlRenderer) {
                LOG("Using existing SDL renderer for WebGL1");
                initSuccess = webglRenderer->Initialize(existingSdlRenderer);
            } else {
                LOG("Creating new SDL renderer for WebGL1");
                initSuccess = webglRenderer->Initialize();
            }
            
            if (initSuccess) {
                renderer.reset(webglRenderer);
                LOG("WebGL1 renderer initialized successfully");
                return true;
            } else {
                LOG("WebGL1 renderer initialization failed");
                delete webglRenderer;
            }
        } catch (const std::exception& e) {
            LOG_ERROR("Exception during WebGL1 initialization: " + std::string(e.what()));
        }
    } else {
        LOG("WebGL1 not available");
    }
    
    return false;
}
#endif

// Log renderer information
void GraphicsManager::LogRendererInfo() const {
    LOG("GraphicsManager::LogRendererInfo() called");
    if (renderer) {
        LOG("Active Renderer: " + renderer->GetRendererName());
        LOG("WebGPU Support: " + std::string(SupportsFeature(RendererFeature::WebGPU) ? "Yes" : "No"));
        LOG("WebGL2 Support: " + std::string(SupportsFeature(RendererFeature::WebGL2) ? "Yes" : "No"));
        LOG("WebGL1 Support: " + std::string(SupportsFeature(RendererFeature::WebGL1) ? "Yes" : "No"));
    } else {
        LOG("No renderer available");
    }
}

#ifdef __EMSCRIPTEN__
// Try to initialize pure WebGPU renderer (WASM only)
bool GraphicsManager::TryInitializePureWebGPU() {
    LOG("GraphicsManager::TryInitializePureWebGPU() called");
    
    LOG("Attempting to initialize pure WebGPU renderer (no SDL dependencies)");
    
    try {
        PureWebGPURenderer* pureWebGPURenderer = new PureWebGPURenderer();
        if (pureWebGPURenderer->Initialize()) {
            renderer.reset(pureWebGPURenderer);
            LOG("Pure WebGPU renderer initialized successfully");
            LOG("🚀 WASM-only pure WebGPU is now active! No SDL dependencies.");
            return true;
        } else {
            LOG("Pure WebGPU renderer initialization failed, will fall back to WebGL");
            delete pureWebGPURenderer;
        }
    } catch (const std::exception& e) {
        LOG_ERROR("Exception during pure WebGPU initialization: " + std::string(e.what()));
    }
    
    return false;
}

// Try to initialize pure WebGL renderer (WASM only)
bool GraphicsManager::TryInitializePureWebGL() {
    LOG("GraphicsManager::TryInitializePureWebGL() called");
    
    LOG("Attempting to initialize pure WebGL renderer (no SDL dependencies)");
    
    try {
        PureWebGLRenderer* pureWebGLRenderer = new PureWebGLRenderer();
        if (pureWebGLRenderer->Initialize()) {
            renderer.reset(pureWebGLRenderer);
            LOG("Pure WebGL renderer initialized successfully");
            LOG("🎉 WASM-only pure WebGL is now active! No SDL dependencies.");
            return true;
        } else {
            LOG("Pure WebGL renderer initialization failed");
            delete pureWebGLRenderer;
        }
    } catch (const std::exception& e) {
        LOG_ERROR("Exception during pure WebGL initialization: " + std::string(e.what()));
    }
    
    return false;
}
#endif
