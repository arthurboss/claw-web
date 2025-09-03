#include "PureWebGLRenderer.h"
#include "../Data/MenuBackgroundData.h"
#include "../Data/MenuItemData.h"
#include "../Data/MenuTextData.h"

// Emscripten includes for JavaScript interop
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

PureWebGLRenderer::PureWebGLRenderer()
    : m_isInitialized(false)
    , m_width(800)
    , m_height(600)
    , m_webglContextId(-1)
    , m_vertexShaderId(-1)
    , m_fragmentShaderId(-1)
    , m_programId(-1)
    , m_vertexBufferId(-1)
    , m_indexBufferId(-1)
    , m_frameTime(0.0f)
    , m_drawCalls(0)
{
}

PureWebGLRenderer::~PureWebGLRenderer() {
    Shutdown();
}

bool PureWebGLRenderer::Initialize() {
    if (m_isInitialized) {
        return true;
    }
    
    // Initialize WebGL context via JavaScript bridge
    if (!InitializeWebGL()) {
        return false;
    }
    
    // Create basic shaders
    if (!CreateShaders()) {
        return false;
    }
    
    // Create basic buffers
    if (!CreateBuffers()) {
        return false;
    }
    
    m_isInitialized = true;
    return true;
}

void PureWebGLRenderer::Shutdown() {
    if (!m_isInitialized) {
        return;
    }
    
    // Clean up WebGL objects via JavaScript bridge
#ifdef __EMSCRIPTEN__
    EM_ASM({
        if (typeof window.webglBridge !== 'undefined' && window.webglBridge.cleanup) {
            window.webglBridge.cleanup();
        }
    });
#endif
    
    m_isInitialized = false;
    m_webglContextId = -1;
    m_vertexShaderId = -1;
    m_fragmentShaderId = -1;
    m_programId = -1;
    m_vertexBufferId = -1;
    m_indexBufferId = -1;
}

void PureWebGLRenderer::BeginFrame() {
    if (!m_isInitialized) {
        return;
    }
    
    // Clear the screen with a default color
    Clear(0.0f, 0.0f, 0.0f, 1.0f);
    
    // Reset frame statistics
    m_drawCalls = 0;
}

void PureWebGLRenderer::EndFrame() {
    if (!m_isInitialized) {
        return;
    }
    
    // Present the frame
    Present();
}

void PureWebGLRenderer::RenderMenuBackground(const MenuBackgroundData& data) {
    if (!m_isInitialized) {
        return;
    }
    
    // For now, render a simple colored background
    // This will be enhanced to load and render actual textures
    float r = 0.2f, g = 0.3f, b = 0.5f, a = 1.0f;
    
    // Render a full-screen quad with the background color
    RenderQuad(0, 0, m_width, m_height, r, g, b, a);
    
    m_drawCalls++;
}

void PureWebGLRenderer::RenderMenuItem(const MenuItemData& data) {
    if (!m_isInitialized) {
        return;
    }
    
    // For now, render a simple colored rectangle for menu items
    // This will be enhanced to load and render actual textures
    float r = 0.8f, g = 0.8f, b = 0.8f, a = 1.0f;
    
    // Render the menu item as a quad
    RenderQuad(data.x, data.y, data.width, data.height, r, g, b, a);
    
    m_drawCalls++;
}

void PureWebGLRenderer::RenderMenuText(const MenuTextData& data) {
    if (!m_isInitialized) {
        return;
    }
    
    // For now, render a simple colored rectangle for text
    // This will be enhanced to render actual text using WebGL
    float r = data.r, g = data.g, b = data.b, a = data.a;
    
    // Calculate approximate text dimensions based on font size
    // This is a rough estimate - will be improved with actual text rendering
    float width = data.text.length() * data.fontSize * 0.6f;  // Approximate character width
    float height = data.fontSize * 1.2f;  // Approximate line height
    
    // Render the text area as a quad
    RenderQuad(data.x, data.y, width, height, r, g, b, a);
    
    m_drawCalls++;
}

void PureWebGLRenderer::SetViewport(int x, int y, int width, int height) {
    m_width = width;
    m_height = height;
    
    // Set viewport via JavaScript bridge
#ifdef __EMSCRIPTEN__
    EM_ASM({
        if (typeof window.webglBridge !== 'undefined' && window.webglBridge.setViewport) {
            window.webglBridge.setViewport($0, $1, $2, $3);
        }
    }, x, y, width, height);
#endif
}

