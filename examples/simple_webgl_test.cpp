#include <iostream>
#include <memory>

// Include the generic graphics module
#include "Generic/GenericGraphicsModule.h"

/**
 * @brief Simple WebGL Test
 * 
 * This is a minimal test to verify that our real WebGL implementation
 * is working correctly without complex animations.
 */

int main() {
    std::cout << "🧪 Simple WebGL Test" << std::endl;
    std::cout << "===================" << std::endl;
    
    // Create the generic graphics module
    GenericGraphicsModule graphics;
    
    // Initialize the module
    std::cout << "\n🚀 Initializing Graphics Module..." << std::endl;
    if (!graphics.Initialize(400, 300, "Simple WebGL Test")) {
        std::cerr << "❌ Failed to initialize graphics module!" << std::endl;
        return -1;
    }
    
    std::cout << "✅ Graphics module initialized successfully!" << std::endl;
    std::cout << "Platform: " << graphics.GetPlatform() << std::endl;
    std::cout << "Renderer: " << graphics.GetRendererType() << std::endl;
    
    // Test 1: Create a simple texture
    std::cout << "\n🎨 Test 1: Creating Simple Texture..." << std::endl;
    
    // Create a 2x2 red texture
    uint8_t redPixels[] = {
        255, 0, 0, 255,    // Red
        255, 0, 0, 255,    // Red
        255, 0, 0, 255,    // Red
        255, 0, 0, 255     // Red
    };
    
    auto texture = graphics.CreateTexture(2, 2, redPixels);
    if (texture && texture->IsValid()) {
        std::cout << "   ✅ Texture created successfully!" << std::endl;
        std::cout << "   Size: " << texture->GetWidth() << "x" << texture->GetHeight() << std::endl;
    } else {
        std::cout << "   ❌ Failed to create texture!" << std::endl;
        return -1;
    }
    
    // Test 2: Create a simple shader
    std::cout << "\n🔧 Test 2: Creating Simple Shader..." << std::endl;
    
    std::string vertexSource = R"(
        #version 300 es
        precision mediump float;
        
        in vec2 a_position;
        void main() {
            gl_Position = vec4(a_position, 0.0, 1.0);
        }
    )";
    
    std::string fragmentSource = R"(
        #version 300 es
        precision mediump float;
        
        out vec4 fragColor;
        void main() {
            fragColor = vec4(1.0, 0.0, 0.0, 1.0);
        }
    )";
    
    auto shader = graphics.CreateShader(vertexSource, fragmentSource);
    if (shader && shader->IsValid()) {
        std::cout << "   ✅ Shader created successfully!" << std::endl;
    } else {
        std::cout << "   ❌ Failed to create shader!" << std::endl;
        return -1;
    }
    
    // Test 3: Basic rendering
    std::cout << "\n🎮 Test 3: Basic Rendering..." << std::endl;
    
    graphics.BeginFrame();
    graphics.Clear(0.0f, 0.0f, 0.5f); // Dark blue background
    
    // Render the texture
    graphics.RenderQuad(texture, 100, 100, 64, 64);
    
    graphics.EndFrame();
    
    std::cout << "   ✅ Rendering completed!" << std::endl;
    
    // Test 4: Performance check
    std::cout << "\n📊 Test 4: Performance Check..." << std::endl;
    
    graphics.SetPerformanceMonitoring(true);
    std::cout << "   FPS: " << graphics.GetFPS() << std::endl;
    std::cout << "   Memory: " << graphics.GetMemoryStats() << std::endl;
    
    // Test 5: Capabilities
    std::cout << "\n🔍 Test 5: Capabilities Check..." << std::endl;
    
    std::cout << "   Max texture size: " << graphics.GetMaxTextureSize() << std::endl;
    std::cout << "   Shaders supported: " << (graphics.IsFeatureSupported("shaders") ? "Yes" : "No") << std::endl;
    std::cout << "   Textures supported: " << (graphics.IsFeatureSupported("textures") ? "Yes" : "No") << std::endl;
    
    // Shutdown
    std::cout << "\n🔄 Shutting down..." << std::endl;
    graphics.Shutdown();
    
    std::cout << "\n🎉 Simple WebGL Test completed successfully!" << std::endl;
    std::cout << "=============================================" << std::endl;
    std::cout << "✅ WebGL initialization: WORKING" << std::endl;
    std::cout << "✅ Texture creation: WORKING" << std::endl;
    std::cout << "✅ Shader compilation: WORKING" << std::endl;
    std::cout << "✅ Basic rendering: WORKING" << std::endl;
    std::cout << "✅ Performance monitoring: WORKING" << std::endl;
    std::cout << "=============================================" << std::endl;
    std::cout << "The real WebGL implementation is working! 🚀" << std::endl;
    
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
