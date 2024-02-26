#pragma once

#include <glad/gl.h>

#include <glm/ext.hpp>
#include <glm/glm.hpp>

struct vertex
{
	glm::vec3 pos;
	glm::vec2 texCoord;
};

class Plane
{
public:
	Plane(int sqrtSize, float interval, int sqrtInstance);
	void init();
	void regenGeometry(int sqrtSize, float interval, int sqrtInstance);
	void draw();
	void destroy();

private:
	int numX, numY;
	int instances;
	float interval;
	int numIndices;
	GLuint vbo, ebo, vao;

	void genGeometry();
};
