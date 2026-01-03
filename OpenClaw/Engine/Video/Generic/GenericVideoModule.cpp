#include "GenericVideoModule.h"
#include <iostream>

#ifdef __EMSCRIPTEN__
#include "WASM/WasmVideoPlayer.h"
#endif

GenericVideoModule::GenericVideoModule() : m_initialized(false) {
    m_platform = DetectPlatform();
}

GenericVideoModule::~GenericVideoModule() {
    Shutdown();
}

bool GenericVideoModule::Initialize() {
    if (m_initialized) return true;
    
    std::cout << "Initializing GenericVideoModule on " << m_platform << std::endl;
    m_initialized = true;
    return true;
}

void GenericVideoModule::Shutdown() {
    m_initialized = false;
}

std::shared_ptr<IGenericVideo> GenericVideoModule::CreateVideoPlayer() {
    if (!m_initialized) {
        std::cerr << "GenericVideoModule not initialized!" << std::endl;
        return nullptr;
    }

#ifdef __EMSCRIPTEN__
    // Return WASM implementation
    return std::make_shared<WasmVideoPlayer>();
#else
    // Fallback or other platforms
    std::cerr << "Video playback not implemented for this platform yet." << std::endl;
    return nullptr; 
#endif
}

std::string GenericVideoModule::GetVersion() {
    return "1.0.0";
}

std::string GenericVideoModule::DetectPlatform() const {
#ifdef __EMSCRIPTEN__
    return "WebAssembly";
#elif defined(_WIN32)
    return "Windows";
#elif defined(__linux__)
    return "Linux";
#elif defined(__APPLE__)
    return "macOS";
#elif defined(__ANDROID__)
    return "Android";
#else
    return "Unknown";
#endif
}
