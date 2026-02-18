#include "PureWebGLRenderer.h"
#include <iostream>
#include <sstream>
#include <chrono>
#include <vector>

// ===== Constructor & Destructor =====

GenericWebGLRenderer::GenericWebGLRenderer()
    : m_initialized(false)
    , m_screenWidth(0)
    , m_screenHeight(0)
    , m_defaultProgram(0)
    , m_screenQuadVAO(0)
    , m_screenQuadVBO(0)
    , m_performanceMonitoring(false)
    , m_fps(0.0f)
    , m_frameCount(0)
    , m_lastFPSUpdate(0.0)
    , m_maxTextureSize(2048)
{
    // Initialize feature support map
    m_featureSupport = {
        {"shaders", true},
        {"textures", true},
        {"particles", true},
        {"post_processing", true},
        {"webgl", true},
        {"opengl", false},
        {"vulkan", false},
        {"sdl", false}
    };
    
    // Initialize effect parameters
    m_enabledEffects = {
        {"blur", false},
        {"bloom", false},
        {"color_correction", false}
    };
    
    m_effectParams = {
        {"blur", {{"intensity", 1.0f}, {"radius", 5.0f}}},
        {"bloom", {{"intensity", 1.0f}, {"threshold", 0.8f}}},
        {"color_correction", {{"brightness", 1.0f}, {"contrast", 1.0f}, {"saturation", 1.0f}}}
    };
}

GenericWebGLRenderer::~GenericWebGLRenderer() {
    Shutdown();
}

// ===== Core Initialization & Lifecycle =====

bool GenericWebGLRenderer::Initialize(int width, int height, const std::string& title) {
    if (m_initialized) {
        std::cout << "PureWebGLRenderer already initialized!" << std::endl;
        return true;
    }
    
    m_screenWidth = width;
    m_screenHeight = height;
    
    std::cout << "Initializing PureWebGLRenderer: " << width << "x" << height << std::endl;
    
    // Initialize WebGL context
    if (!InitializeWebGL()) {
        std::cerr << "Failed to initialize WebGL context!" << std::endl;
        return false;
    }
    
    // Create default shaders
    if (!CreateDefaultShaders()) {
        std::cerr << "Failed to create default shaders!" << std::endl;
        return false;
    }
    
    // Create screen quad for post-processing
    if (!CreateScreenQuad()) {
        std::cerr << "Failed to create screen quad!" << std::endl;
        return false;
    }
    
    // Get WebGL capabilities
    GLint maxTextureSize;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize);
    m_maxTextureSize = maxTextureSize;
    
    std::cout << "WebGL initialized successfully!" << std::endl;
    std::cout << "Max texture size: " << m_maxTextureSize << "x" << m_maxTextureSize << std::endl;
    
    m_initialized = true;
    return true;
}

void GenericWebGLRenderer::Shutdown() {
    if (!m_initialized) return;
    
    std::cout << "Shutting down PureWebGLRenderer..." << std::endl;
    
    // Clean up WebGL objects
    if (m_defaultProgram) {
        glDeleteProgram(m_defaultProgram);
        m_defaultProgram = 0;
    }
    
    DestroyScreenQuad();
    
    m_initialized = false;
    std::cout << "PureWebGLRenderer shutdown complete!" << std::endl;
}

bool GenericWebGLRenderer::IsInitialized() const {
    return m_initialized;
}

// ===== Platform & Capability Detection =====

std::string GenericWebGLRenderer::GetRendererType() const {
    return "WebGL";
}

std::string GenericWebGLRenderer::GetPlatform() const {
    return "WASM";
}

bool GenericWebGLRenderer::IsFeatureSupported(const std::string& feature) const {
    auto it = m_featureSupport.find(feature);
    return (it != m_featureSupport.end()) ? it->second : false;
}

int GenericWebGLRenderer::GetMaxTextureSize() const {
    return m_maxTextureSize;
}

// ===== Rendering & Display =====

void GenericWebGLRenderer::BeginFrame() {
    if (!m_initialized) return;
    
    // Clear the screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // Set viewport
    glViewport(0, 0, m_screenWidth, m_screenHeight);
    
    // Use default shader
    glUseProgram(m_defaultProgram);
}

void GenericWebGLRenderer::EndFrame() {
    if (!m_initialized) return;
    
    // Update FPS counter
    UpdateFPS();
    
    // Present the frame (in WASM, this happens automatically)
    // In a real implementation, you might swap buffers here
}

