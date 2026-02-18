#pragma once

#include "../IGenericRenderer.h"
#include <GLES3/gl3.h>
#include <string>
#include <memory>
#include <unordered_map>
#include <iostream>

// Forward declarations
class PureWebGLSprite;

// Real WebGL texture implementation
class PureWebGLTexture : public ITexture {
public:
    PureWebGLTexture(int width, int height, GLuint textureId = 0) 
        : m_width(width), m_height(height), m_textureId(textureId), m_valid(textureId != 0) {}
    
    ~PureWebGLTexture() {
        if (m_textureId != 0) {
            glDeleteTextures(1, &m_textureId);
        }
    }
    
    int GetWidth() const override { return m_width; }
    int GetHeight() const override { return m_height; }
    bool IsValid() const override { return m_valid && m_textureId != 0; }
    
    void Bind(int unit = 0) override {
        if (IsValid()) {
            glActiveTexture(GL_TEXTURE0 + unit);
            glBindTexture(GL_TEXTURE_2D, m_textureId);
        }
    }
    
    void Unbind() override {
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    
    GLuint GetTextureId() const { return m_textureId; }
    
private:
    int m_width, m_height;
    GLuint m_textureId;
    bool m_valid;
};

class PureWebGLShader : public IShader {
public:
    PureWebGLShader(GLuint programId = 0) : m_programId(programId), m_valid(programId != 0) {}
    
    ~PureWebGLShader() {
        if (m_programId != 0) {
            glDeleteProgram(m_programId);
        }
    }
    
    bool IsValid() const override { return m_valid && m_programId != 0; }
    
    void Use() override {
        if (IsValid()) {
            glUseProgram(m_programId);
        }
    }
    
    void SetUniform(const std::string& name, float value) override {
        if (IsValid()) {
            GLint location = glGetUniformLocation(m_programId, name.c_str());
            if (location != -1) {
                glUniform1f(location, value);
            }
        }
    }
    
    void SetUniform(const std::string& name, int value) override {
        if (IsValid()) {
            GLint location = glGetUniformLocation(m_programId, name.c_str());
            if (location != -1) {
                glUniform1i(location, value);
            }
        }
    }
    
    void SetUniform(const std::string& name, float x, float y) override {
        if (IsValid()) {
            GLint location = glGetUniformLocation(m_programId, name.c_str());
            if (location != -1) {
                glUniform2f(location, x, y);
            }
        }
    }
    
    GLuint GetProgramId() const { return m_programId; }
    
private:
    GLuint m_programId;
    bool m_valid;
};

class PureWebGLParticleSystem : public IParticleSystem {
public:
    PureWebGLParticleSystem(size_t maxParticles) : m_maxParticles(maxParticles), m_initialized(false) {}
    
    bool Initialize() override { m_initialized = true; return true; }
    void Shutdown() override { m_initialized = false; }
    void Update(float deltaTime) override { std::cout << "Updating particle system, dt=" << deltaTime << std::endl; }
    void Render() override { std::cout << "Rendering particle system" << std::endl; }
    void AddEmitter(const class ParticleEmitter& emitter) override { std::cout << "Adding particle emitter" << std::endl; }
    void RemoveEmitter(size_t index) override { std::cout << "Removing emitter " << index << std::endl; }
    size_t GetActiveParticleCount() const override { return 0; }
    
private:
    size_t m_maxParticles;
    bool m_initialized;
};

/**
 * @brief Pure WebGL renderer for WASM builds (no SDL dependencies)
 * 
 * This renderer provides WebGL graphics functionality directly through
 * the browser's WebGL API, without requiring any external libraries.
 */
class GenericWebGLRenderer : public IGenericRenderer {
public:
    GenericWebGLRenderer();
    ~GenericWebGLRenderer() override;

    // ===== Core Initialization & Lifecycle =====
    bool Initialize(int width, int height, const std::string& title = "") override;
    void Shutdown() override;
    bool IsInitialized() const override;

    // ===== Platform & Capability Detection =====
    std::string GetRendererType() const override;
    std::string GetPlatform() const override;
    bool IsFeatureSupported(const std::string& feature) const override;
    int GetMaxTextureSize() const override;

    // ===== Rendering & Display =====
    void BeginFrame() override;
    void EndFrame() override;
    void Clear(float r, float g, float b, float a = 1.0f) override;
    void SetViewport(int x, int y, int width, int height) override;

    // ===== Resource Management =====
    std::shared_ptr<ITexture> CreateTexture(int width, int height, const uint8_t* data) override;
    std::shared_ptr<ITexture> LoadTexture(const std::string& filepath) override;
    std::shared_ptr<IShader> CreateShader(const std::string& vertexSource, const std::string& fragmentSource) override;

    // ===== Rendering Operations =====
    void RenderSprite(const std::shared_ptr<ISprite>& sprite, float x, float y, float scale = 1.0f, float rotation = 0.0f) override;
    void RenderQuad(const std::shared_ptr<ITexture>& texture, float x, float y, float width, float height) override;
    void RenderText(const std::string& text, float x, float y, uint32_t color = 0xFFFFFFFF, int fontSize = 16) override;

    // ===== Performance & Monitoring =====
    float GetFPS() const override;
    std::string GetMemoryStats() const override;
    std::string GetPerformanceStats() const override;
    void SetPerformanceMonitoring(bool enabled) override;

    // ===== Advanced Features =====
    void SetPostProcessEffect(const std::string& effect, bool enabled) override;
    void SetPostProcessParameter(const std::string& effect, const std::string& param, float value) override;
    std::shared_ptr<IParticleSystem> CreateParticleSystem(size_t maxParticles) override;

private:
    // ===== Private Implementation Details =====
    bool InitializeWebGL();
    bool CreateDefaultShaders();
    bool CreateScreenQuad();
    void DestroyScreenQuad();
    void UpdateFPS();
    
    // ===== WebGL Helper Methods =====
    GLuint CreateWebGLTexture(int width, int height, const uint8_t* data);
    GLuint CompileShader(GLenum type, const std::string& source);
    GLuint CreateShaderProgram(const std::string& vertexSource, const std::string& fragmentSource);
    bool CheckGLError(const std::string& operation);
    void SetupTextureParameters(GLuint textureId);
    
    // ===== Member Variables =====
    bool m_initialized;
    int m_screenWidth;
    int m_screenHeight;
    
    // WebGL objects
    GLuint m_defaultProgram;
    GLuint m_screenQuadVAO;
    GLuint m_screenQuadVBO;
    
    // Performance monitoring
    bool m_performanceMonitoring;
    float m_fps;
    int m_frameCount;
    double m_lastFPSUpdate;
    
    // Feature support
    std::unordered_map<std::string, bool> m_featureSupport;
    int m_maxTextureSize;
    
    // Post-processing
    std::unordered_map<std::string, bool> m_enabledEffects;
    std::unordered_map<std::string, std::unordered_map<std::string, float>> m_effectParams;
};
