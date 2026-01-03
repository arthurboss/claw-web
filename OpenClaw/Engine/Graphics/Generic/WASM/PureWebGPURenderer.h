#pragma once

#include "../IGenericRenderer.h"
#include <string>
#include <memory>
#include <vector>
#include <unordered_map>
#include <iostream>

#ifdef __EMSCRIPTEN__
#include <webgpu/webgpu.h>
#endif

// Forward declarations
class PureWebGPUShader;
class PureWebGPUTexture;

/**
 * @brief Pure WebGPU renderer for WASM builds
 * 
 * This renderer provides WebGPU graphics functionality directly through
 * the browser's WebGPU API. It serves as the primary renderer for modern
 * browsers, offering higher performance than WebGL.
 */
class GenericWebGPURenderer : public IGenericRenderer {
public:
    GenericWebGPURenderer();
    ~GenericWebGPURenderer() override;

    // ===== Core Initialization & Lifecycle =====
    bool Initialize(int width, int height, const std::string& title = "") override;
    void Shutdown() override;
    bool IsInitialized() const override;

    // ===== Platform & Capability Detection =====
    std::string GetRendererType() const override;
    std::string GetPlatform() const override;
    bool IsFeatureSupported(const std::string& feature) const override;
    int GetMaxTextureSize() const override;

    // ===== Rendering & Display =====
    void BeginFrame() override;
    void EndFrame() override;
    void Clear(float r, float g, float b, float a = 1.0f) override;
    void SetViewport(int x, int y, int width, int height) override;

    // ===== Resource Management =====
    std::shared_ptr<ITexture> CreateTexture(int width, int height, const uint8_t* data) override;
    std::shared_ptr<ITexture> LoadTexture(const std::string& filepath) override;
    std::shared_ptr<IShader> CreateShader(const std::string& vertexSource, const std::string& fragmentSource) override;

    // ===== Rendering Operations =====
    void RenderSprite(const std::shared_ptr<ISprite>& sprite, float x, float y, float scale = 1.0f, float rotation = 0.0f) override;
    void RenderQuad(const std::shared_ptr<ITexture>& texture, float x, float y, float width, float height) override;
    void RenderText(const std::string& text, float x, float y, uint32_t color = 0xFFFFFFFF, int fontSize = 16) override;

    // ===== Performance & Monitoring =====
    float GetFPS() const override;
    std::string GetMemoryStats() const override;
    std::string GetPerformanceStats() const override;
    void SetPerformanceMonitoring(bool enabled) override;

    // ===== Advanced Features =====
    void SetPostProcessEffect(const std::string& effect, bool enabled) override;
    void SetPostProcessParameter(const std::string& effect, const std::string& param, float value) override;
    std::shared_ptr<IParticleSystem> CreateParticleSystem(size_t maxParticles) override;

private:
    // ===== Private Implementation Details =====
    bool InitializeWebGPU();
    void SetupDevice();
    void CreateSwapchain();
    bool CreateDefaultPipelines();
    
    // ===== Member Variables =====
    bool m_initialized;
    int m_screenWidth;
    int m_screenHeight;
    
#ifdef __EMSCRIPTEN__
    WGPUInstance m_instance;
    WGPUSurface m_surface;
    WGPUAdapter m_adapter;
    WGPUDevice m_device;
    WGPUQueue m_queue;
    WGPUSwapChain m_swapChain;
    WGPUTextureFormat m_swapChainFormat;
    
    WGPURenderPipeline m_pipeline;
    WGPUCommandEncoder m_commandEncoder;
    WGPURenderPassEncoder m_renderPassEncoder;
#endif

    // Performance monitoring
    bool m_performanceMonitoring;
    float m_fps;
    
    // Feature support
    std::unordered_map<std::string, bool> m_featureSupport;
};