void GenericWebGLRenderer::Clear(float r, float g, float b, float a) {
    if (!m_initialized) return;
    
    glClearColor(r, g, b, a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void GenericWebGLRenderer::SetViewport(int x, int y, int width, int height) {
    if (!m_initialized) return;
    
    glViewport(x, y, width, height);
}

// ===== Resource Management =====

std::shared_ptr<ITexture> GenericWebGLRenderer::CreateTexture(int width, int height, const uint8_t* data) {
    if (!m_initialized) return nullptr;
    
    std::cout << "Creating WebGL texture: " << width << "x" << height << std::endl;
    
    // Create actual WebGL texture
    GLuint textureId = CreateWebGLTexture(width, height, data);
    if (textureId == 0) {
        std::cerr << "Failed to create WebGL texture!" << std::endl;
        return nullptr;
    }
    
    return std::shared_ptr<ITexture>(new PureWebGLTexture(width, height, textureId));
}

std::shared_ptr<ITexture> GenericWebGLRenderer::LoadTexture(const std::string& filepath) {
    if (!m_initialized) return nullptr;
    
    std::cout << "Loading texture: " << filepath << std::endl;
    
    // For now, return a placeholder texture
    // In a full implementation, this would load and create an actual WebGL texture
    return std::make_shared<class PureWebGLTexture>(256, 256); // Default size
}

std::shared_ptr<IShader> GenericWebGLRenderer::CreateShader(const std::string& vertexSource, const std::string& fragmentSource) {
    if (!m_initialized) return nullptr;
    
    std::cout << "Creating WebGL shader program" << std::endl;
    
    // Create actual WebGL shader program
    GLuint programId = CreateShaderProgram(vertexSource, fragmentSource);
    if (programId == 0) {
        std::cerr << "Failed to create WebGL shader program!" << std::endl;
        return nullptr;
    }
    
    return std::shared_ptr<IShader>(new PureWebGLShader(programId));
}

// ===== Rendering Operations =====

void GenericWebGLRenderer::RenderSprite(const std::shared_ptr<ISprite>& sprite, float x, float y, float scale, float rotation) {
    if (!m_initialized || !sprite) return;
    
    // For now, just log the render call
    // In a full implementation, this would render the sprite using WebGL
    std::cout << "Rendering sprite at (" << x << ", " << y << ") scale: " << scale << " rotation: " << rotation << std::endl;
}

void GenericWebGLRenderer::RenderQuad(const std::shared_ptr<ITexture>& texture, float x, float y, float width, float height) {
    if (!m_initialized || !texture) return;
    
    std::cout << "Rendering WebGL quad with texture at (" << x << ", " << y << ") size " << width << "x" << height << std::endl;
    
    // Use the default shader program
    glUseProgram(m_defaultProgram);
    
    // Bind the texture
    if (auto webglTexture = std::dynamic_pointer_cast<PureWebGLTexture>(texture)) {
        webglTexture->Bind(0);
    }
    
    // Set up orthographic projection matrix
    // Convert screen coordinates to normalized device coordinates
    float left = (x / m_screenWidth) * 2.0f - 1.0f;
    float right = ((x + width) / m_screenWidth) * 2.0f - 1.0f;
    float bottom = 1.0f - ((y + height) / m_screenHeight) * 2.0f;
    float top = 1.0f - (y / m_screenHeight) * 2.0f;
    
    // Define quad vertices in normalized device coordinates
    float vertices[] = {
        // Position    // Texture Coords
        left,  bottom,  0.0f, 1.0f,  // Bottom-left
        right, bottom,  1.0f, 1.0f,  // Bottom-right
        right, top,     1.0f, 0.0f,  // Top-right
        left,  top,     0.0f, 0.0f   // Top-left
    };
    
    // Create temporary VAO and VBO for this quad
    GLuint tempVAO, tempVBO;
    glGenVertexArrays(1, &tempVAO);
    glGenBuffers(1, &tempVBO);
    
    glBindVertexArray(tempVAO);
    glBindBuffer(GL_ARRAY_BUFFER, tempVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    // Set up vertex attributes
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    // Draw the quad
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    
    // Clean up temporary resources
    glDeleteVertexArrays(1, &tempVAO);
    glDeleteBuffers(1, &tempVBO);
    
    // Unbind texture
    if (auto webglTexture = std::dynamic_pointer_cast<PureWebGLTexture>(texture)) {
        webglTexture->Unbind();
    }
    
    CheckGLError("RenderQuad");
}

void GenericWebGLRenderer::RenderText(const std::string& text, float x, float y, uint32_t color, int fontSize) {
    if (!m_initialized) return;
    
    // For now, just log the render call
    // In a full implementation, this would render text using WebGL
    std::cout << "Rendering text: \"" << text << "\" at (" << x << ", " << y << ") size: " << fontSize << std::endl;
}

// ===== Performance & Monitoring =====

float GenericWebGLRenderer::GetFPS() const {
    return m_fps;
}

std::string GenericWebGLRenderer::GetMemoryStats() const {
    if (!m_initialized) return "Not initialized";
    
    // In WASM, we can't easily get detailed memory stats
    // This is a placeholder implementation
    std::ostringstream oss;
    oss << "WASM Memory: ~" << (m_screenWidth * m_screenHeight * 4 / 1024) << "KB (estimated)";
    return oss.str();
}

std::string GenericWebGLRenderer::GetPerformanceStats() const {
    if (!m_initialized) return "Not initialized";
    
    std::ostringstream oss;
    oss << "FPS: " << m_fps << "\n";
    oss << "Frames: " << m_frameCount << "\n";
    oss << "Resolution: " << m_screenWidth << "x" << m_screenHeight << "\n";
    oss << "Renderer: " << GetRendererType() << "\n";
    oss << "Platform: " << GetPlatform();
    
    return oss.str();
}

void GenericWebGLRenderer::SetPerformanceMonitoring(bool enabled) {
    m_performanceMonitoring = enabled;
    std::cout << "Performance monitoring " << (enabled ? "enabled" : "disabled") << std::endl;
}

// ===== Advanced Features =====

void GenericWebGLRenderer::SetPostProcessEffect(const std::string& effect, bool enabled) {
    auto it = m_enabledEffects.find(effect);
    if (it != m_enabledEffects.end()) {
        it->second = enabled;
        std::cout << "Post-process effect '" << effect << "' " << (enabled ? "enabled" : "disabled") << std::endl;
    }
}

void GenericWebGLRenderer::SetPostProcessParameter(const std::string& effect, const std::string& param, float value) {
    auto effectIt = m_effectParams.find(effect);
    if (effectIt != m_effectParams.end()) {
        auto paramIt = effectIt->second.find(param);
        if (paramIt != effectIt->second.end()) {
            paramIt->second = value;
            std::cout << "Set '" << effect << "." << param << "' = " << value << std::endl;
        }
    }
}

std::shared_ptr<IParticleSystem> GenericWebGLRenderer::CreateParticleSystem(size_t maxParticles) {
    if (!m_initialized) return nullptr;
    
    std::cout << "Creating particle system with " << maxParticles << " particles" << std::endl;
    
    // For now, return a placeholder particle system
    // In a full implementation, this would create an actual WebGL-based particle system
    return std::make_shared<class PureWebGLParticleSystem>(maxParticles);
}

// ===== Private Implementation Details =====

bool GenericWebGLRenderer::InitializeWebGL() {
    std::cout << "Initializing WebGL context..." << std::endl;
    
    // In WASM, the WebGL context should already be available
    // We just need to verify it's working
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr << "WebGL context not available or error: " << error << std::endl;
        return false;
    }
    
    // Get WebGL version info
    const char* version = reinterpret_cast<const char*>(glGetString(GL_VERSION));
    const char* renderer = reinterpret_cast<const char*>(glGetString(GL_RENDERER));
    const char* vendor = reinterpret_cast<const char*>(glGetString(GL_VENDOR));
    
    std::cout << "WebGL Version: " << (version ? version : "Unknown") << std::endl;
    std::cout << "WebGL Renderer: " << (renderer ? renderer : "Unknown") << std::endl;
    std::cout << "WebGL Vendor: " << (vendor ? vendor : "Unknown") << std::endl;
    
    // Enable depth testing
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    
    // Enable blending for transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Set clear color
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    
    std::cout << "WebGL context initialized successfully!" << std::endl;
    return true;
}

bool GenericWebGLRenderer::CreateDefaultShaders() {
    std::cout << "Creating default WebGL shaders..." << std::endl;
    
    // Vertex shader source (simple pass-through)
    const std::string vertexSource = R"(
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
    
    // Fragment shader source (simple color output)
    const std::string fragmentSource = R"(
        #version 300 es
        precision mediump float;
        
        in vec2 v_texCoord;
        out vec4 fragColor;
        
        void main() {
            fragColor = vec4(1.0, 1.0, 1.0, 1.0);
        }
    )";
    
    // Create actual shader program
    m_defaultProgram = CreateShaderProgram(vertexSource, fragmentSource);
    if (m_defaultProgram == 0) {
        std::cerr << "Failed to create default shader program!" << std::endl;
        return false;
    }
    
    std::cout << "Default WebGL shaders created successfully!" << std::endl;
    return true;
}

bool GenericWebGLRenderer::CreateScreenQuad() {
    std::cout << "Creating WebGL screen quad..." << std::endl;
    
    // Create VAO
    glGenVertexArrays(1, &m_screenQuadVAO);
    glBindVertexArray(m_screenQuadVAO);
    
    // Create VBO
    glGenBuffers(1, &m_screenQuadVBO);
    glBindBuffer(GL_ARRAY_BUFFER, m_screenQuadVBO);
    
    // Define screen quad vertices (position + texture coordinates)
    // Full screen quad: (-1,-1) to (1,1)
    float vertices[] = {
        // Position    // Texture Coords
        -1.0f, -1.0f,  0.0f, 0.0f,  // Bottom-left
         1.0f, -1.0f,  1.0f, 0.0f,  // Bottom-right
         1.0f,  1.0f,  1.0f, 1.0f,  // Top-right
        -1.0f,  1.0f,  0.0f, 1.0f   // Top-left
    };
    
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    // Set up vertex attributes
    // Position attribute
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // Texture coordinate attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    // Unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    
    if (!CheckGLError("CreateScreenQuad")) {
        return false;
    }
    
    std::cout << "WebGL screen quad created successfully!" << std::endl;
    return true;
}

void GenericWebGLRenderer::DestroyScreenQuad() {
    // Clean up screen quad resources
    if (m_screenQuadVAO) {
        glDeleteVertexArrays(1, &m_screenQuadVAO);
        m_screenQuadVAO = 0;
    }
    
    if (m_screenQuadVBO) {
        glDeleteBuffers(1, &m_screenQuadVBO);
        m_screenQuadVBO = 0;
    }
}

void GenericWebGLRenderer::UpdateFPS() {
    if (!m_performanceMonitoring) return;
    
    m_frameCount++;
    
    auto now = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());
    double currentTime = duration.count() / 1000.0;
    
    // Update FPS every second
    if (currentTime - m_lastFPSUpdate >= 1.0) {
        m_fps = m_frameCount / (currentTime - m_lastFPSUpdate);
        m_frameCount = 0;
        m_lastFPSUpdate = currentTime;
    }
}

