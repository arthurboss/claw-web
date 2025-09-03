#include "PostProcessShader.h"
#include <iostream>
#include <fstream>
#include <sstream>

#ifdef __EMSCRIPTEN__

PostProcessShader::PostProcessShader() 
    : m_currentProgram(0), m_initialized(false) {
}

PostProcessShader::~PostProcessShader() {
    Shutdown();
}

bool PostProcessShader::Initialize() {
    if (m_initialized) return true;
    
    // Load built-in shaders
    if (!LoadShader("blur", 
        "#version 300 es\n"
        "precision mediump float;\n"
        "in vec2 a_position;\n"
        "in vec2 a_texCoord;\n"
        "out vec2 v_texCoord;\n"
        "void main() {\n"
        "    gl_Position = vec4(a_position, 0.0, 1.0);\n"
        "    v_texCoord = a_texCoord;\n"
        "}",
        "#version 300 es\n"
        "precision mediump float;\n"
        "uniform sampler2D u_texture;\n"
        "uniform vec2 u_resolution;\n"
        "uniform float u_intensity;\n"
        "uniform float u_radius;\n"
        "in vec2 v_texCoord;\n"
        "out vec4 fragColor;\n"
        "void main() {\n"
        "    vec2 texelSize = 1.0 / u_resolution;\n"
        "    vec4 color = vec4(0.0);\n"
        "    float total = 0.0;\n"
        "    for (float x = -u_radius; x <= u_radius; x += 1.0) {\n"
        "        for (float y = -u_radius; y <= u_radius; y += 1.0) {\n"
        "            vec2 offset = vec2(x, y) * texelSize * u_intensity;\n"
        "            vec2 sampleCoord = v_texCoord + offset;\n"
        "            if (sampleCoord.x >= 0.0 && sampleCoord.x <= 1.0 && \n"
        "                sampleCoord.y >= 0.0 && sampleCoord.y <= 1.0) {\n"
        "                float weight = exp(-(x*x + y*y) / (2.0 * u_radius * u_radius));\n"
        "                color += texture(u_texture, sampleCoord) * weight;\n"
        "                total += weight;\n"
        "            }\n"
        "        }\n"
        "    }\n"
        "    fragColor = color / total;\n"
        "}")) {
        std::cerr << "Failed to load blur shader" << std::endl;
        return false;
    }
    
    if (!LoadShader("bloom",
        "#version 300 es\n"
        "precision mediump float;\n"
        "in vec2 a_position;\n"
        "in vec2 a_texCoord;\n"
        "out vec2 v_texCoord;\n"
        "void main() {\n"
        "    gl_Position = vec4(a_position, 0.0, 1.0);\n"
        "    v_texCoord = a_texCoord;\n"
        "}",
        "#version 300 es\n"
        "precision mediump float;\n"
        "uniform sampler2D u_texture;\n"
        "uniform float u_intensity;\n"
        "uniform float u_threshold;\n"
        "in vec2 v_texCoord;\n"
        "out vec4 fragColor;\n"
        "void main() {\n"
        "    vec4 color = texture(u_texture, v_texCoord);\n"
        "    float brightness = (color.r * 0.299 + color.g * 0.587 + color.b * 0.114);\n"
        "    float bloom = max(0.0, brightness - u_threshold);\n"
        "    vec4 bloomColor = color * bloom * u_intensity;\n"
        "    fragColor = color + bloomColor;\n"
        "}")) {
        std::cerr << "Failed to load bloom shader" << std::endl;
        return false;
    }
    
    if (!LoadShader("color",
        "#version 300 es\n"
        "precision mediump float;\n"
        "in vec2 a_position;\n"
        "in vec2 a_texCoord;\n"
        "out vec2 v_texCoord;\n"
        "void main() {\n"
        "    gl_Position = vec4(a_position, 0.0, 1.0);\n"
        "    v_texCoord = a_texCoord;\n"
        "}",
        "#version 300 es\n"
        "precision mediump float;\n"
        "uniform sampler2D u_texture;\n"
        "uniform float u_brightness;\n"
        "uniform float u_contrast;\n"
        "uniform float u_saturation;\n"
        "in vec2 v_texCoord;\n"
        "out vec4 fragColor;\n"
        "void main() {\n"
        "    vec4 color = texture(u_texture, v_texCoord);\n"
        "    color.rgb *= u_brightness;\n"
        "    color.rgb = (color.rgb - 0.5) * u_contrast + 0.5;\n"
        "    float gray = dot(color.rgb, vec3(0.299, 0.587, 0.114));\n"
        "    color.rgb = mix(vec3(gray), color.rgb, u_saturation);\n"
        "    color.rgb = clamp(color.rgb, 0.0, 1.0);\n"
        "    fragColor = color;\n"
        "}")) {
        std::cerr << "Failed to load color shader" << std::endl;
        return false;
    }
    
    m_initialized = true;
    return true;
}

