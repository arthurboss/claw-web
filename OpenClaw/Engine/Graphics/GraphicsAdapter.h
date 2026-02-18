#pragma once

#include "GraphicsManager.h"
#include "Data/MenuBackgroundData.h"
#include "Data/MenuItemData.h"
#include "Data/MenuTextData.h"
#include "../UserInterface/UserInterface.h"
#include <memory>

// Conditional includes based on platform
#ifdef __EMSCRIPTEN__
    // WASM: No SDL dependencies
    #define USE_WASM_RENDERER 1
#else
    // Non-WASM: Include SDL dependencies
    #include <SDL2/SDL.h>
    #define USE_WASM_RENDERER 0
#endif

// Forward declarations
class Image;

// Graphics adapter that provides a bridge to the new modular graphics system
class GraphicsAdapter {
private:
    std::unique_ptr<GraphicsManager> m_graphicsManager;
    bool m_isInitialized;
    
public:
    GraphicsAdapter();
    ~GraphicsAdapter();
    
    // Initialize/shutdown
    bool Initialize();
    #if !USE_WASM_RENDERER
        bool Initialize(SDL_Renderer* existingRenderer); // Overload to use existing renderer (non-WASM only)
    #endif
    void Shutdown();
    
    // Direct rendering methods (for integration with existing system)
    void BeginFrame();
    void EndFrame();
    
    // Background rendering
    void RenderBackground(shared_ptr<Image> background, const SDL_Rect& rect);
    
    // Menu item rendering
    void RenderMenuItem(const std::string& name, const Point& position, 
                       float width, float height, ::MenuItemState state, bool visible);
    
    // Text rendering
    void RenderText(const std::string& text, const Point& position, float fontSize = 16.0f);
    
    // Utility methods
    bool IsInitialized() const { return m_isInitialized; }
    GraphicsManager* GetGraphicsManager() { return m_graphicsManager.get(); }
    std::string GetRendererName() const;
    
    // WebGPU status methods
    bool IsUsingWebGPU() const;
    bool IsUsingWebGL() const;
    std::string GetRendererStatus() const;
    
    // Performance queries
    float GetFrameTime() const;
    int GetDrawCalls() const;
    void ResetStats();
    
private:
    // Helper methods
    GraphicsMenuItemState ConvertMenuItemState(::MenuItemState oldState);
};
