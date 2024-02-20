#pragma once

#include <string>
#include <vector>

#include <glad/gl.h>

char *loadSource(std::string path, int *size);

GLuint loadShader(GLenum type, std::string path);

GLuint linkProgram(std::vector<GLuint> shaders);
