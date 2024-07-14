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
	WorkingSpectrum,
	RandomData,
};

class Spectrum
{
public:
	void init(int size, int patchSize, float scale, float depth, float fetch, float windSpeed, float cutoffLow,
						float cutoffHigh, float spread_blend, float swell, float angle);
	void cleanup();
	void updateSpectrumTexture();
	void fft();
	void combineTextures(float scale);
	void regen(int size, int patchSize, float scale, float depth, float fetch, float windSpeed, float cutoffLow,
						float cutoffHigh, float spread_blend, float swell, float angle);
	void loadShaders();
	GLuint getTexture(SpectrumTextures texType)
	{
		return textures[texType];
	}

private:
	int size, log2size, patchSize;
	float scale;
	float cutoffLow, cutoffHigh;
	float depth, fetch, windSpeed;
	float spread_blend, swell, angle;

	static std::vector<int> reverseIndices;
	static bool initDataGenerated;

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
	int reverseBitsR(int num, int depth);

	int numTextures = 9;
	GLuint *textures;
};
