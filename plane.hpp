#pragma once

#include <glad/gl.h>

#include <glm/ext.hpp>
#include <glm/glm.hpp>

class Plane
{
public:
	Plane(float size, int sqrtInstances);
	void init();
	void regenGeometry(float size, int sqrtInstances);
	void draw();
	void destroy();

private:
	int instances, sqrtInstances, numVertices, startLOD;
	float size;
	GLuint vbo, vao;
	GLuint offsetsBuffer;

	void genGeometry();
};
