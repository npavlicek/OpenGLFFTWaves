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
	Plane(int numX, int numY, float interval);
	void init();
	void regenGeometry(int numX, int numY, float interval);
	void draw();
	void destroy();

private:
	int numX, numY;
	float interval;
	int numIndices;
	GLuint vbo, ebo, vao;

	void genGeometry();
};
