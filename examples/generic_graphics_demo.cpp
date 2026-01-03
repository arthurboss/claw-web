/**
 * @file generic_graphics_demo.cpp
 * @brief Demonstration of the GenericGraphicsModule usage
 * 
 * This example shows how to use the GenericGraphicsModule in a simple
 * graphics application. It demonstrates:
 * - Module initialization and configuration
 * - Platform detection and renderer selection
 * - Basic rendering operations
 * - Configuration management
 * - Performance monitoring
 */

#include "OpenClaw/Engine/Graphics/Generic/GenericGraphicsModule.h"
#include <iostream>
#include <chrono>
#include <thread>

int main() {
    std::cout << "=== GenericGraphicsModule Demo ===" << std::endl;
    
    // Create the graphics module
    GenericGraphicsModule graphics;
    
    // Display module information
    std::cout << "\n--- Module Information ---" << std::endl;
    std::cout << "Version: " << GenericGraphicsModule::GetVersion() << std::endl;
    std::cout << "Build Info:\n" << GenericGraphicsModule::GetBuildInfo() << std::endl;
    
    // Display platform and capabilities before initialization
    std::cout << "\n--- Platform Detection ---" << std::endl;
    std::cout << "Platform: " << graphics.GetPlatform() << std::endl;
    std::cout << "Available Renderers: ";
    for (const auto& renderer : graphics.GetAvailableRenderers()) {
        std::cout << renderer << " ";
    }
    std::cout << std::endl;
    
    // Configure the module
    std::cout << "\n--- Configuration ---" << std::endl;
    graphics.SetPreferredRenderer("WebGL");
    graphics.SetDefaultTextureFiltering("linear");
    graphics.SetDefaultBlending("alpha");
    graphics.SetMaxTextureCacheSize(128); // 128MB cache
    graphics.SetPerformanceMonitoring(true);
    
    // Display current configuration
    std::cout << graphics.GetConfiguration() << std::endl;
    
    // Initialize the module
    std::cout << "\n--- Initialization ---" << std::endl;
    if (!graphics.Initialize(800, 600, "Generic Graphics Demo")) {
        std::cerr << "Failed to initialize graphics module!" << std::endl;
        return 1;
    }
    
    // Display capabilities after initialization
    std::cout << "\n--- Capabilities ---" << std::endl;
    std::cout << graphics.GetCapabilityInfo() << std::endl;
    
    // Simulate a simple rendering loop
    std::cout << "\n--- Rendering Demo ---" << std::endl;
    const int numFrames = 60; // Simulate 1 second at 60 FPS
    
    for (int frame = 0; frame < numFrames; ++frame) {
        // Begin frame
        graphics.BeginFrame();
        
        // Clear with animated color
        float time = frame / 60.0f;
        float r = 0.5f + 0.5f * sin(time * 2.0f);
        float g = 0.5f + 0.5f * sin(time * 2.0f + 2.0f);
        float b = 0.5f + 0.5f * sin(time * 2.0f + 4.0f);
        graphics.Clear(r, g, b, 1.0f);
        
        // Simulate some rendering work
        // In a real application, you would render sprites, textures, etc.
        
        // End frame
        graphics.EndFrame();
        
        // Display progress every 10 frames
        if (frame % 10 == 0) {
            std::cout << "Frame " << frame << "/" << numFrames 
                      << " - FPS: " << graphics.GetFPS() << std::endl;
        }
        
        // Simulate frame time (16.67ms for 60 FPS)
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }
    
    // Display final performance stats
    std::cout << "\n--- Final Performance Stats ---" << std::endl;
    std::cout << graphics.GetPerformanceStats() << std::endl;
    std::cout << graphics.GetMemoryStats() << std::endl;
    
    // Test configuration changes at runtime
    std::cout << "\n--- Runtime Configuration Test ---" << std::endl;
    std::string config = "texture_filtering=nearest\nblending=additive\nmax_texture_cache=64";
    if (graphics.SetConfiguration(config)) {
        std::cout << "Configuration updated successfully!" << std::endl;
        std::cout << graphics.GetConfiguration() << std::endl;
    } else {
        std::cout << "Failed to update configuration!" << std::endl;
    }
    
    // Shutdown
    std::cout << "\n--- Shutdown ---" << std::endl;
    graphics.Shutdown();
    
    std::cout << "\nDemo completed successfully!" << std::endl;
    return 0;
}
