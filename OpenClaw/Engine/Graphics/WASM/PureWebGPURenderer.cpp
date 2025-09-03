#include "PureWebGPURenderer.h"
#include "../Data/MenuBackgroundData.h"
#include "../Data/MenuItemData.h"
#include "../Data/MenuTextData.h"

// Emscripten includes for JavaScript interop
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

PureWebGPURenderer::PureWebGPURenderer()
    : m_isInitialized(false)
    , m_width(800)
    , m_height(600)
    , m_webgpuAdapterId(-1)
    , m_webgpuDeviceId(-1)
    , m_webgpuQueueId(-1)
    , m_webgpuPipelineId(-1)
    , m_webgpuBufferId(-1)
    , m_frameTime(0.0f)
    , m_drawCalls(0)
    , m_fps(0)
    , m_lastFrameTime(0.0f)
{
}

PureWebGPURenderer::~PureWebGPURenderer() {
    Shutdown();
}

bool PureWebGPURenderer::Initialize() {
    if (m_isInitialized) {
        return true;
    }

#ifdef __EMSCRIPTEN__
    // Check if WebGPU is available via JavaScript bridge
    if (!IsWebGPUAvailable()) {
        EM_ASM({ console.log("PureWebGPURenderer: WebGPU not available, cannot initialize."); });
        return false;
    }

    // Initialize WebGPU context via JavaScript bridge
    EM_ASM({
        if (!window.webGPUBridge) {
            window.webGPUBridge = new WebGPUBridge();
        }
        window.webGPUBridge.initializeWebGPU();
    });

    // Get WebGPU object IDs from JavaScript
    m_webgpuAdapterId = EM_ASM_INT({ return window.webGPUBridge.getAdapterId(); });
    m_webgpuDeviceId = EM_ASM_INT({ return window.webGPUBridge.getDeviceId(); });
    m_webgpuQueueId = EM_ASM_INT({ return window.webGPUBridge.getQueueId(); });
    m_webgpuPipelineId = EM_ASM_INT({ return window.webGPUBridge.getPipelineId(); });
    m_webgpuBufferId = EM_ASM_INT({ return window.webGPUBridge.getBufferId(); });

    if (m_webgpuDeviceId != -1 && m_webgpuPipelineId != -1) {
        m_isInitialized = true;
        EM_ASM({ console.log("PureWebGPURenderer initialized via WebGPU bridge."); });
        return true;
    } else {
        EM_ASM({ console.error("PureWebGPURenderer failed to initialize via WebGPU bridge."); });
        return false;
    }
#else
    // Non-WASM builds should not use this renderer
    return false;
#endif
}

void PureWebGPURenderer::Shutdown() {
    if (!m_isInitialized) {
        return;
    }

#ifdef __EMSCRIPTEN__
    EM_ASM({
        if (window.webGPUBridge) {
            window.webGPUBridge.shutdownWebGPU();
            delete window.webGPUBridge;
            window.webGPUBridge = null;
        }
    });
#endif
    m_isInitialized = false;
}

void PureWebGPURenderer::BeginFrame() {
    if (!m_isInitialized) {
        return;
    }
#ifdef __EMSCRIPTEN__
    EM_ASM({
        if (window.webGPUBridge) {
            window.webGPUBridge.beginFrame();
        }
    });
#endif
    m_drawCalls = 0; // Reset draw calls per frame
}

void PureWebGPURenderer::EndFrame() {
    if (!m_isInitialized) {
        return;
    }
#ifdef __EMSCRIPTEN__
    EM_ASM({
        if (window.webGPUBridge) {
            window.webGPUBridge.endFrame();
        }
    });
#endif
    UpdatePerformanceMetrics();
}

void PureWebGPURenderer::ResetStats() {
    m_frameTime = 0.0f;
    m_drawCalls = 0;
    m_fps = 0;
    m_lastFrameTime = 0.0f;
}

bool PureWebGPURenderer::SupportsFeature(RendererFeature feature) {
    switch (feature) {
        case RendererFeature::WebGPU: return true;
        case RendererFeature::WebGL2: return false; // This is WebGPU renderer
        case RendererFeature::WebGL1: return false; // This is WebGPU renderer
        case RendererFeature::ShaderSupport: return true;
        case RendererFeature::TextureCompression: return true; // WebGPU supports this
        default: return false;
    }
}

