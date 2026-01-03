/**
 * @file generic_video_demo.cpp
 * @brief Demonstration of GenericVideoModule with GenericGraphicsModule
 */

#include "OpenClaw/Engine/Graphics/Generic/GenericGraphicsModule.h"
#include "OpenClaw/Engine/Video/Generic/GenericVideoModule.h"
#include <iostream>
#include <vector>
#include <cmath>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

// Global application state
struct AppState {
    GenericGraphicsModule graphics;
    GenericVideoModule video;
    std::shared_ptr<IGenericVideo> videoPlayer;
    bool running = true;
};

AppState app;

void MainLoop() {
    if (!app.running) return;

    app.graphics.BeginFrame();
    app.graphics.Clear(0.05f, 0.05f, 0.1f);

    // Update video state
    if (app.videoPlayer) {
        app.videoPlayer->Update(1.0f / 60.0f);
        
        // Render video frame
        auto frameTexture = app.videoPlayer->GetFrameTexture();
        if (frameTexture && frameTexture->IsValid()) {
            // Full screen video
            app.graphics.RenderQuad(frameTexture, 0, 0, 800, 600);
        } else {
            // Placeholder text or loading spinner could go here
            app.graphics.RenderText("Loading Video / No Frame...", 10, 10, 0xFF0000FF, 20);
        }

        // Render debug info
        std::string timeStr = "Time: " + std::to_string(app.videoPlayer->GetCurrentTime()) + 
                              " / " + std::to_string(app.videoPlayer->GetDuration());
        app.graphics.RenderText(timeStr, 10, 50);
    } else {
         app.graphics.RenderText("Video Capability Not Supported", 10, 10, 0xFF0000FF, 20);
    }

    app.graphics.RenderText("FPS: " + std::to_string(static_cast<int>(app.graphics.GetFPS())), 10, 80);

    app.graphics.EndFrame();
}

int main() {
    std::cout << "=== Generic Video Module Demo ===" << std::endl;

    // 1. Initialize Graphics
    if (!app.graphics.Initialize(800, 600, "Generic Video Demo")) {
        std::cerr << "Failed to initialize graphics!" << std::endl;
        return 1;
    }
    app.graphics.SetPerformanceMonitoring(true);

    // 2. Initialize Video
    if (app.video.Initialize()) {
        std::cout << "Video module initialized." << std::endl;
        app.videoPlayer = app.video.CreateVideoPlayer();
        
        if (app.videoPlayer) {
            // Load a sample video (Big Buck Bunny is a classic test)
            // Note: In a real WASM build, this URL must be CORS-accessible or hosted locally.
            // Using a placeholder or assuming local file 'video.mp4' exists in build dir
            app.videoPlayer->Load("video.mp4");
            app.videoPlayer->Play();
        }
    } else {
        std::cerr << "Failed to initialize video module!" << std::endl;
    }

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(MainLoop, 0, 1);
#else
    while (app.running) {
        MainLoop();
        // Simple loop handling for desktop verification
    }
#endif

    app.video.Shutdown();
    app.graphics.Shutdown();
    return 0;
}
