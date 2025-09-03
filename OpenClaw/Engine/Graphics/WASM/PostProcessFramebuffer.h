#pragma once

#ifdef __EMSCRIPTEN__

#include <GLES3/gl3.h>
#include <vector>

struct Framebuffer {
    GLuint fbo;
    GLuint colorTexture;
    GLuint depthRenderbuffer;
    int width;
    int height;
    bool valid;
};

class PostProcessFramebuffer {
public:
    PostProcessFramebuffer();
    ~PostProcessFramebuffer();
    
    bool Initialize(int width, int height);
    void Shutdown();
    
    // Framebuffer management
    bool CreateFramebuffer(int width, int height);
    void DestroyFramebuffer(Framebuffer& fb);
    
    // Binding
    void BindFramebuffer(const Framebuffer& fb);
    void BindDefault();
    
    // Texture access
    GLuint GetColorTexture(const Framebuffer& fb) const;
    
    // Utility
    void SetViewport(const Framebuffer& fb);
    bool IsValid() const { return m_initialized; }
    
    // Framebuffer access
    const Framebuffer* GetFirstFramebuffer() const;
    
    // Screen quad rendering
    void RenderScreenQuad();

private:
    bool CreateScreenQuad();
    void DestroyScreenQuad();
    
    std::vector<Framebuffer> m_framebuffers;
    GLuint m_screenQuadVAO;
    GLuint m_screenQuadVBO;
    bool m_initialized;
    int m_currentWidth;
    int m_currentHeight;
};

#endif // __EMSCRIPTEN__
