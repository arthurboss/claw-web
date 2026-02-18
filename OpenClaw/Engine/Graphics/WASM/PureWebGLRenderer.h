#pragma once

#include "../IRenderer.h"
#include <string>

// Forward declarations
struct MenuBackgroundData;
struct MenuItemData;
struct MenuTextData;

// WASM-Only Pure WebGL Renderer (No SDL Dependencies)
// This renderer is ONLY compiled for WASM builds and provides
// direct WebGL access without any SDL abstraction layers
class PureWebGLRenderer : public IRenderer {
private:
    // WebGL context and state
    bool m_isInitialized;
    int m_width, m_height;
    
    // WebGL objects managed via JavaScript bridge
    int m_webglContextId;  // Reference to JavaScript WebGL context
    int m_vertexShaderId;  // Reference to JavaScript vertex shader
    int m_fragmentShaderId; // Reference to JavaScript fragment shader
    int m_programId;       // Reference to JavaScript shader program
    
    // Basic WebGL buffers
    int m_vertexBufferId;  // Reference to JavaScript VBO
    int m_indexBufferId;   // Reference to JavaScript IBO
    
    // Rendering state
    float m_frameTime;
    int m_drawCalls;
    
public:
    PureWebGLRenderer();
    virtual ~PureWebGLRenderer();
    
    // IRenderer interface implementation
    bool Initialize() override;
    void Shutdown() override;
    void BeginFrame() override;
    void EndFrame() override;
    
    // Menu-specific rendering
    void RenderMenuBackground(const MenuBackgroundData& data) override;
    void RenderMenuItem(const MenuItemData& data) override;
    void RenderMenuText(const MenuTextData& data) override;
    
    // Common operations
    void SetViewport(int x, int y, int width, int height) override;
    void Clear(float r, float g, float b, float a) override;
    void Present() override;
    
    // Capability queries
    bool SupportsFeature(RendererFeature feature) override;
    std::string GetRendererName() const override;
    
    // Performance queries
    float GetFrameTime() const override;
    int GetDrawCalls() const override;
    void ResetStats() override;
    
private:
    // WebGL initialization and setup
    bool InitializeWebGL();
    bool CreateShaders();
    bool CreateBuffers();
    bool CreateBasicShaders();
    
    // WebGL rendering helpers
    void RenderQuad(float x, float y, float width, float height, float r, float g, float b, float a);
    void RenderTexture(float x, float y, float width, float height, int textureId);
    
    // JavaScript bridge calls
    bool CallJavaScriptWebGLInit();
    bool CallJavaScriptCreateShaders();
    bool CallJavaScriptCreateBuffers();
    void CallJavaScriptRenderQuad(float x, float y, float width, float height, float r, float g, float b, float a);
};
