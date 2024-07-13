#include "Skybox.hpp"

#include <glm/gtc/type_ptr.hpp>
#include <iostream>

#include <glad/gl.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stbi_image.h"

#include "Shader.hpp"

Skybox::Skybox()
{
}

const char *texs[] = {"px.png", "nx.png", "py.png", "ny.png", "pz.png", "nz.png"};

const GLfloat verts[] = {
		-1.f, -1.f, -1.f, // Back bottom left
		1.f,  -1.f, -1.f, // Back bottom right
		-1.f, 1.f,  -1.f, // Back top left
		1.f,  1.f,  -1.f, // Back top right

		-1.f, -1.f, 1.f, // Front bottom left
		1.f,  -1.f, 1.f, // Front bottom right
		-1.f, 1.f,  1.f, // Front top left
		1.f,  1.f,  1.f, // Front top right
};

const GLubyte indices[] = {
		0, 2, 3, // Back face
		0, 3, 1,

		4, 2, 0, // Left face
		4, 6, 2,

		5, 3, 7, // Right face
		5, 1, 3,

		4, 7, 6, // Front face
		4, 5, 7,

		6, 7, 2, // Top Face
		7, 3, 2,

		4, 0, 5, // Bottom Face
		5, 0, 1,
};

GLuint cubemapID;
GLuint shaderProgram;
GLuint skyboxVAO;
GLuint skyboxVBO;
GLuint skyboxEAB;

void Skybox::init()
{
	shaderProgram = linkProgram({loadShader(GL_FRAGMENT_SHADER, "./shaders/compiled/fragment/skybox.spv"),
	                             loadShader(GL_VERTEX_SHADER, "./shaders/compiled/vertex/skybox.spv")});

	glGenTextures(1, &cubemapID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapID);
	glObjectLabel(GL_TEXTURE, cubemapID, 7, "cubemap");

	for (int i = 0; i < 6; i++)
	{
		int width, height, channels;
		stbi_uc *data = stbi_load(texs[i], &width, &height, &channels, 0);
		if (data)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
			stbi_image_free(data);
		}
		else
			std::cerr << "could not find cubemap file!" << std::endl;
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glCreateVertexArrays(1, &skyboxVAO);
	glCreateBuffers(1, &skyboxVBO);
	glCreateBuffers(1, &skyboxEAB);

	glObjectLabel(GL_BUFFER, skyboxVBO, 9, "skyboxVBO");

	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, skyboxEAB);

	glBufferData(GL_ARRAY_BUFFER, 8 * 3 * sizeof(GLfloat), verts, GL_STATIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 36 * sizeof(GLuint), indices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
}

void Skybox::render(glm::mat4 view, glm::mat4 proj)
{
	view = glm::mat4(glm::mat3(view));
	glDepthMask(GL_FALSE);
	glUseProgram(shaderProgram);
	glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(1, 1, GL_FALSE, glm::value_ptr(proj));
	glBindVertexArray(skyboxVAO);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapID);
	glDrawElements(GL_TRIANGLES, sizeof(indices), GL_UNSIGNED_BYTE, 0);
	glDepthMask(GL_TRUE);
}

void Skybox::cleanup()
{
	glDeleteProgram(shaderProgram);
	glDeleteBuffers(1, &skyboxVBO);
	glDeleteBuffers(1, &skyboxEAB);
	glDeleteVertexArrays(1, &skyboxVAO);
	glDeleteTextures(1, &cubemapID);
}