// ===== WebGL Helper Methods Implementation =====

GLuint GenericWebGLRenderer::CreateWebGLTexture(int width, int height, const uint8_t* data) {
    GLuint textureId;
    glGenTextures(1, &textureId);
    
    if (textureId == 0) {
        std::cerr << "Failed to generate texture ID" << std::endl;
        return 0;
    }
    
    glBindTexture(GL_TEXTURE_2D, textureId);
    
    // Set texture parameters
    SetupTextureParameters(textureId);
    
    // Upload texture data
    if (data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    } else {
        // Create empty texture
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    }
    
    if (!CheckGLError("CreateWebGLTexture")) {
        glDeleteTextures(1, &textureId);
        return 0;
    }
    
    glBindTexture(GL_TEXTURE_2D, 0);
    return textureId;
}

GLuint GenericWebGLRenderer::CompileShader(GLenum type, const std::string& source) {
    GLuint shader = glCreateShader(type);
    if (shader == 0) {
        std::cerr << "Failed to create shader" << std::endl;
        return 0;
    }
    
    const char* sourceCStr = source.c_str();
    glShaderSource(shader, 1, &sourceCStr, nullptr);
    glCompileShader(shader);
    
    // Check compilation status
    GLint compiled;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
        GLint infoLen = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
        if (infoLen > 1) {
            std::vector<char> infoLog(infoLen);
            glGetShaderInfoLog(shader, infoLen, nullptr, &infoLog[0]);
            std::cerr << "Shader compilation error: " << &infoLog[0] << std::endl;
        }
        glDeleteShader(shader);
        return 0;
    }
    
    return shader;
}

