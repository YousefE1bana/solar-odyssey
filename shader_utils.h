#pragma once
#include <GL/glew.h>
#include <cstdio>
#include <string>
#include <fstream>
#include <sstream>

inline std::string readFileText(const char* path){
    std::ifstream f(path, std::ios::in);
    if (!f) {
        fprintf(stderr, "[Shader] Could not open file: %s\n", path);
        return std::string();
    }
    std::stringstream ss; ss << f.rdbuf();
    return ss.str();
}

inline GLuint compileShader(GLenum type, const std::string& src){
    GLuint sh = glCreateShader(type);
    const char* c = src.c_str();
    glShaderSource(sh, 1, &c, nullptr);
    glCompileShader(sh);
    GLint ok=0; glGetShaderiv(sh, GL_COMPILE_STATUS, &ok);
    if(!ok){
        char log[2048]; GLsizei len=0; glGetShaderInfoLog(sh, 2048, &len, log);
        fprintf(stderr, "[Shader] %s shader compile failed:\n%.*s\n",
                (type == GL_VERTEX_SHADER) ? "vertex" : "fragment", (int)len, log);
        glDeleteShader(sh);
        return 0;
    }
    return sh;
}

inline GLuint linkProgram(GLuint vs, GLuint fs){
    if (vs == 0 || fs == 0) {
        if (vs) glDeleteShader(vs);
        if (fs) glDeleteShader(fs);
        return 0;
    }
    GLuint p = glCreateProgram();
    glAttachShader(p, vs); glAttachShader(p, fs);
    glLinkProgram(p);
    GLint ok=0; glGetProgramiv(p, GL_LINK_STATUS, &ok);
    if(!ok){
        char log[2048]; GLsizei len=0; glGetProgramInfoLog(p, 2048, &len, log);
        fprintf(stderr, "[Shader] Program link failed:\n%.*s\n", (int)len, log);
        glDetachShader(p, vs); glDetachShader(p, fs);
        glDeleteShader(vs); glDeleteShader(fs);
        glDeleteProgram(p);
        return 0;
    }
    glDetachShader(p, vs); glDetachShader(p, fs);
    glDeleteShader(vs); glDeleteShader(fs);
    return p;
}

// High-level loader: compiles + links from source files and ALWAYS deletes the
// intermediate shader objects (fixes the leak where compileShader results were
// attached and never released on the success path).
inline GLuint loadProgramFromFiles(const char* vsPath, const char* fsPath){
    std::string vsSrc = readFileText(vsPath);
    std::string fsSrc = readFileText(fsPath);
    if (vsSrc.empty() || fsSrc.empty()) return 0;
    GLuint vs = compileShader(GL_VERTEX_SHADER, vsSrc);
    GLuint fs = compileShader(GL_FRAGMENT_SHADER, fsSrc);
    GLuint p = linkProgram(vs, fs); // linkProgram deletes shaders on both paths
    if (p == 0) {
        fprintf(stderr, "[Shader] Failed to load program: %s + %s\n", vsPath, fsPath);
    }
    return p;
}
