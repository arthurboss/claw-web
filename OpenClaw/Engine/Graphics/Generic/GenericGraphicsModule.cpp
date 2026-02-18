#include "GenericGraphicsModule.h"
#include <iostream>
#include <algorithm>

// Platform-specific includes
#ifdef __EMSCRIPTEN__
#include "WASM/PureWebGLRenderer.h"
#endif

// Future platform-specific includes will go here

// ===== Constructor & Destructor =====

GenericGraphicsModule::GenericGraphicsModule()
    : m_platform("Unknown")
    , m_rendererType("Unknown")
    , m_preferredRenderer("")
    , m_renderer(nullptr)
    , m_initialized(false)
    , m_performanceMonitoring(false)
    , m_defaultTextureFiltering("linear")
    , m_defaultBlending("alpha")
    , m_maxTextureCacheSize(256) // 256MB default
{
    // Auto-detect platform
    m_platform = DetectPlatform();
    
    // Get available renderers for this platform
    m_availableRenderers = GetPlatformRenderers();
    
    // Set default preferred renderer
    if (!m_availableRenderers.empty()) {
        m_preferredRenderer = m_availableRenderers[0];
    }
    
    std::cout << "GenericGraphicsModule created for platform: " << m_platform << std::endl;
    std::cout << "Available renderers: ";
    for (const auto& renderer : m_availableRenderers) {
        std::cout << renderer << " ";
    }
    std::cout << std::endl;
}

GenericGraphicsModule::~GenericGraphicsModule() {
    Shutdown();
}

// ===== Core Module Interface =====

bool GenericGraphicsModule::Initialize(int width, int height, const std::string& title) {
    if (m_initialized) {
        std::cout << "GenericGraphicsModule already initialized!" << std::endl;
        return true;
    }
    
    std::cout << "Initializing GenericGraphicsModule: " << width << "x" << height << std::endl;
    std::cout << "Platform: " << m_platform << std::endl;
    std::cout << "Preferred renderer: " << m_preferredRenderer << std::endl;
    
    // Create the appropriate renderer
    m_renderer = CreateRenderer(m_preferredRenderer);
    if (!m_renderer) {
        std::cerr << "Failed to create renderer: " << m_preferredRenderer << std::endl;
        
        // Try fallback renderers
        for (const auto& rendererName : m_availableRenderers) {
            if (rendererName != m_preferredRenderer) {
                std::cout << "Trying fallback renderer: " << rendererName << std::endl;
                m_renderer = CreateRenderer(rendererName);
                if (m_renderer) {
                    m_rendererType = rendererName;
                    std::cout << "Fallback renderer created successfully: " << rendererName << std::endl;
                    break;
                }
            }
        }
        
        if (!m_renderer) {
            std::cerr << "Failed to create any renderer!" << std::endl;
            return false;
        }
    } else {
        m_rendererType = m_preferredRenderer;
    }
    
    // Initialize the renderer
    if (!m_renderer->Initialize(width, height, title)) {
        std::cerr << "Failed to initialize renderer!" << std::endl;
        return false;
    }
    
    // Initialize platform-specific renderers for fallback
    if (m_platform == "WASM") {
        if (!InitializeWASMRenderers(width, height)) {
            std::cout << "Warning: Failed to initialize WASM fallback renderers" << std::endl;
        }
    }
    
    m_initialized = true;
    std::cout << "GenericGraphicsModule initialized successfully!" << std::endl;
    std::cout << "Active renderer: " << m_rendererType << std::endl;
    
    return true;
}

void GenericGraphicsModule::Shutdown() {
    if (!m_initialized) return;
    
    std::cout << "Shutting down GenericGraphicsModule..." << std::endl;
    
    // Shutdown main renderer
    if (m_renderer) {
        m_renderer->Shutdown();
        m_renderer.reset();
    }
    
    // Shutdown platform-specific renderers
    if (m_platform == "WASM") {
        m_wasmWebGLRenderer.reset();
    }
    
    m_initialized = false;
    std::cout << "GenericGraphicsModule shutdown complete!" << std::endl;
}

bool GenericGraphicsModule::IsInitialized() const {
    return m_initialized && m_renderer && m_renderer->IsInitialized();
}

