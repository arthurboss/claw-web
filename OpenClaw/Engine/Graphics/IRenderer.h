#pragma once

#include <string>
#include <memory>

// Forward declarations
struct MenuBackgroundData;
struct MenuItemData;
struct MenuTextData;

// Renderer feature enumeration
enum class RendererFeature {
    WebGPU,
    WebGL2,
    WebGL1,
    TextureCompression,
    ShaderSupport,
    MultiSampling
};

// Abstract renderer interface
class IRenderer {
public:
    virtual ~IRenderer() = default;
    
    // Core rendering operations
    virtual bool Initialize() = 0;
    virtual void Shutdown() = 0;
    virtual void BeginFrame() = 0;
    virtual void EndFrame() = 0;
    
    // Menu-specific rendering
    virtual void RenderMenuBackground(const MenuBackgroundData& data) = 0;
    virtual void RenderMenuItem(const MenuItemData& data) = 0;
    virtual void RenderMenuText(const MenuTextData& data) = 0;
    
    // Common operations
    virtual void SetViewport(int x, int y, int width, int height) = 0;
    virtual void Clear(float r, float g, float b, float a) = 0;
    virtual void Present() = 0;
    
    // Capability queries
    virtual bool SupportsFeature(RendererFeature feature) = 0;
    virtual std::string GetRendererName() const = 0;
    
    // Performance queries
    virtual float GetFrameTime() const = 0;
    virtual int GetDrawCalls() const = 0;
    virtual void ResetStats() = 0;
};
