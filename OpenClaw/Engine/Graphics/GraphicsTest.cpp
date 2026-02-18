#include "GraphicsManager.h"
#include "Data/MenuBackgroundData.h"
#include "Data/MenuItemData.h"
#include "Data/MenuTextData.h"
#include "../Logger/Logger.h"
#include <iostream>

// Simple test function to demonstrate the graphics system
void TestGraphicsSystem() {
    LOG("=== Testing Graphics System ===");
    
    // Create graphics manager
    GraphicsManager graphicsManager;
    
    // Initialize graphics system
    if (!graphicsManager.Initialize()) {
        LOG_ERROR("Failed to initialize graphics system");
        return;
    }
    
    LOG("Graphics system initialized successfully");
    LOG("Active renderer: " + graphicsManager.GetRendererName());
    LOG("Renderer type: " + std::to_string(static_cast<int>(graphicsManager.GetCurrentType())));
    
    // Test feature support
    LOG("WebGPU Support: " + std::string(graphicsManager.SupportsFeature(RendererFeature::WebGPU) ? "Yes" : "No"));
    LOG("WebGL2 Support: " + std::string(graphicsManager.SupportsFeature(RendererFeature::WebGL2) ? "Yes" : "No"));
    LOG("WebGL1 Support: " + std::string(graphicsManager.SupportsFeature(RendererFeature::WebGL1) ? "Yes" : "No"));
    
    // Create test menu data
    MenuBackgroundData backgroundData;
    backgroundData.texturePath = "menu_background.png";
    backgroundData.x = 0.0f;
    backgroundData.y = 0.0f;
    backgroundData.width = 1280.0f;
    backgroundData.height = 960.0f;
    backgroundData.alpha = 1.0f;
    backgroundData.visible = true;
    
    MenuItemData menuItemData;
    menuItemData.name = "Start Game";
    menuItemData.inactiveTexturePath = "button_inactive.png";
    menuItemData.activeTexturePath = "button_active.png";
    menuItemData.x = 100.0f;
    menuItemData.y = 200.0f;
    menuItemData.width = 200.0f;
    menuItemData.height = 50.0f;
    menuItemData.alpha = 1.0f;
    menuItemData.visible = true;
    menuItemData.state = GraphicsMenuItemState::Active;
    menuItemData.canBeFocused = true;
    
    MenuTextData textData;
    textData.text = "OpenClaw - WebGPU Migration Test";
    textData.fontPath = "clacon.ttf";
    textData.fontSize = 24.0f;
    textData.x = 50.0f;
    textData.y = 50.0f;
    textData.r = 1.0f;
    textData.g = 1.0f;
    textData.b = 1.0f;
    textData.a = 1.0f;
    textData.alignment = TextAlignment::Center;
    textData.visible = true;
    textData.bold = true;
    textData.italic = false;
    
    // Test rendering (this will just log for now since we don't have actual textures)
    LOG("Testing menu rendering...");
    
    graphicsManager.BeginFrame();
    
    // Render background
    if (graphicsManager.GetRenderer()) {
        graphicsManager.GetRenderer()->RenderMenuBackground(backgroundData);
        graphicsManager.GetRenderer()->RenderMenuItem(menuItemData);
        graphicsManager.GetRenderer()->RenderMenuText(textData);
    }
    
    graphicsManager.EndFrame();
    
    // Display performance stats
    LOG("Frame time: " + std::to_string(graphicsManager.GetFrameTime()) + "ms");
    LOG("Draw calls: " + std::to_string(graphicsManager.GetDrawCalls()));
    
    // Shutdown
    graphicsManager.Shutdown();
    
    LOG("=== Graphics System Test Complete ===");
}
