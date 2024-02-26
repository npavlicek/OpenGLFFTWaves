#include "plane.hpp"

Plane::Plane(int numX, int numY, float interval)
{
	this->numX = numX;
	this->numY = numY;
	this->interval = interval;
}

void Plane::init()
{
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &vbo);
	glGenBuffers(1, &ebo);

	genGeometry();

	// Specify vertex attributes
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 5, 0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 5, reinterpret_cast<void *>(sizeof(GLfloat) * 3));
}

void Plane::genGeometry()
{

	numIndices = (numX - 1) * (numY - 1) * 6;
	int numVertices = numX * numY;
	vertex *vertices = new vertex[numVertices];
	int *indices = new int[numIndices];

	float width = numX * interval;
	float height = numY * interval;

	float offsetX = width / 2;
	float offsetY = height / 2;

	for (int y = 0; y < numY; y++)
	{
		for (int x = 0; x < numX; x++)
		{
			int vertex = (y * numX + x);
			vertices[vertex].pos = glm::vec3(interval * x - offsetX, 0, interval * y - offsetY);
			vertices[vertex].texCoord = glm::vec2(interval * x / width, interval * y / height);

			if (x != numX - 1 && y != numY - 1)
			{
				int index = (y * (numX - 1) + x) * 6;
				indices[index] = y * numX + x;
				indices[index + 1] = (y + 1) * numX + x + 1;
				indices[index + 2] = y * numX + x + 1;

				indices[index + 3] = y * numX + x;
				indices[index + 4] = (y + 1) * numX + x;
				indices[index + 5] = (y + 1) * numX + x + 1;
			}
		}
	}

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex) * numVertices, vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * numIndices, indices, GL_STATIC_DRAW);

	delete[] vertices;
	delete[] indices;
}

void Plane::regenGeometry(int numX, int numY, float interval)
{
	this->numX = numX;
	this->numY = numY;
	this->interval = interval;

	genGeometry();
}

void Plane::draw()
{
	glBindVertexArray(vao);
	glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, nullptr);
}

void Plane::destroy()
{
	glDeleteBuffers(1, &vbo);
	glDeleteBuffers(1, &ebo);
	glDeleteVertexArrays(1, &vao);
}
