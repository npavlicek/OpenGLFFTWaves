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
	void fft();
	void combineTextures(float scale);
	GLuint getDisplacementsTex()
	{
		return textures[Displacements];
	}
	GLuint getDerivativesTex()
	{
		return textures[Derivates];
	}

private:
	int size, log2size, patchSize;
	std::vector<int> reverseIndices;
	std::vector<Color> initialRandomData;

	std::chrono::time_point<std::chrono::system_clock> start;

	GLuint phillipsShader, butterflyShader, conjugateShader, timeSpectrumShader, fftShader, combineShader;
	GLuint butterflyTexture;
	GLuint reverseIndexBuffer;

	void dispatchFFT(GLuint spectrum);
	void generateGaussianDist();
	void calculateReverseIndices();

	int numTextures = 8;
	GLuint *textures;
	enum SpectrumTextures
	{
		DyDx,
		DzDzx,
		DyxDyz,
		DxxDzz,
		Buffer,
		Displacements,
		Derivates,
		InitialSpectrum
	};
};
