#include "Shader.hpp"

#include <fstream>
#include <iostream>
#include <stdexcept>

char *loadSource(std::string path, int *size)
{
	char *res;
	std::ifstream file(path, std::ios::binary | std::ios::in);
	if (file.is_open())
	{
		file.seekg(0, std::ios::end);
		*size = file.tellg();
		file.seekg(0, std::ios::beg);

		res = reinterpret_cast<char *>(calloc(*size, sizeof(char)));
		file.read(res, *size);
	}
	else
	{
		std::cerr << "Failed to open file: " << path << std::endl;
	}
	file.close();

	return res;
}

GLuint loadShader(GLenum type, std::string path)
{
	GLuint id = glCreateShader(type);

	int size;
	char *source = loadSource(path, &size);

	glShaderBinary(1, &id, GL_SHADER_BINARY_FORMAT_SPIR_V, reinterpret_cast<void *>(source), size);

	free(source);

	glSpecializeShader(id, "main", 0, nullptr, nullptr);

	GLint err;
	glGetShaderiv(id, GL_COMPILE_STATUS, &err);
	if (err == GL_FALSE)
	{
		GLchar log[512];
		glGetShaderInfoLog(id, 512, nullptr, log);
		std::cerr << "Shader error: " << path << std::endl;
		std::cerr << log << std::endl;
	}

	return id;
}

GLuint linkProgram(std::vector<GLuint> shaders)
{
	GLuint id = glCreateProgram();

	for (auto shader : shaders)
	{
		glAttachShader(id, shader);
	}

	glLinkProgram(id);

	int err;
	glGetProgramiv(id, GL_LINK_STATUS, &err);
	if (err == GL_FALSE)
	{
		GLchar log[512];
		glGetProgramInfoLog(id, 512, nullptr, log);
		std::cerr << "Error linking shader program: " << std::endl;
		std::cerr << log << std::endl;
	}

	for (auto shader : shaders)
	{
		glDetachShader(id, shader);
		glDeleteShader(shader);
	}

	return id;
}
