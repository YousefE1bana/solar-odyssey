#pragma once
#include <GL/glew.h>
#include <string>

std::string readFileText(const char* path);
GLuint compileShader(GLenum type, const std::string& src);
GLuint linkProgram(GLuint vs, GLuint fs);
GLuint loadProgramFromFiles(const char* vsPath, const char* fsPath);