void PureWebGPURenderer::SetViewport(int x, int y, int width, int height) {
    m_width = width;
    m_height = height;
    
#ifdef __EMSCRIPTEN__
    EM_ASM_({
        if (window.webGPUBridge) {
            window.webGPUBridge.setViewport($0, $1, $2, $3);
        }
    }, x, y, width, height);
#endif
}

void PureWebGPURenderer::Clear(float r, float g, float b, float a) {
#ifdef __EMSCRIPTEN__
    EM_ASM_({
        if (window.webGPUBridge) {
            window.webGPUBridge.clear($0, $1, $2, $3);
        }
    }, r, g, b, a);
#endif
}

void PureWebGPURenderer::Present() {
#ifdef __EMSCRIPTEN__
    EM_ASM({
        if (window.webGPUBridge) {
            window.webGPUBridge.present();
        }
    });
#endif
}

bool PureWebGPURenderer::IsWebGPUAvailable() const {
#ifdef __EMSCRIPTEN__
    return EM_ASM_INT({ 
        return window.webGPUBridge && window.webGPUBridge.isWebGPUAvailable() ? 1 : 0; 
    });
#else
    return false;
#endif
}

void PureWebGPURenderer::UpdatePerformanceMetrics() {
#ifdef __EMSCRIPTEN__
    float currentTime = EM_ASM_DOUBLE({ return performance.now() / 1000.0; });
    if (m_lastFrameTime > 0.0f) {
        m_frameTime = currentTime - m_lastFrameTime;
        if (m_frameTime > 0.0f) {
            m_fps = static_cast<int>(1.0f / m_frameTime);
        }
    }
    m_lastFrameTime = currentTime;
#endif
}

void PureWebGPURenderer::RenderQuad(float x, float y, float width, float height, float r, float g, float b, float a) {
#ifdef __EMSCRIPTEN__
    EM_ASM_({
        if (window.webGPUBridge) {
            window.webGPUBridge.renderQuad($0, $1, $2, $3, $4, $5, $6, $7);
        }
    }, x, y, width, height, r, g, b, a);
#endif
}

void PureWebGPURenderer::RenderMenuBackground(const MenuBackgroundData& data) {
    if (!m_isInitialized) {
        return;
    }
    // Render background using WebGPU
    float r = 0.2f, g = 0.2f, b = 0.2f, a = 1.0f; // Dark gray background
    RenderQuad(data.x, data.y, data.width, data.height, r, g, b, a);
    m_drawCalls++;
}

void PureWebGPURenderer::RenderMenuItem(const MenuItemData& data) {
    if (!m_isInitialized) {
        return;
    }
    // Render menu item using WebGPU
    float r, g, b, a = 1.0f;
    switch (data.state) {
        case GraphicsMenuItemState::Inactive: r = 0.5f; g = 0.5f; b = 0.5f; break; // Gray
        case GraphicsMenuItemState::Active:   r = 0.0f; g = 0.7f; b = 0.0f; break; // Green
        case GraphicsMenuItemState::Focused:  r = 0.0f; g = 0.8f; b = 0.0f; break; // Bright Green
        case GraphicsMenuItemState::Pressed:  r = 0.0f; g = 0.5f; b = 0.0f; break; // Dark Green
        default: r = 0.5f; g = 0.5f; b = 0.5f; break;
    }
    RenderQuad(data.x, data.y, data.width, data.height, r, g, b, a);
    m_drawCalls++;
}

void PureWebGPURenderer::RenderMenuText(const MenuTextData& data) {
    if (!m_isInitialized) {
        return;
    }
    
    // Render text using WebGPU
    float r = data.r, g = data.g, b = data.b, a = data.a;
    
    // Calculate approximate text dimensions based on font size
    float textWidth = data.text.length() * (data.fontSize * 0.6f); // Rough estimate
    float textHeight = data.fontSize * 1.2f; // Rough estimate
    
    // Render the text area as a quad
    RenderQuad(data.x, data.y, textWidth, textHeight, r, g, b, a);
    
    m_drawCalls++;
}
