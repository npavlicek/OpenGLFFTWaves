#include "Spectrum.hpp"

#include <cmath>

std::vector<int> calculateReverseIndices(int size)
{
	std::vector<int> res;
	int numBits = log2(size);
	for (int i = 0; i < size; i++)
	{
		int reversed = 0;
		for (int j = 0; j < numBits; j++)
		{
			if (i & static_cast<int>(pow(2, j)))
			{
				reversed |= static_cast<int>(pow(2, numBits - j - 1));
			}
		}
		res.push_back(reversed);
	}
	return res;
}

GLuint genRandDist(int size)
{
	std::random_device device;
	std::normal_distribution<GLfloat> dist{0, 1};
	std::mt19937 gen(device());

	auto randVal = [&dist, &gen]() -> GLfloat { return dist(gen); };

	std::vector<Color> data;

	for (int i = 0; i < size * size; i++)
	{
		Color col{randVal(), randVal(), 0, 0};
		data.push_back(col);
	}

	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, size, size, 0, GL_RGBA, GL_FLOAT, data.data());

	// This is necessary because the default filter is GL_LINEAR_MIPMAP_LINEAR and would result in a "mipmap incomplete"
	// texture and the compute shader wont accept it.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	std::vector<GLuint> shaders{loadShader(GL_COMPUTE_SHADER, "shaders/spectrum.spv")};
	GLuint program = linkProgram(shaders);
	glUseProgram(program);

	glBindImageTexture(0, texture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA16F);

	glDispatchCompute(256 / 8, 256 / 8, 1);

	glDeleteProgram(program);

	auto indices = calculateReverseIndices(size);

	GLuint indexBuffer;
	glGenBuffers(1, &indexBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, indexBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, indices.size() * sizeof(int), indices.data(), GL_STREAM_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, indexBuffer);

	GLuint butterflyTexture;
	glGenTextures(1, &butterflyTexture);
	glBindTexture(GL_TEXTURE_2D, butterflyTexture);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA16F, static_cast<GLsizei>(log2(size)), size);
	glBindImageTexture(0, butterflyTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);

	GLuint butterflyProgram = linkProgram({loadShader(GL_COMPUTE_SHADER, "shaders/butterfly.spv")});
	glUseProgram(butterflyProgram);

	glDispatchCompute(static_cast<GLuint>(log2(size)), size / 16, 1);

	glDeleteProgram(butterflyProgram);

	return texture;
}
