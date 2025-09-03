#pragma once

#include "../IRenderer.h"
#include <string>

// Forward declarations
struct MenuBackgroundData;
struct MenuItemData;
struct MenuTextData;

// WASM-Only Pure WebGPU Renderer (No SDL Dependencies)
// This renderer is ONLY compiled for WASM builds and provides
// direct WebGPU access without any SDL abstraction layers
class PureWebGPURenderer : public IRenderer {
private:
    // WebGPU context and state
    bool m_isInitialized;
    int m_width, m_height;
    
    // WebGPU objects managed via JavaScript bridge
    int m_webgpuAdapterId;     // Reference to JavaScript WebGPU adapter
    int m_webgpuDeviceId;      // Reference to JavaScript WebGPU device
    int m_webgpuQueueId;       // Reference to JavaScript WebGPU queue
    int m_webgpuPipelineId;    // Reference to JavaScript WebGPU pipeline
    int m_webgpuBufferId;      // Reference to JavaScript WebGPU buffer
    
    // Performance tracking
    float m_frameTime;
    int m_drawCalls;
    int m_fps;
    float m_lastFrameTime;

    // Helper for rendering a quad (e.g., for background, menu items, text)
    void RenderQuad(float x, float y, float width, float height, float r, float g, float b, float a);

public:
    PureWebGPURenderer();
    ~PureWebGPURenderer();

    // IRenderer Interface Implementation
    bool Initialize() override;
    void Shutdown() override;
    void BeginFrame() override;
    void EndFrame() override;
    void ResetStats() override;
    float GetFrameTime() const override { return m_frameTime; }
    int GetDrawCalls() const override { return m_drawCalls; }
    std::string GetRendererName() const override { return "PureWebGPURenderer"; }
    bool SupportsFeature(RendererFeature feature) override;
    
    // Additional required IRenderer methods
    void SetViewport(int x, int y, int width, int height) override;
    void Clear(float r, float g, float b, float a) override;
    void Present() override;

    // Menu-specific rendering
    void RenderMenuBackground(const MenuBackgroundData& data) override;
    void RenderMenuItem(const MenuItemData& data) override;
    void RenderMenuText(const MenuTextData& data) override;

    // WebGPU-specific methods
    bool IsWebGPUAvailable() const;
    int GetFPS() const { return m_fps; }
    void UpdatePerformanceMetrics();
};
