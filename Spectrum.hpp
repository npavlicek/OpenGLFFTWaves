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

class Spectrum
{
public:
	void init(int size, int patchSize);
	void cleanup();
	void updateSpectrumTexture();
	void fft();
	void combineTextures(float scale);
	void regen(int size, int patchSize, float scale);
	GLuint getTexture(SpectrumTextures texType)
	{
		return textures[texType];
	}

private:
	int size, log2size, patchSize;
	float scale;
	std::vector<int> reverseIndices;
	Color *initialSpectrum;

	std::chrono::time_point<std::chrono::system_clock> start;

	GLuint jonswapShader, butterflyShader, conjugateShader, timeSpectrumShader, fftShader, combineShader, phillipsShader;
	GLuint butterflyTexture;
	GLuint reverseIndexBuffer;

	void dispatchFFT(GLuint spectrum);
	void generateGaussianDist();
	void calculateReverseIndices();
	void calculateJonswapSpectrum();
	void formatTextures();
	void genInitDataAndUpload();

	int numTextures = 8;
	GLuint *textures;
};
