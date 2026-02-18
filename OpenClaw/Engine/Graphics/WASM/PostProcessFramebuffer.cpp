#include "PostProcessFramebuffer.h"
#include <iostream>

#ifdef __EMSCRIPTEN__

PostProcessFramebuffer::PostProcessFramebuffer() 
    : m_screenQuadVAO(0), m_screenQuadVBO(0), m_initialized(false), m_currentWidth(0), m_currentHeight(0) {
}

PostProcessFramebuffer::~PostProcessFramebuffer() {
    Shutdown();
}

bool PostProcessFramebuffer::Initialize(int width, int height) {
    if (m_initialized) return true;
    
    m_currentWidth = width;
    m_currentHeight = height;
    
    if (!CreateScreenQuad()) {
        std::cerr << "Failed to create screen quad" << std::endl;
        return false;
    }
    
    m_initialized = true;
    return true;
}

void PostProcessFramebuffer::Shutdown() {
    for (auto& fb : m_framebuffers) {
        DestroyFramebuffer(fb);
    }
    m_framebuffers.clear();
    
    DestroyScreenQuad();
    m_initialized = false;
}

bool PostProcessFramebuffer::CreateFramebuffer(int width, int height) {
    Framebuffer fb = {};
    fb.width = width;
    fb.height = height;
    
    // Create framebuffer object
    glGenFramebuffers(1, &fb.fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fb.fbo);
    
    // Create color texture
    glGenTextures(1, &fb.colorTexture);
    glBindTexture(GL_TEXTURE_2D, fb.colorTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    // Create depth renderbuffer
    glGenRenderbuffers(1, &fb.depthRenderbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, fb.depthRenderbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    
    // Attach to framebuffer
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fb.colorTexture, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, fb.depthRenderbuffer);
    
    // Check framebuffer completeness
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Framebuffer not complete" << std::endl;
        DestroyFramebuffer(fb);
        return false;
    }
    
    fb.valid = true;
    m_framebuffers.push_back(fb);
    
    // Restore default framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    return true;
}

void PostProcessFramebuffer::DestroyFramebuffer(Framebuffer& fb) {
    if (fb.valid) {
        if (fb.colorTexture) {
            glDeleteTextures(1, &fb.colorTexture);
            fb.colorTexture = 0;
        }
        if (fb.depthRenderbuffer) {
            glDeleteRenderbuffers(1, &fb.depthRenderbuffer);
            fb.depthRenderbuffer = 0;
        }
        if (fb.fbo) {
            glDeleteFramebuffers(1, &fb.fbo);
            fb.fbo = 0;
        }
        fb.valid = false;
    }
}

void PostProcessFramebuffer::BindFramebuffer(const Framebuffer& fb) {
    if (fb.valid) {
        glBindFramebuffer(GL_FRAMEBUFFER, fb.fbo);
        glViewport(0, 0, fb.width, fb.height);
    }
}

void PostProcessFramebuffer::BindDefault() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, m_currentWidth, m_currentHeight);
}

GLuint PostProcessFramebuffer::GetColorTexture(const Framebuffer& fb) const {
    return fb.valid ? fb.colorTexture : 0;
}

void PostProcessFramebuffer::SetViewport(const Framebuffer& fb) {
    if (fb.valid) {
        glViewport(0, 0, fb.width, fb.height);
    }
}

bool PostProcessFramebuffer::CreateScreenQuad() {
    // Screen quad vertices (NDC coordinates)
    float vertices[] = {
        // positions   // texCoords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
        
        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };
    
    glGenVertexArrays(1, &m_screenQuadVAO);
    glGenBuffers(1, &m_screenQuadVBO);
    
    glBindVertexArray(m_screenQuadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_screenQuadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    // Position attribute
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    
    // TexCoord attribute
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    
    glBindVertexArray(0);
    
    return true;
}

void PostProcessFramebuffer::DestroyScreenQuad() {
    if (m_screenQuadVAO) {
        glDeleteVertexArrays(1, &m_screenQuadVAO);
        m_screenQuadVAO = 0;
    }
    if (m_screenQuadVBO) {
        glDeleteBuffers(1, &m_screenQuadVBO);
        m_screenQuadVBO = 0;
    }
}

const Framebuffer* PostProcessFramebuffer::GetFirstFramebuffer() const {
    if (m_framebuffers.empty()) {
        return nullptr;
    }
    return &m_framebuffers[0];
}

void PostProcessFramebuffer::RenderScreenQuad() {
    glBindVertexArray(m_screenQuadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

#endif // __EMSCRIPTEN__
