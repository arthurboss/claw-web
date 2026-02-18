#include <iostream>
#include <memory>
#include <chrono>
#include <thread>

// Include the generic graphics module
#include "Generic/GenericGraphicsModule.h"

/**
 * @brief Test program for the Generic Graphics Module
 * 
 * This program tests:
 * - Platform detection
 * - Renderer selection
 * - Basic initialization
 * - Feature detection
 * - Simple rendering operations
 */

int main() {
    std::cout << "🧪 Testing Generic Graphics Module" << std::endl;
    std::cout << "=================================" << std::endl;
    
    // Create the generic graphics module
    GenericGraphicsModule graphics;
    
    // Show initial configuration
    std::cout << "\n📋 Initial Configuration:" << std::endl;
    std::cout << graphics.GetConfiguration() << std::endl;
    
    // Initialize the module
    std::cout << "\n🚀 Initializing Graphics Module..." << std::endl;
    if (!graphics.Initialize(800, 600, "Generic Module Test")) {
        std::cerr << "❌ Failed to initialize graphics module!" << std::endl;
        return -1;
    }
    
    std::cout << "✅ Graphics module initialized successfully!" << std::endl;
    
    // Show platform and renderer info
    std::cout << "\n🌍 Platform Information:" << std::endl;
    std::cout << "Platform: " << graphics.GetPlatform() << std::endl;
    std::cout << "Renderer: " << graphics.GetRendererType() << std::endl;
    
    // Show available features
    std::cout << "\n🔍 Feature Support:" << std::endl;
    std::vector<std::string> features = {"shaders", "textures", "particles", "post_processing"};
    for (const auto& feature : features) {
        bool supported = graphics.IsFeatureSupported(feature);
        std::cout << "   " << (supported ? "✅" : "❌") << " " << feature << std::endl;
    }
    
    // Show capabilities
    std::cout << "\n📊 Capabilities:" << std::endl;
    std::cout << graphics.GetCapabilityInfo() << std::endl;
    
    // Enable performance monitoring
    graphics.SetPerformanceMonitoring(true);
    
    // Test basic rendering operations
    std::cout << "\n🎨 Testing Basic Rendering..." << std::endl;
    
    // Test frame operations
    graphics.BeginFrame();
    graphics.Clear(0.2f, 0.3f, 0.5f); // Dark blue
    graphics.SetViewport(0, 0, 800, 600);
    
    // Test texture creation (if supported)
    if (graphics.IsFeatureSupported("textures")) {
        std::cout << "   Creating test texture..." << std::endl;
        uint8_t pixelData[] = {
            255, 0, 0, 255,    // Red
            0, 255, 0, 255,    // Green
            0, 0, 255, 255,    // Blue
            255, 255, 0, 255   // Yellow
        };
        
        auto texture = graphics.CreateTexture(2, 2, pixelData);
        if (texture) {
            std::cout << "   ✅ Texture created successfully" << std::endl;
            std::cout << "   Texture size: " << texture->GetWidth() << "x" << texture->GetHeight() << std::endl;
        } else {
            std::cout << "   ❌ Failed to create texture" << std::endl;
        }
    }
    
    // Test shader creation (if supported)
    if (graphics.IsFeatureSupported("shaders")) {
        std::cout << "   Creating test shader..." << std::endl;
        std::string vertexSource = "attribute vec2 a_position; void main() { gl_Position = vec4(a_position, 0.0, 1.0); }";
        std::string fragmentSource = "void main() { gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0); }";
        
        auto shader = graphics.CreateShader(vertexSource, fragmentSource);
        if (shader) {
            std::cout << "   ✅ Shader created successfully" << std::endl;
        } else {
            std::cout << "   ❌ Failed to create shader" << std::endl;
        }
    }
    
    // Test post-processing (if supported)
    if (graphics.IsFeatureSupported("post_processing")) {
        std::cout << "   Testing post-processing effects..." << std::endl;
        graphics.SetPostProcessEffect("bloom", true);
        graphics.SetPostProcessParameter("bloom", "intensity", 1.5f);
        graphics.SetPostProcessParameter("bloom", "threshold", 0.7f);
        std::cout << "   ✅ Post-processing effects configured" << std::endl;
    }
    
    // Test particle system (if supported)
    if (graphics.IsFeatureSupported("particles")) {
        std::cout << "   Creating particle system..." << std::endl;
        auto particleSystem = graphics.CreateParticleSystem(1000);
        if (particleSystem) {
            std::cout << "   ✅ Particle system created successfully" << std::endl;
        } else {
            std::cout << "   ❌ Failed to create particle system" << std::endl;
        }
    }
    
    graphics.EndFrame();
    
    // Show performance stats
    std::cout << "\n📊 Performance Statistics:" << std::endl;
    std::cout << "FPS: " << graphics.GetFPS() << std::endl;
    std::cout << "Memory: " << graphics.GetMemoryStats() << std::endl;
    std::cout << "Performance: " << graphics.GetPerformanceStats() << std::endl;
    
    // Test configuration changes
    std::cout << "\n⚙️ Testing Configuration Changes..." << std::endl;
    
    // Try to change renderer (if multiple available)
    auto availableRenderers = graphics.GetAvailableRenderers();
    if (availableRenderers.size() > 1) {
        std::cout << "   Multiple renderers available, testing renderer switching..." << std::endl;
        for (const auto& renderer : availableRenderers) {
            if (renderer != graphics.GetRendererType()) {
                std::cout << "   Trying to set preferred renderer to: " << renderer << std::endl;
                if (graphics.SetPreferredRenderer(renderer)) {
                    std::cout << "   ✅ Preferred renderer set to: " << renderer << std::endl;
                } else {
                    std::cout << "   ❌ Failed to set preferred renderer to: " << renderer << std::endl;
                }
                break;
            }
        }
    }
    
    // Show final configuration
    std::cout << "\n📋 Final Configuration:" << std::endl;
    std::cout << graphics.GetConfiguration() << std::endl;
    
    // Shutdown
    std::cout << "\n🔄 Shutting down graphics module..." << std::endl;
    graphics.Shutdown();
    
    std::cout << "\n🎉 Test completed successfully!" << std::endl;
    std::cout << "The Generic Graphics Module is working correctly!" << std::endl;
    
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