GLuint GenericWebGLRenderer::CreateShaderProgram(const std::string& vertexSource, const std::string& fragmentSource) {
    // Compile vertex shader
    GLuint vertexShader = CompileShader(GL_VERTEX_SHADER, vertexSource);
    if (vertexShader == 0) {
        return 0;
    }
    
    // Compile fragment shader
    GLuint fragmentShader = CompileShader(GL_FRAGMENT_SHADER, fragmentSource);
    if (fragmentShader == 0) {
        glDeleteShader(vertexShader);
        return 0;
    }
    
    // Create program
    GLuint program = glCreateProgram();
    if (program == 0) {
        std::cerr << "Failed to create shader program" << std::endl;
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        return 0;
    }
    
    // Attach shaders
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    
    // Link program
    glLinkProgram(program);
    
    // Check linking status
    GLint linked;
    glGetProgramiv(program, GL_LINK_STATUS, &linked);
    if (!linked) {
        GLint infoLen = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLen);
        if (infoLen > 1) {
            std::vector<char> infoLog(infoLen);
            glGetProgramInfoLog(program, infoLen, nullptr, &infoLog[0]);
            std::cerr << "Program linking error: " << &infoLog[0] << std::endl;
        }
        glDeleteProgram(program);
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        return 0;
    }
    
    // Clean up shaders (they're now part of the program)
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    
    return program;
}

bool GenericWebGLRenderer::CheckGLError(const std::string& operation) {
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr << "OpenGL error in " << operation << ": " << error << std::endl;
        return false;
    }
    return true;
}

void GenericWebGLRenderer::SetupTextureParameters(GLuint textureId) {
    // Set texture filtering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    // Set texture wrapping
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}