// ===== Platform & Renderer Information =====

std::string GenericGraphicsModule::GetPlatform() const {
    return m_platform;
}

std::string GenericGraphicsModule::GetRendererType() const {
    return m_rendererType;
}

IGenericRenderer* GenericGraphicsModule::GetRenderer() const {
    return m_renderer.get();
}

std::vector<std::string> GenericGraphicsModule::GetAvailableRenderers() const {
    return m_availableRenderers;
}

// ===== Module Information =====

std::string GenericGraphicsModule::GetVersion() {
    return "1.0.0";
}

std::string GenericGraphicsModule::GetBuildInfo() {
    std::ostringstream oss;
    oss << "GenericGraphicsModule v" << GetVersion() << "\n";
    oss << "Build Date: " << __DATE__ << " " << __TIME__ << "\n";
    oss << "Compiler: " << 
#ifdef __EMSCRIPTEN__
        "Emscripten"
#elif defined(__GNUC__)
        "GCC " << __GNUC__ << "." << __GNUC_MINOR__ << "." << __GNUC_PATCHLEVEL__
#elif defined(_MSC_VER)
        "MSVC " << _MSC_VER
#else
        "Unknown"
#endif
        << "\n";
    oss << "Platform: " << 
#ifdef __EMSCRIPTEN__
        "WebAssembly"
#elif defined(__APPLE__)
        "macOS"
#elif defined(_WIN32)
        "Windows"
#elif defined(__linux__)
        "Linux"
#elif defined(__ANDROID__)
        "Android"
#else
        "Unknown"
#endif
        << "\n";
    return oss.str();
}

// ===== Feature Detection & Capabilities =====

bool GenericGraphicsModule::IsFeatureSupported(const std::string& feature) const {
    if (!m_renderer) return false;
    return m_renderer->IsFeatureSupported(feature);
}

int GenericGraphicsModule::GetMaxTextureSize() const {
    if (!m_renderer) return 0;
    return m_renderer->GetMaxTextureSize();
}

std::string GenericGraphicsModule::GetCapabilityInfo() const {
    if (!m_renderer) return "No renderer available";
    
    std::ostringstream oss;
    oss << "Platform: " << m_platform << "\n";
    oss << "Renderer: " << m_rendererType << "\n";
    oss << "Max Texture Size: " << GetMaxTextureSize() << "x" << GetMaxTextureSize() << "\n";
    oss << "Features:\n";
    
    std::vector<std::string> features = {"shaders", "textures", "particles", "post_processing"};
    for (const auto& feature : features) {
        bool supported = IsFeatureSupported(feature);
        oss << "  " << (supported ? "✅" : "❌") << " " << feature << "\n";
    }
    
    return oss.str();
}

// ===== Configuration & Customization =====

bool GenericGraphicsModule::SetPreferredRenderer(const std::string& rendererType) {
    // Check if the renderer type is available
    auto it = std::find(m_availableRenderers.begin(), m_availableRenderers.end(), rendererType);
    if (it == m_availableRenderers.end()) {
        std::cerr << "Renderer type not available: " << rendererType << std::endl;
        return false;
    }
    
    m_preferredRenderer = rendererType;
    std::cout << "Preferred renderer set to: " << rendererType << std::endl;
    return true;
}

void GenericGraphicsModule::SetFeatureEnabled(const std::string& feature, bool enabled) {
    // This is a placeholder - in a full implementation, you might
    // enable/disable specific features at runtime
    std::cout << "Feature '" << feature << "' " << (enabled ? "enabled" : "disabled") << std::endl;
}

bool GenericGraphicsModule::SetConfiguration(const std::string& config) {
    // Parse configuration string in key=value format
    std::istringstream iss(config);
    std::string line;
    bool success = true;
    
    while (std::getline(iss, line)) {
        size_t pos = line.find('=');
        if (pos == std::string::npos) continue;
        
        std::string key = line.substr(0, pos);
        std::string value = line.substr(pos + 1);
        
        // Trim whitespace
        key.erase(0, key.find_first_not_of(" \t"));
        key.erase(key.find_last_not_of(" \t") + 1);
        value.erase(0, value.find_first_not_of(" \t"));
        value.erase(value.find_last_not_of(" \t") + 1);
        
        if (key == "renderer") {
            if (!SetPreferredRenderer(value)) {
                success = false;
            }
        } else if (key == "texture_filtering") {
            SetDefaultTextureFiltering(value);
        } else if (key == "blending") {
            SetDefaultBlending(value);
        } else if (key == "max_texture_cache") {
            try {
                size_t size = std::stoul(value);
                SetMaxTextureCacheSize(size);
            } catch (const std::exception&) {
                std::cerr << "Invalid texture cache size: " << value << std::endl;
                success = false;
            }
        } else {
            std::cerr << "Unknown configuration key: " << key << std::endl;
            success = false;
        }
    }
    
    return success;
}

