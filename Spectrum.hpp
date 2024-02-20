#pragma once

#include <random>
#include <vector>

#include <glad/gl.h>

#include "Shader.hpp"

struct Color
{
	GLfloat r;
	GLfloat g;
	GLfloat b;
	GLfloat a;
};

std::vector<int> calculateReverseIndices(int size);

GLuint genRandDist(int size);
