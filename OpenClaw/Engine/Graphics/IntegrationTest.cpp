#include "GraphicsAdapter.h"
#include "../Logger/Logger.h"
#include <iostream>

// Example integration test showing how to use GraphicsAdapter with existing menu system
void TestGraphicsIntegration() {
    LOG("=== Testing Graphics Integration ===");
    
    // Create graphics adapter
    GraphicsAdapter graphicsAdapter;
    
    // Initialize the graphics system
    if (!graphicsAdapter.Initialize()) {
        LOG_ERROR("Failed to initialize GraphicsAdapter");
        return;
    }
    
    LOG("GraphicsAdapter initialized successfully");
    LOG("Active renderer: " + graphicsAdapter.GetRendererName());
    
    // Simulate menu rendering (this would be called from the existing menu system)
    graphicsAdapter.BeginFrame();
    
    // Simulate rendering a menu background
    // In the real system, this would be called from ScreenElementMenu::VOnRender
    SDL_Rect backgroundRect = {0, 0, 1280, 960};
    // graphicsAdapter.RenderBackground(background, backgroundRect);
    
    // Simulate rendering menu items
    // In the real system, this would be called from ScreenElementMenuItem::VOnRender
    Point buttonPos(100, 200);
    graphicsAdapter.RenderMenuItem("StartGame", buttonPos, 200.0f, 50.0f, MenuItemState_Active, true);
    
    Point buttonPos2(100, 300);
    graphicsAdapter.RenderMenuItem("Options", buttonPos2, 200.0f, 50.0f, MenuItemState_Inactive, true);
    
    // Simulate rendering text
    Point textPos(50, 50);
    graphicsAdapter.RenderText("OpenClaw - WebGPU Integration Test", textPos, 24.0f);
    
    graphicsAdapter.EndFrame();
    
    // Display performance stats
    LOG("Frame time: " + std::to_string(graphicsAdapter.GetFrameTime()) + "ms");
    LOG("Draw calls: " + std::to_string(graphicsAdapter.GetDrawCalls()));
    
    // Shutdown
    graphicsAdapter.Shutdown();
    
    LOG("=== Graphics Integration Test Complete ===");
}

// Example of how to integrate with existing ScreenElementMenu::VOnRender
void ExampleMenuIntegration() {
    LOG("=== Example Menu Integration ===");
    
    /*
    // This is how you would integrate with the existing menu system:
    
    // In ScreenElementMenu::VOnRender, you would add:
    
    GraphicsAdapter* graphicsAdapter = GetGraphicsAdapter(); // Get from somewhere
    
    if (graphicsAdapter && graphicsAdapter->IsInitialized()) {
        // Use new graphics system
        graphicsAdapter->BeginFrame();
        
        // Render background
        if (m_pBackground && m_pBackground->GetTexture()) {
            SDL_Rect backgroundRect = GetScreenRect();
            graphicsAdapter->RenderBackground(m_pBackground, backgroundRect);
        }
        
        // Render active page (this would be handled by the page itself)
        if (m_pActiveMenuPage) {
            m_pActiveMenuPage->VOnRender(msDiff);
        }
        
        graphicsAdapter->EndFrame();
    } else {
        // Fallback to original SDL2 rendering
        SDL_RenderSetScale(m_pRenderer, 1.0f, 1.0f);
        // ... original rendering code ...
    }
    
    // In ScreenElementMenuItem::VOnRender, you would add:
    
    GraphicsAdapter* graphicsAdapter = GetGraphicsAdapter(); // Get from somewhere
    
    if (graphicsAdapter && graphicsAdapter->IsInitialized()) {
        // Use new graphics system
        shared_ptr<Image> pCurrImage = m_Images[m_State];
        if (pCurrImage) {
            graphicsAdapter->RenderMenuItem(
                m_Name,
                m_Position,
                pCurrImage->GetWidth(),
                pCurrImage->GetHeight(),
                m_State,
                m_bVisible
            );
        }
    } else {
        // Fallback to original SDL2 rendering
        // ... original rendering code ...
    }
    */
    
    LOG("=== Example Menu Integration Complete ===");
}