void PureWebGLRenderer::Clear(float r, float g, float b, float a) {
    // Clear the screen via JavaScript bridge
#ifdef __EMSCRIPTEN__
    EM_ASM({
        if (typeof window.webglBridge !== 'undefined' && window.webglBridge.clear) {
            window.webglBridge.clear($0, $1, $2, $3);
        }
    }, r, g, b, a);
#endif
}

void PureWebGLRenderer::Present() {
    // Present the frame via JavaScript bridge
#ifdef __EMSCRIPTEN__
    EM_ASM({
        if (typeof window.webglBridge !== 'undefined' && window.webglBridge.present) {
            window.webglBridge.present();
        }
    });
#endif
}

bool PureWebGLRenderer::SupportsFeature(RendererFeature feature) {
    switch (feature) {
        case RendererFeature::WebGL1:
        case RendererFeature::WebGL2:
            return true;
        case RendererFeature::WebGPU:
        case RendererFeature::TextureCompression:
        case RendererFeature::ShaderSupport:
        case RendererFeature::MultiSampling:
            return false; // Not implemented yet
        default:
            return false;
    }
}

std::string PureWebGLRenderer::GetRendererName() const {
    return "Pure WebGL Renderer (WASM)";
}

float PureWebGLRenderer::GetFrameTime() const {
    return m_frameTime;
}

int PureWebGLRenderer::GetDrawCalls() const {
    return m_drawCalls;
}

void PureWebGLRenderer::ResetStats() {
    m_frameTime = 0.0f;
    m_drawCalls = 0;
}

// Private implementation methods

bool PureWebGLRenderer::InitializeWebGL() {
    return CallJavaScriptWebGLInit();
}

bool PureWebGLRenderer::CreateShaders() {
    return CallJavaScriptCreateShaders();
}

bool PureWebGLRenderer::CreateBuffers() {
    return CallJavaScriptCreateBuffers();
}

bool PureWebGLRenderer::CreateBasicShaders() {
    // This will be implemented via JavaScript bridge
    return true;
}

void PureWebGLRenderer::RenderQuad(float x, float y, float width, float height, float r, float g, float b, float a) {
    CallJavaScriptRenderQuad(x, y, width, height, r, g, b, a);
}

void PureWebGLRenderer::RenderTexture(float x, float y, float width, float height, int textureId) {
    // This will be implemented via JavaScript bridge
    // For now, just render a colored quad
    RenderQuad(x, y, width, height, 1.0f, 1.0f, 1.0f, 1.0f);
}

// JavaScript bridge calls

bool PureWebGLRenderer::CallJavaScriptWebGLInit() {
#ifdef __EMSCRIPTEN__
    int result = EM_ASM_INT({
        if (typeof window.webglBridge !== 'undefined' && window.webglBridge.initialize) {
            return window.webglBridge.initialize() ? 1 : 0;
        }
        return 0;
    });
    return result == 1;
#else
    return false;
#endif
}

bool PureWebGLRenderer::CallJavaScriptCreateShaders() {
#ifdef __EMSCRIPTEN__
    int result = EM_ASM_INT({
        if (typeof window.webglBridge !== 'undefined' && window.webglBridge.createShaders) {
            return window.webglBridge.createShaders() ? 1 : 0;
        }
        return 0;
    });
    return result == 1;
#else
    return false;
#endif
}

bool PureWebGLRenderer::CallJavaScriptCreateBuffers() {
#ifdef __EMSCRIPTEN__
    int result = EM_ASM_INT({
        if (typeof window.webglBridge !== 'undefined' && window.webglBridge.createBuffers) {
            return window.webglBridge.createBuffers() ? 1 : 0;
        }
        return 0;
    });
    return result == 1;
#else
    return false;
#endif
}

void PureWebGLRenderer::CallJavaScriptRenderQuad(float x, float y, float width, float height, float r, float g, float b, float a) {
#ifdef __EMSCRIPTEN__
    EM_ASM({
        if (typeof window.webglBridge !== 'undefined' && window.webglBridge.renderQuad) {
            window.webglBridge.renderQuad($0, $1, $2, $3, $4, $5, $6, $7);
        }
    }, x, y, width, height, r, g, b, a);
#endif
}
