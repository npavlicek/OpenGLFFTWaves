#include "plane.hpp"

#include <glm/gtx/string_cast.hpp>
#include <iostream>

Plane::Plane(int numX, int numY, float interval)
{
	this->numX = numX;
	this->numY = numY;
	this->interval = interval;
}

void Plane::init()
{
	numIndices = (numX - 1) * (numY - 1) * 6;
	vertex vertices[numX * numY];
	int indices[numIndices];

	float offsetX = (numX * interval) / 2;
	float offsetY = (numY * interval) / 2;

	std::random_device rd;
	std::normal_distribution<float> nd{0.f, 0.5f};
	std::mt19937 mt(rd());

	auto randomHeight = [&nd, &mt]() -> float { return nd(mt); };

	for (int y = 0; y < numY; y++)
	{
		for (int x = 0; x < numX; x++)
		{
			int vertex = (y * numX + x);
			vertices[vertex].pos = glm::vec3(interval * x - offsetX, randomHeight(), interval * y - offsetY);

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

	for (int y = 0; y < numY; y++)
	{
		for (int x = 0; x < numX; x++)
		{
			int vertex = (y * numX + x);

			int nextX = (x + 1) % numX;
			int nextY = y - 1;

			if (nextY < 0)
				nextY += numY;

			int nextVertX = y * numX + nextX;
			int nextVertY = nextY * numX + x;

			// x cross z
			float dy_dx = vertices[nextVertX].pos.y - vertices[vertex].pos.y;
			float dy_dz = vertices[nextVertY].pos.y - vertices[vertex].pos.y;

			vertices[vertex].norm = glm::normalize(glm::cross(glm::vec3(1, dy_dx, 0.f), glm::vec3(0.f, dy_dz, -1)));
		}
	}

	glGenBuffers(1, &vbo);
	glGenBuffers(1, &ebo);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
}

void Plane::draw()
{
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, nullptr);
}

void Plane::destroy()
{
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glDeleteBuffers(1, &vbo);
	glDeleteBuffers(1, &ebo);
}
