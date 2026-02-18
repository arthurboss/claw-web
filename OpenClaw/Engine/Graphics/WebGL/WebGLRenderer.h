#pragma once

#include "../IRenderer.h"
#include "../Data/MenuBackgroundData.h"
#include "../Data/MenuItemData.h"
#include "../Data/MenuTextData.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <map>
#include <string>

// WebGL renderer implementation
class WebGLRenderer : public IRenderer {
private:
    SDL_Renderer* sdlRenderer;
    TTF_Font* defaultFont;
    bool isInitialized;
    bool ownsRenderer; // Track if we created the renderer or it was provided externally
    
    // Performance tracking
    float frameTime;
    int drawCalls;
    uint32_t frameStartTime;
    
    // Viewport
    int viewportX, viewportY, viewportWidth, viewportHeight;
    
    // Texture cache
    std::map<std::string, SDL_Texture*> textureCache;
    
public:
    // Constructor/Destructor
    WebGLRenderer();
    ~WebGLRenderer();
    
    // Core rendering operations
    bool Initialize() override;
    bool Initialize(SDL_Renderer* existingRenderer); // New overload to use existing renderer
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
    float GetFrameTime() const override { return frameTime; }
    int GetDrawCalls() const override { return drawCalls; }
    void ResetStats() override;
    
private:
    // Helper methods
    bool InitializeRenderer(); // Common initialization logic
    void ResetPerformanceStats();
    SDL_Texture* LoadTexture(const std::string& path);
    void RenderTexture(SDL_Texture* texture, float x, float y, float width, float height, float alpha);
    void RenderTexture(const std::string& texturePath, float x, float y, float width, float height, float alpha);
    void ClearTextureCache();
    SDL_Texture* CreateTextTexture(const std::string& text, const MenuTextData& textData);
};
