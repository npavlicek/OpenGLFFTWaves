#pragma once

#include <chrono>
#include <vector>

#include <glad/gl.h>

struct Color
{
	GLfloat r;
	GLfloat g;
	GLfloat b;
	GLfloat a;
};

class Spectrum
{
public:
	void init(int size, int patchSize);
	void cleanup();
	void updateSpectrumTexture();
	void conductFFT();
	GLuint getTexture()
	{
		return spectrumTexture;
	}

private:
	int size, log2size, patchSize;
	std::vector<int> reverseIndices;
	std::vector<Color> initialRandomData;

	std::chrono::time_point<std::chrono::system_clock> start;

	GLuint phillipsShader, butterflyShader, conjugateShader, timeSpectrumShader, fftShader;
	GLuint butterflyTexture, spectrumTexture, initialSpectrumTexture, bufferTexture;
	GLuint reverseIndexBuffer;

	void generateGaussianDist();
	void calculateReverseIndices();
};
