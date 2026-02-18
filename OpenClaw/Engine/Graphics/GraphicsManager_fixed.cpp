#include "GraphicsManager.h"
#include "WebGL/WebGLRenderer.h"
#include "../Logger/Logger.h"
#include <iostream>
#include <memory>

// Constructor
GraphicsManager::GraphicsManager()
    : currentType(RendererType::None)
    , isInitialized(false)
    , frameTime(0.0f)
    , drawCalls(0)
{
}

// Destructor
GraphicsManager::~GraphicsManager() {
    Shutdown();
}

// Initialize graphics system
bool GraphicsManager::Initialize() {
    LOG("Initializing Graphics Manager...");
    
    // Try WebGPU first (priority)
    if (TryInitializeWebGPU()) {
        currentType = RendererType::WebGPU;
        LOG("WebGPU renderer initialized successfully");
        isInitialized = true;
        LogRendererInfo();
        return true;
    }
    
    // Fallback to WebGL2
    if (TryInitializeWebGL2()) {
        currentType = RendererType::WebGL2;
        LOG("WebGL2 renderer initialized (fallback)");
        isInitialized = true;
        LogRendererInfo();
        return true;
    }
    
    // Final fallback to WebGL1
    if (TryInitializeWebGL1()) {
        currentType = RendererType::WebGL1;
        LOG("WebGL1 renderer initialized (fallback)");
        isInitialized = true;
        LogRendererInfo();
        return true;
    }
    
    LOG_ERROR("No graphics renderer available");
    return false;
}

// Shutdown graphics system
void GraphicsManager::Shutdown() {
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

// Try to initialize WebGPU
bool GraphicsManager::TryInitializeWebGPU() {
#ifdef __EMSCRIPTEN__
    // WebGPU detection via JavaScript
    bool webgpuAvailable = EM_ASM_INT({
        if (typeof navigator !== 'undefined' && navigator.gpu) {
            console.log('WebGPU detected in browser');
            return 1;
        }
        console.log('WebGPU not available');
        return 0;
    });
    
    if (webgpuAvailable) {
        // TODO: Create WebGPURenderer instance
        // renderer = std::make_unique<WebGPURenderer>();
        // return renderer->Initialize();
        
        // For now, return false until WebGPU renderer is implemented
        LOG("WebGPU detected but renderer not yet implemented");
        return false;
    }
#endif
    
    LOG("WebGPU not available");
    return false;
}

// Try to initialize WebGL2
bool GraphicsManager::TryInitializeWebGL2() {
#ifdef __EMSCRIPTEN__
    // WebGL2 detection via JavaScript
    bool webgl2Available = EM_ASM_INT({
        const canvas = document.createElement('canvas');
        const gl = canvas.getContext('webgl2');
        if (gl) {
            console.log('WebGL2 available');
            return 1;
        }
        console.log('WebGL2 not available');
        return 0;
    });
    
    if (webgl2Available) {
        renderer = std::make_unique<WebGLRenderer>();
        return renderer->Initialize();
    }
#endif
    
    LOG("WebGL2 not available");
    return false;
}

// Try to initialize WebGL1
bool GraphicsManager::TryInitializeWebGL1() {
#ifdef __EMSCRIPTEN__
    // WebGL1 detection via JavaScript
    bool webgl1Available = EM_ASM_INT({
        const canvas = document.createElement('canvas');
        const gl = canvas.getContext('webgl') || canvas.getContext('experimental-webgl');
        if (gl) {
            console.log('WebGL1 available');
            return 1;
        }
        console.log('WebGL1 not available');
        return 0;
    });
    
    if (webgl1Available) {
        renderer = std::make_unique<WebGLRenderer>();
        return renderer->Initialize();
    }
#endif
    
    LOG("WebGL1 not available");
    return false;
}

// Log renderer information
void GraphicsManager::LogRendererInfo() const {
    if (renderer) {
        LOG("Active Renderer: " + renderer->GetRendererName());
        LOG("WebGPU Support: " + std::string(SupportsFeature(RendererFeature::WebGPU) ? "Yes" : "No"));
        LOG("WebGL2 Support: " + std::string(SupportsFeature(RendererFeature::WebGL2) ? "Yes" : "No"));
        LOG("WebGL1 Support: " + std::string(SupportsFeature(RendererFeature::WebGL1) ? "Yes" : "No"));
    }
}
