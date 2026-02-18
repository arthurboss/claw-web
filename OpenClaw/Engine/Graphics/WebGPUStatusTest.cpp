#include "GraphicsAdapter.h"
#include "../Logger/Logger.h"
#include <iostream>

// Test function to demonstrate WebGPU status checking
void TestWebGPUStatus() {
    LOG("=== Testing WebGPU Status Detection ===");
    
    // Create graphics adapter
    GraphicsAdapter graphicsAdapter;
    
    // Initialize the graphics system
    if (!graphicsAdapter.Initialize()) {
        LOG_ERROR("Failed to initialize GraphicsAdapter");
        return;
    }
    
    LOG("GraphicsAdapter initialized successfully");
    
    // Check WebGPU status
    bool isUsingWebGPU = graphicsAdapter.IsUsingWebGPU();
    bool isUsingWebGL = graphicsAdapter.IsUsingWebGL();
    
    LOG("WebGPU Status Check:");
    LOG("  Using WebGPU: " + std::string(isUsingWebGPU ? "✅ YES" : "❌ NO"));
    LOG("  Using WebGL: " + std::string(isUsingWebGL ? "✅ YES" : "❌ NO"));
    
    if (isUsingWebGPU) {
        LOG("🎉 WebGPU is active! You should see better performance.");
        LOG("   - Modern graphics API");
        LOG("   - Better performance");
        LOG("   - Advanced features available");
    } else if (isUsingWebGL) {
        LOG("📱 Using WebGL fallback (still good performance)");
        LOG("   - Compatible with most browsers");
        LOG("   - Good performance");
        LOG("   - Limited advanced features");
    } else {
        LOG("⚠️  No graphics system available");
        LOG("   - Check browser compatibility");
        LOG("   - Try updating your browser");
    }
    
    // Get detailed renderer status
    std::string rendererStatus = graphicsAdapter.GetRendererStatus();
    LOG("Detailed Renderer Status:");
    LOG(rendererStatus);
    
    // Get renderer name
    std::string rendererName = graphicsAdapter.GetRendererName();
    LOG("Active Renderer: " + rendererName);
    
    // Performance info
    float frameTime = graphicsAdapter.GetFrameTime();
    int drawCalls = graphicsAdapter.GetDrawCalls();
    LOG("Performance Info:");
    LOG("  Frame Time: " + std::to_string(frameTime) + "ms");
    LOG("  Draw Calls: " + std::to_string(drawCalls));
    
    // Shutdown
    graphicsAdapter.Shutdown();
    
    LOG("=== WebGPU Status Test Complete ===");
}

// Function to check WebGPU status from anywhere in the code
void CheckWebGPUStatus(GraphicsAdapter* adapter) {
    if (!adapter) {
        LOG("GraphicsAdapter not available");
        return;
    }
    
    if (adapter->IsUsingWebGPU()) {
        LOG("🎉 WebGPU is active - Modern graphics API in use!");
    } else if (adapter->IsUsingWebGL()) {
        LOG("📱 Using WebGL fallback - Good compatibility, decent performance");
    } else {
        LOG("⚠️  No graphics system available");
    }
    
    LOG("Renderer: " + adapter->GetRendererName());
}

// Example of how to use in the main application
void ExampleWebGPUUsage() {
    LOG("=== Example WebGPU Usage ===");
    
    /*
    // In your main application (e.g., BaseGameApp):
    
    GraphicsAdapter* graphicsAdapter = GetGraphicsAdapter();
    
    if (graphicsAdapter && graphicsAdapter->IsInitialized()) {
        // Check what graphics system is being used
        if (graphicsAdapter->IsUsingWebGPU()) {
            LOG("🎉 Using WebGPU - Modern graphics API!");
            // You can enable advanced features here
            // EnablePostProcessing();
            // EnableAdvancedLighting();
        } else if (graphicsAdapter->IsUsingWebGL()) {
            LOG("📱 Using WebGL - Good compatibility");
            // Use standard features
        }
        
        // Log detailed status
        LOG(graphicsAdapter->GetRendererStatus());
        
        // Monitor performance
        float frameTime = graphicsAdapter->GetFrameTime();
        if (frameTime > 16.67f) { // More than 60 FPS threshold
            LOG_WARNING("Frame time high: " + std::to_string(frameTime) + "ms");
        }
    }
    */
    
    LOG("=== Example WebGPU Usage Complete ===");
}
