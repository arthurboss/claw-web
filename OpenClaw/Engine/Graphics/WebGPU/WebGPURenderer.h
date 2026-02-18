#ifndef __WEBGPU_RENDERER_H__
#define __WEBGPU_RENDERER_H__

#include "../IRenderer.h"
#include "../Data/MenuBackgroundData.h"
#include "../Data/MenuItemData.h"
#include "../Data/MenuTextData.h"
#include <string>

class WebGPURenderer : public IRenderer
{
public:
    WebGPURenderer();
    virtual ~WebGPURenderer();

    // IRenderer interface implementation
    virtual bool Initialize() override;
    virtual void Shutdown() override;
    virtual void BeginFrame() override;
    virtual void EndFrame() override;
    virtual void Clear(float r, float g, float b, float a) override;
    virtual void Present() override;
    virtual void SetViewport(int x, int y, int width, int height) override;
    
    // Menu rendering
    virtual void RenderMenuBackground(const MenuBackgroundData& data) override;
    virtual void RenderMenuItem(const MenuItemData& data) override;
    virtual void RenderMenuText(const MenuTextData& data) override;
    
    // Status queries
    virtual std::string GetRendererName() const override;
    virtual bool SupportsFeature(RendererFeature feature) override;
    
    // Performance queries
    virtual float GetFrameTime() const override;
    virtual int GetDrawCalls() const override;
    virtual void ResetStats() override;

private:
    bool m_isInitialized;
    int m_frameCount;
    float m_lastFrameTime;
};

#endif // __WEBGPU_RENDERER_H__