void PostProcessShader::Shutdown() {
    for (auto& program : m_programs) {
        glDeleteProgram(program.second);
    }
    for (auto& shader : m_shaders) {
        glDeleteShader(shader.second);
    }
    m_programs.clear();
    m_shaders.clear();
    m_currentProgram = 0;
    m_initialized = false;
}

bool PostProcessShader::LoadShader(const std::string& name, const std::string& vertexSource, const std::string& fragmentSource) {
    GLuint vertexShader, fragmentShader;
    
    if (!CompileShader(vertexShader, GL_VERTEX_SHADER, vertexSource)) {
        return false;
    }
    
    if (!CompileShader(fragmentShader, GL_FRAGMENT_SHADER, fragmentSource)) {
        glDeleteShader(vertexShader);
        return false;
    }
    
    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    
    if (!LinkProgram(program)) {
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        glDeleteProgram(program);
        return false;
    }
    
    m_programs[name] = program;
    m_shaders[name + "_vertex"] = vertexShader;
    m_shaders[name + "_fragment"] = fragmentShader;
    
    return true;
}

bool PostProcessShader::UseShader(const std::string& name) {
    auto it = m_programs.find(name);
    if (it == m_programs.end()) {
        std::cerr << "Shader not found: " << name << std::endl;
        return false;
    }
    
    glUseProgram(it->second);
    m_currentProgram = it->second;
    return true;
}

void PostProcessShader::SetUniform1f(const std::string& name, float value) {
    if (m_currentProgram == 0) return;
    
    GLint location = glGetUniformLocation(m_currentProgram, name.c_str());
    if (location != -1) {
        glUniform1f(location, value);
    }
}

void PostProcessShader::SetUniform2f(const std::string& name, float x, float y) {
    if (m_currentProgram == 0) return;
    
    GLint location = glGetUniformLocation(m_currentProgram, name.c_str());
    if (location != -1) {
        glUniform2f(location, x, y);
    }
}

void PostProcessShader::SetUniform1i(const std::string& name, int value) {
    if (m_currentProgram == 0) return;
    
    GLint location = glGetUniformLocation(m_currentProgram, name.c_str());
    if (location != -1) {
        glUniform1i(location, value);
    }
}

void PostProcessShader::BindTexture(const std::string& uniformName, GLuint textureId, int unit) {
    if (m_currentProgram == 0) return;
    
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, textureId);
    
    GLint location = glGetUniformLocation(m_currentProgram, uniformName.c_str());
    if (location != -1) {
        glUniform1i(location, unit);
    }
}

bool PostProcessShader::CompileShader(GLuint& shader, GLenum type, const std::string& source) {
    shader = glCreateShader(type);
    const char* sourcePtr = source.c_str();
    glShaderSource(shader, 1, &sourcePtr, nullptr);
    glCompileShader(shader);
    
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        std::string infoLog = GetShaderInfoLog(shader);
        std::cerr << "Shader compilation failed: " << infoLog << std::endl;
        glDeleteShader(shader);
        return false;
    }
    
    return true;
}

bool PostProcessShader::LinkProgram(GLuint program) {
    glLinkProgram(program);
    
    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        std::string infoLog = GetProgramInfoLog(program);
        std::cerr << "Program linking failed: " << infoLog << std::endl;
        return false;
    }
    
    return true;
}

std::string PostProcessShader::GetShaderInfoLog(GLuint shader) {
    GLint length;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
    
    if (length > 0) {
        std::vector<char> infoLog(length);
        glGetShaderInfoLog(shader, length, nullptr, infoLog.data());
        return std::string(infoLog.data());
    }
    
    return "";
}

std::string PostProcessShader::GetProgramInfoLog(GLuint program) {
    GLint length;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
    
    if (length > 0) {
        std::vector<char> infoLog(length);
        glGetProgramInfoLog(program, length, nullptr, infoLog.data());
        return std::string(infoLog.data());
    }
    
    return "";
}

#endif // __EMSCRIPTEN__