std::string GenericGraphicsModule::GetConfiguration() const {
    std::ostringstream oss;
    oss << "GenericGraphicsModule Configuration:\n";
    oss << "  Platform: " << m_platform << "\n";
    oss << "  Preferred Renderer: " << m_preferredRenderer << "\n";
    oss << "  Active Renderer: " << m_rendererType << "\n";
    oss << "  Available Renderers: ";
    for (const auto& renderer : m_availableRenderers) {
        oss << renderer << " ";
    }
    oss << "\n";
    oss << "  Performance Monitoring: " << (m_performanceMonitoring ? "Enabled" : "Disabled") << "\n";
    oss << "  Default Texture Filtering: " << m_defaultTextureFiltering << "\n";
    oss << "  Default Blending: " << m_defaultBlending << "\n";
    oss << "  Max Texture Cache: " << m_maxTextureCacheSize << "MB\n";
    
    return oss.str();
}

void GenericGraphicsModule::SetDefaultTextureFiltering(const std::string& filtering) {
    if (filtering == "linear" || filtering == "nearest") {
        m_defaultTextureFiltering = filtering;
        std::cout << "Default texture filtering set to: " << filtering << std::endl;
    } else {
        std::cerr << "Invalid texture filtering mode: " << filtering << std::endl;
    }
}

void GenericGraphicsModule::SetDefaultBlending(const std::string& blending) {
    if (blending == "alpha" || blending == "additive" || blending == "multiply") {
        m_defaultBlending = blending;
        std::cout << "Default blending set to: " << blending << std::endl;
    } else {
        std::cerr << "Invalid blending mode: " << blending << std::endl;
    }
}

void GenericGraphicsModule::SetMaxTextureCacheSize(size_t sizeMB) {
    m_maxTextureCacheSize = sizeMB;
    std::cout << "Max texture cache size set to: " << sizeMB << "MB" << std::endl;
}

// ===== Performance & Monitoring =====

void GenericGraphicsModule::SetPerformanceMonitoring(bool enabled) {
    m_performanceMonitoring = enabled;
    if (m_renderer) {
        m_renderer->SetPerformanceMonitoring(enabled);
    }
}

float GenericGraphicsModule::GetFPS() const {
    if (!m_renderer) return 0.0f;
    return m_renderer->GetFPS();
}

std::string GenericGraphicsModule::GetMemoryStats() const {
    if (!m_renderer) return "No renderer available";
    return m_renderer->GetMemoryStats();
}

std::string GenericGraphicsModule::GetPerformanceStats() const {
    if (!m_renderer) return "No renderer available";
    return m_renderer->GetPerformanceStats();
}

// ===== Convenience Methods (Delegate to current renderer) =====

void GenericGraphicsModule::BeginFrame() {
    if (m_renderer) m_renderer->BeginFrame();
}

void GenericGraphicsModule::EndFrame() {
    if (m_renderer) m_renderer->EndFrame();
}

void GenericGraphicsModule::Clear(float r, float g, float b, float a) {
    if (m_renderer) m_renderer->Clear(r, g, b, a);
}

void GenericGraphicsModule::SetViewport(int x, int y, int width, int height) {
    if (m_renderer) m_renderer->SetViewport(x, y, width, height);
}

std::shared_ptr<ITexture> GenericGraphicsModule::CreateTexture(int width, int height, const uint8_t* data) {
    if (!m_renderer) return nullptr;
    return m_renderer->CreateTexture(width, height, data);
}

