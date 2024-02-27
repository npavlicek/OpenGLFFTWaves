#include "plane.hpp"

#include <glm/glm.hpp>
#include <iostream>

Plane::Plane(float size, int sqrtInstances)
{
	this->size = size;
	this->sqrtInstances = sqrtInstances;
	this->instances = sqrtInstances * sqrtInstances;
	this->startLOD = 20;
	this->numVertices = startLOD * startLOD * 4;
}

void Plane::init()
{
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// Specify vertex attributes
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	glGenBuffers(1, &vbo);
	glGenBuffers(1, &offsetsBuffer);

	genGeometry();

	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 5, 0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 5, reinterpret_cast<void *>(sizeof(GLfloat) * 3));

	glBindBuffer(GL_ARRAY_BUFFER, offsetsBuffer);

	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), 0);
	glVertexAttribDivisor(2, 1);
}

void Plane::genGeometry()
{
	glm::vec2 *offsets = new glm::vec2[instances];

	struct vertex
	{
		glm::vec3 pos;
		glm::vec2 texCoord;
	};

	vertex *vertices = new vertex[numVertices];

	float interval = size / (startLOD - 1);
	float texInterval = 1.0 / (startLOD - 1);

	for (int i = 0; i < startLOD; i++)
	{
		for (int j = 0; j < startLOD; j++)
		{
			int index = (j * startLOD + i) * 4;

			vertices[index] = vertex{glm::vec3{i * interval, 0.f, j * interval}, glm::vec2{i * texInterval, j * texInterval}};
			vertices[index + 1] = vertex{glm::vec3{i * interval, 0.f, j * interval - interval},
																	 glm::vec2{i * texInterval, j * texInterval + texInterval}};
			vertices[index + 2] = vertex{glm::vec3{i * interval + interval, 0.f, j * interval - interval},
																	 glm::vec2{i * texInterval + texInterval, j * texInterval + texInterval}};
			vertices[index + 3] = vertex{glm::vec3{i * interval + interval, 0.f, j * interval},
																	 glm::vec2{i * texInterval + texInterval, j * texInterval}};
		}
	}

	float offset = -sqrtInstances * size / 2;

	for (int i = 0; i < instances; i++)
	{
		int column = i % sqrtInstances;
		int row = i / sqrtInstances;

		offsets[i] = glm::vec2(column * size + offset, row * size + offset);
	}

	glBindBuffer(GL_ARRAY_BUFFER, offsetsBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * instances, offsets, GL_STATIC_DRAW);

	glObjectLabel(GL_BUFFER, offsetsBuffer, 15, "Offsets Buffer");

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex) * numVertices, vertices, GL_STATIC_DRAW);

	glObjectLabel(GL_BUFFER, vbo, 16, "Vertices Buffer");

	delete[] offsets;
	delete[] vertices;
}

void Plane::regenGeometry(float size, int sqrtInstances)
{
	this->size = size;
	this->sqrtInstances = sqrtInstances;
	this->instances = sqrtInstances * sqrtInstances;

	genGeometry();
}

void Plane::draw()
{
	glBindVertexArray(vao);
	glPatchParameteri(GL_PATCH_VERTICES, 4);
	glDrawArraysInstanced(GL_PATCHES, 0, numVertices, instances);
}

void Plane::destroy()
{
	glDeleteBuffers(1, &vbo);
	glDeleteBuffers(1, &offsetsBuffer);
	glDeleteVertexArrays(1, &vao);
}
