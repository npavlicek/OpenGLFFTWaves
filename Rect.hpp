#pragma once

#include <glad/gl.h>

class Rect
{
public:
	void init()
	{
		glGenBuffers(1, &vbo);
		glGenBuffers(1, &ebo);

		// Buffer data
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	}

	void bind()
	{
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	}

	void draw()
	{
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
	}

	void cleanup()
	{
		glDeleteBuffers(1, &vbo);
		glDeleteBuffers(1, &ebo);
	}

private:
	GLuint vbo, ebo;

	// clang-format off
	GLfloat vertices[20] = {
		// x, y, x,   u, v
		 -1.0f,  1.0f,  0.f,  0.f,  1.f,  // top left
		  1.f,   1.f,   0.f,  1.f,  1.f,  // top right
		 1.f, -1.f,  0.f, 1.f, 0.f,  // bottom right
		-1.f, -1.f, -0.f, 0.f, 0.f   // bottom left
	};

	GLint indices[6] = {
		0, 1, 3,
		3, 1, 2
	};
	// clang-format on
};
