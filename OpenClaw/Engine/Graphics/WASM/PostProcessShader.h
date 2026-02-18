#pragma once

#ifdef __EMSCRIPTEN__

#include <GLES3/gl3.h>
#include <string>
#include <unordered_map>

class PostProcessShader {
public:
    PostProcessShader();
    ~PostProcessShader();
    
    bool Initialize();
    void Shutdown();
    
    // Shader management
    bool LoadShader(const std::string& name, const std::string& vertexSource, const std::string& fragmentSource);
    bool UseShader(const std::string& name);
    
    // Uniform setters
    void SetUniform1f(const std::string& name, float value);
    void SetUniform2f(const std::string& name, float x, float y);
    void SetUniform1i(const std::string& name, int value);
    
    // Texture binding
    void BindTexture(const std::string& uniformName, GLuint textureId, int unit);
    
    // Utility
    GLuint GetCurrentProgram() const { return m_currentProgram; }
    bool IsValid() const { return m_initialized; }

private:
    bool CompileShader(GLuint& shader, GLenum type, const std::string& source);
    bool LinkProgram(GLuint program);
    std::string GetShaderInfoLog(GLuint shader);
    std::string GetProgramInfoLog(GLuint program);
    
    std::unordered_map<std::string, GLuint> m_shaders;
    std::unordered_map<std::string, GLuint> m_programs;
    GLuint m_currentProgram;
    bool m_initialized;
};

#endif // __EMSCRIPTEN__
