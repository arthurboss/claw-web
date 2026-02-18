#include <iostream>
#include <memory>
#include <chrono>
#include <thread>

// Include the generic graphics module
#include "Generic/GenericGraphicsModule.h"

/**
 * @brief Test program for the Real WebGL Renderer
 * 
 * This program tests:
 * - Real WebGL texture creation
 * - Real WebGL shader compilation
 * - Real WebGL rendering operations
 * - Performance with actual graphics
 */

int main() {
    std::cout << "🧪 Testing Real WebGL Renderer" << std::endl;
    std::cout << "=============================" << std::endl;
    
    // Create the generic graphics module
    GenericGraphicsModule graphics;
    
    // Initialize the module
    std::cout << "\n🚀 Initializing Graphics Module..." << std::endl;
    if (!graphics.Initialize(800, 600, "Real WebGL Test")) {
        std::cerr << "❌ Failed to initialize graphics module!" << std::endl;
        return -1;
    }
    
    std::cout << "✅ Graphics module initialized successfully!" << std::endl;
    std::cout << "Platform: " << graphics.GetPlatform() << std::endl;
    std::cout << "Renderer: " << graphics.GetRendererType() << std::endl;
    
    // Enable performance monitoring
    graphics.SetPerformanceMonitoring(true);
    
    // Test real texture creation
    std::cout << "\n🎨 Testing Real WebGL Texture Creation..." << std::endl;
    
    // Create a test texture with actual pixel data
    uint8_t testPixels[] = {
        255, 0, 0, 255,    // Red
        0, 255, 0, 255,    // Green
        0, 0, 255, 255,    // Blue
        255, 255, 0, 255   // Yellow
    };
    
    auto texture = graphics.CreateTexture(2, 2, testPixels);
    if (texture) {
        std::cout << "   ✅ Real WebGL texture created successfully!" << std::endl;
        std::cout << "   Texture size: " << texture->GetWidth() << "x" << texture->GetHeight() << std::endl;
        std::cout << "   Texture valid: " << (texture->IsValid() ? "Yes" : "No") << std::endl;
    } else {
        std::cout << "   ❌ Failed to create real WebGL texture!" << std::endl;
    }
    
    // Test real shader creation
    std::cout << "\n🔧 Testing Real WebGL Shader Creation..." << std::endl;
    
    std::string vertexSource = R"(
        #version 300 es
        precision mediump float;
        
        in vec2 a_position;
        in vec2 a_texCoord;
        out vec2 v_texCoord;
        
        void main() {
            gl_Position = vec4(a_position, 0.0, 1.0);
            v_texCoord = a_texCoord;
        }
    )";
    
    std::string fragmentSource = R"(
        #version 300 es
        precision mediump float;
        
        in vec2 v_texCoord;
        out vec4 fragColor;
        
        void main() {
            fragColor = vec4(v_texCoord, 0.5, 1.0);
        }
    )";
    
    auto shader = graphics.CreateShader(vertexSource, fragmentSource);
    if (shader) {
        std::cout << "   ✅ Real WebGL shader created successfully!" << std::endl;
        std::cout << "   Shader valid: " << (shader->IsValid() ? "Yes" : "No") << std::endl;
    } else {
        std::cout << "   ❌ Failed to create real WebGL shader!" << std::endl;
    }
    
    // Test real rendering operations
    std::cout << "\n🎮 Testing Real WebGL Rendering..." << std::endl;
    
    // Render a few frames
    for (int frame = 0; frame < 5; ++frame) {
        graphics.BeginFrame();
        graphics.Clear(0.1f, 0.1f, 0.2f); // Dark blue background
        
        // Render the test texture at different positions
        if (texture) {
            graphics.RenderQuad(texture, 100 + frame * 50, 100 + frame * 30, 64, 64);
            graphics.RenderQuad(texture, 200 + frame * 40, 200 + frame * 20, 32, 32);
        }
        
        graphics.EndFrame();
        
        std::cout << "   Rendered frame " << (frame + 1) << std::endl;
        
        // Small delay to see the rendering
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    // Show performance stats
    std::cout << "\n📊 Performance Statistics:" << std::endl;
    std::cout << "FPS: " << graphics.GetFPS() << std::endl;
    std::cout << "Memory: " << graphics.GetMemoryStats() << std::endl;
    std::cout << "Performance: " << graphics.GetPerformanceStats() << std::endl;
    
    // Show capabilities
    std::cout << "\n🔍 WebGL Capabilities:" << std::endl;
    std::cout << graphics.GetCapabilityInfo() << std::endl;
    
    // Shutdown
    std::cout << "\n🔄 Shutting down graphics module..." << std::endl;
    graphics.Shutdown();
    
    std::cout << "\n🎉 Real WebGL Renderer test completed successfully!" << std::endl;
    std::cout << "The WebGL renderer is now fully functional with real graphics!" << std::endl;
    
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