std::shared_ptr<ITexture> GenericGraphicsModule::LoadTexture(const std::string& filepath) {
    if (!m_renderer) return nullptr;
    return m_renderer->LoadTexture(filepath);
}

std::shared_ptr<IShader> GenericGraphicsModule::CreateShader(const std::string& vertexSource, const std::string& fragmentSource) {
    if (!m_renderer) return nullptr;
    return m_renderer->CreateShader(vertexSource, fragmentSource);
}

void GenericGraphicsModule::RenderSprite(const std::shared_ptr<ISprite>& sprite, float x, float y, float scale, float rotation) {
    if (m_renderer) m_renderer->RenderSprite(sprite, x, y, scale, rotation);
}

void GenericGraphicsModule::RenderQuad(const std::shared_ptr<ITexture>& texture, float x, float y, float width, float height) {
    if (m_renderer) m_renderer->RenderQuad(texture, x, y, width, height);
}

void GenericGraphicsModule::RenderText(const std::string& text, float x, float y, uint32_t color, int fontSize) {
    if (m_renderer) m_renderer->RenderText(text, x, y, color, fontSize);
}

void GenericGraphicsModule::SetPostProcessEffect(const std::string& effect, bool enabled) {
    if (m_renderer) m_renderer->SetPostProcessEffect(effect, enabled);
}

void GenericGraphicsModule::SetPostProcessParameter(const std::string& effect, const std::string& param, float value) {
    if (m_renderer) m_renderer->SetPostProcessParameter(effect, param, value);
}

std::shared_ptr<IParticleSystem> GenericGraphicsModule::CreateParticleSystem(size_t maxParticles) {
    if (!m_renderer) return nullptr;
    return m_renderer->CreateParticleSystem(maxParticles);
}

// ===== Private Implementation Details =====

std::string GenericGraphicsModule::DetectPlatform() const {
#ifdef __EMSCRIPTEN__
    return "WASM";
#elif defined(_WIN32)
    return "Windows";
#elif defined(__linux__)
    return "Linux";
#elif defined(__ANDROID__)
    return "Android";
#elif defined(__APPLE__)
    return "macOS";
#else
    return "Unknown";
#endif
}

std::vector<std::string> GenericGraphicsModule::GetPlatformRenderers() const {
    std::vector<std::string> renderers;
    
    if (m_platform == "WASM") {
        renderers = {"WebGL"};
    } else if (m_platform == "Windows" || m_platform == "Linux") {
        renderers = {"OpenGL", "Vulkan"};
    } else if (m_platform == "Android") {
        renderers = {"OpenGL ES"};
    } else if (m_platform == "macOS") {
        renderers = {"OpenGL", "Metal"};
    }
    
    return renderers;
}

std::unique_ptr<IGenericRenderer> GenericGraphicsModule::CreateRenderer(const std::string& rendererType) {
    std::cout << "Creating renderer: " << rendererType << std::endl;
    
    if (m_platform == "WASM") {
        if (rendererType == "WebGL") {
            return std::unique_ptr<IGenericRenderer>(new GenericWebGLRenderer());
        }
    }
    
    // For other platforms, return nullptr (not implemented yet)
    std::cerr << "Renderer type not implemented for platform " << m_platform << ": " << rendererType << std::endl;
    return nullptr;
}

bool GenericGraphicsModule::InitializeWASMRenderers(int width, int height) {
    std::cout << "Initializing WASM renderers..." << std::endl;

    // Initialize WebGL renderer
    m_wasmWebGLRenderer = std::unique_ptr<GenericWebGLRenderer>(new GenericWebGLRenderer());
    if (!m_wasmWebGLRenderer->Initialize(width, height)) {
        std::cerr << "Failed to initialize WASM WebGL renderer" << std::endl;
        return false;
    }

    std::cout << "WASM renderers initialized successfully" << std::endl;
    return true;
}

bool GenericGraphicsModule::InitializeDesktopRenderers(int width, int height, const std::string& title) {
    // This will be implemented when we add desktop platform support
    std::cout << "Desktop renderers not yet implemented" << std::endl;
    return false;
}

bool GenericGraphicsModule::InitializeMobileRenderers(int width, int height) {
    // This will be implemented when we add mobile platform support
    std::cout << "Mobile renderers not yet implemented" << std::endl;
    return false;
}
