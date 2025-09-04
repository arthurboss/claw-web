#include <iostream>
#include <memory>
#include <chrono>
#include <thread>

// Include the generic graphics module
#include "GenericGraphicsModule.h"

/**
 * @brief Simple Graphics Demo - Shows how easy it is to use the generic module
 * 
 * This example demonstrates:
 * - Automatic platform detection
 * - Easy initialization
 * - Simple rendering operations
 * - Cross-platform compatibility
 * 
 * The same code will work on:
 * - WASM (WebGL/WebGPU)
 * - Windows/Linux (SDL2 + OpenGL)
 * - Android (OpenGL ES)
 */

int main() {
    std::cout << "🚀 Simple Graphics Demo - Generic Module Test" << std::endl;
    std::cout << "=============================================" << std::endl;
    
    // Create the graphics module - it automatically detects the platform
    GenericGraphicsModule graphics;
    
    // Initialize with 800x600 resolution
    if (!graphics.Initialize(800, 600, "Generic Graphics Demo")) {
        std::cerr << "❌ Failed to initialize graphics module!" << std::endl;
        return -1;
    }
    
    std::cout << "✅ Graphics module initialized successfully!" << std::endl;
    std::cout << "🌍 Platform: " << graphics.GetPlatform() << std::endl;
    std::cout << "🎨 Renderer: " << graphics.GetRendererType() << std::endl;
    
    // Show available features
    std::cout << "\n🔍 Available Features:" << std::endl;
    std::vector<std::string> features = {"shaders", "textures", "particles", "post_processing"};
    for (const auto& feature : features) {
        bool supported = graphics.IsFeatureSupported(feature);
        std::cout << "   " << (supported ? "✅" : "❌") << " " << feature << std::endl;
    }
    
    // Show available renderers
    std::cout << "\n🎯 Available Renderers:" << std::endl;
    auto renderers = graphics.GetAvailableRenderers();
    for (const auto& renderer : renderers) {
        std::cout << "   • " << renderer << std::endl;
    }
    
    // Enable performance monitoring
    graphics.SetPerformanceMonitoring(true);
    
    // Main render loop
    std::cout << "\n🎬 Starting render loop (5 seconds)..." << std::endl;
    
    auto startTime = std::chrono::high_resolution_clock::now();
    int frameCount = 0;
    
    while (std::chrono::duration_cast<std::chrono::seconds>(
           std::chrono::high_resolution_clock::now() - startTime).count() < 5) {
        
        // Begin frame
        graphics.BeginFrame();
        
        // Clear screen with dark blue
        graphics.Clear(0.1f, 0.1f, 0.3f);
        
        // Set viewport
        graphics.SetViewport(0, 0, 800, 600);
        
        // Create a simple colored quad (if textures are supported)
        if (graphics.IsFeatureSupported("textures")) {
            // Create a simple colored texture
            uint8_t pixelData[] = {
                255, 0, 0, 255,    // Red pixel
                0, 255, 0, 255,    // Green pixel
                0, 0, 255, 255,    // Blue pixel
                255, 255, 0, 255   // Yellow pixel
            };
            
            auto texture = graphics.CreateTexture(2, 2, pixelData);
            if (texture) {
                // Render the texture as a quad
                graphics.RenderQuad(texture, 100, 100, 200, 200);
            }
        }
        
        // Render some text
        graphics.RenderText("Generic Graphics Module", 50, 50, 0xFFFFFFFF, 24);
        graphics.RenderText("Cross-platform graphics solution", 50, 80, 0xFFFF00FF, 18);
        
        // Show current FPS
        float fps = graphics.GetFPS();
        std::string fpsText = "FPS: " + std::to_string(static_cast<int>(fps));
        graphics.RenderText(fpsText, 50, 120, 0x00FFFFFF, 16);
        
        // Show platform info
        std::string platformText = "Platform: " + graphics.GetPlatform();
        graphics.RenderText(platformText, 50, 140, 0x00FFFFFF, 16);
        
        std::string rendererText = "Renderer: " + graphics.GetRendererType();
        graphics.RenderText(rendererText, 50, 160, 0x00FFFFFF, 16);
        
        // End frame
        graphics.EndFrame();
        
        frameCount++;
        
        // Small delay to maintain reasonable FPS
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }
    
    // Show final statistics
    std::cout << "\n📊 Final Statistics:" << std::endl;
    std::cout << "   Frames rendered: " << frameCount << std::endl;
    std::cout << "   Average FPS: " << graphics.GetFPS() << std::endl;
    std::cout << "   Memory usage: " << graphics.GetMemoryStats() << std::endl;
    std::cout << "   Performance: " << graphics.GetPerformanceStats() << std::endl;
    
    // Shutdown
    graphics.Shutdown();
    std::cout << "\n✅ Graphics module shutdown complete!" << std::endl;
    
    std::cout << "\n🎉 Demo completed successfully!" << std::endl;
    std::cout << "This same code works on WASM, Windows, Linux, and Android!" << std::endl;
    
    return 0;
}

// ===== Platform-Specific Entry Points =====

#ifdef __EMSCRIPTEN__
// WASM entry point
extern "C" int main_wasm() {
    return main();
}
#endif

#ifdef _WIN32
// Windows entry point
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    return main();
}
#endif

#ifdef __ANDROID__
// Android entry point
extern "C" void android_main(struct android_app* app) {
    main();
}
#endif
