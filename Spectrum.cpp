#include "Spectrum.hpp"

#include <chrono>
#include <cmath>
#include <fstream>
#include <glm/geometric.hpp>
#include <initializer_list>
#include <iostream>
#include <random>
#include <ratio>

#include <glm/glm.hpp>

#include "Shader.hpp"

void Spectrum::init(int size, int patchSize)
{
	this->size = size;
	this->log2size = static_cast<int>(log2(size));
	this->patchSize = patchSize;
	this->scale = 1.f;

	start = std::chrono::system_clock::now();

	initialSpectrum = new Color[size * size];

	// Calculate the reverse indices and initial random data
	calculateReverseIndices();
	generateGaussianDist();
	calculateJonswapSpectrum();

	// Load all the shaders
	jonswapShader = linkProgram({loadShader(GL_COMPUTE_SHADER, "shaders/compiled/compute/jonswap_spec.spv")});
	phillipsShader = linkProgram({loadShader(GL_COMPUTE_SHADER, "shaders/compiled/compute/phillips_spec.spv")});
	conjugateShader = linkProgram({loadShader(GL_COMPUTE_SHADER, "shaders/compiled/compute/conjugate.spv")});
	butterflyShader = linkProgram({loadShader(GL_COMPUTE_SHADER, "shaders/compiled/compute/butterfly.spv")});
	timeSpectrumShader = linkProgram({loadShader(GL_COMPUTE_SHADER, "shaders/compiled/compute/time_spec.spv")});
	fftShader = linkProgram({loadShader(GL_COMPUTE_SHADER, "shaders/compiled/compute/fft.spv")});
	combineShader = linkProgram({loadShader(GL_COMPUTE_SHADER, "shaders/compiled/compute/combine_tex.spv")});

	// Generate all the necessary textures and buffers
	textures = new GLuint[numTextures];

	glGenTextures(numTextures, textures);
	glGenTextures(1, &butterflyTexture);
	glGenBuffers(1, &reverseIndexBuffer);

	formatTextures();

	genInitDataAndUpload();
}

void Spectrum::regen(int size, int patchSize, float scale)
{
	this->size = size;
	this->patchSize = patchSize;
	this->log2size = static_cast<int>(log2(size));
	this->scale = scale;

	delete[] initialSpectrum;
	reverseIndices.clear();

	initialSpectrum = new Color[size * size];

	calculateReverseIndices();
	generateGaussianDist();
	calculateJonswapSpectrum();

	formatTextures();

	genInitDataAndUpload();
}

void Spectrum::genInitDataAndUpload()
{
	glTextureSubImage2D(textures[InitialSpectrum], 0, 0, 0, size, size, GL_RGBA, GL_FLOAT, initialSpectrum);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, reverseIndexBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, reverseIndices.size() * sizeof(int), reverseIndices.data(), GL_STREAM_DRAW);

	// Execute compute shaders
	// butterfly shader
	glUseProgram(butterflyShader);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, reverseIndexBuffer);
	glBindImageTexture(0, butterflyTexture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
	glDispatchCompute(log2size, size / 16, 1);

	// spectrum shader
	glUseProgram(jonswapShader);
	glBindImageTexture(0, textures[InitialSpectrum], 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
	glUniform1i(0, patchSize);
	// glDispatchCompute(size / 8, size / 8, 1);

	// To make sure spectrum is generated by the time we execute conjugate shader
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	glUseProgram(conjugateShader);
	glBindImageTexture(0, textures[InitialSpectrum], 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
	glDispatchCompute(size / 8, size / 8, 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

void Spectrum::formatTextures()
{
	for (int i = 0; i < numTextures; i++)
	{
		glBindTexture(GL_TEXTURE_2D, textures[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, size, size, 0, GL_RGBA, GL_FLOAT, nullptr);
		// This is necessary because the default filter is GL_LINEAR_MIPMAP_LINEAR and would result in a "mipmap incomplete"
		// texture and the compute shader wont accept it.
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}

	glBindTexture(GL_TEXTURE_2D, textures[Displacements]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindTexture(GL_TEXTURE_2D, butterflyTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, static_cast<GLsizei>(log2(size)), size, 0, GL_RGBA, GL_FLOAT, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindTexture(GL_TEXTURE_2D, textures[Derivates]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glObjectLabel(GL_TEXTURE, textures[Derivates], 12, "Derivatives");
	glObjectLabel(GL_TEXTURE, textures[InitialSpectrum], 17, "Initial Spectrum");
	glObjectLabel(GL_TEXTURE, textures[DyDx], 8, "DyDxTex");
}

void Spectrum::combineTextures(float scale)
{
	glUseProgram(combineShader);
	glBindImageTexture(0, textures[DyDx], 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
	glBindImageTexture(1, textures[DzDzx], 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
	glBindImageTexture(2, textures[DyxDyz], 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
	glBindImageTexture(3, textures[DxxDzz], 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
	glBindImageTexture(4, textures[Displacements], 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
	glBindImageTexture(5, textures[Derivates], 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
	glUniform1f(0, scale);
	glDispatchCompute(size / 16, size / 16, 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	glGenerateTextureMipmap(textures[Derivates]);
}

void Spectrum::updateSpectrumTexture()
{
	auto time = std::chrono::system_clock::now();
	auto timeSinceStart = std::chrono::duration_cast<std::chrono::duration<float, std::ratio<1>>>(time - start).count();

	glUseProgram(timeSpectrumShader);
	glBindImageTexture(0, textures[InitialSpectrum], 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
	glBindImageTexture(1, textures[DyDx], 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
	glBindImageTexture(2, textures[DzDzx], 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
	glBindImageTexture(3, textures[DyxDyz], 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
	glBindImageTexture(4, textures[DxxDzz], 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
	glUniform1f(0, timeSinceStart);
	glUniform1i(1, patchSize);
	glDispatchCompute(size / 8, size / 8, 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

void Spectrum::fft()
{
	dispatchFFT(textures[DyDx]);
	dispatchFFT(textures[DzDzx]);
	dispatchFFT(textures[DyxDyz]);
	dispatchFFT(textures[DxxDzz]);
}

void Spectrum::dispatchFFT(GLuint spectrum)
{
	glUseProgram(fftShader);
	glBindImageTexture(0, butterflyTexture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
	glBindImageTexture(1, spectrum, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
	glBindImageTexture(2, textures[Buffer], 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

	int buffer = 0;

	// horizontal
	glUniform1i(0, 0);
	for (int i = 0; i < log2size; i++)
	{
		// Set stage
		glUniform1i(1, buffer);
		glUniform1i(2, i);
		glDispatchCompute(size / 16, size / 16, 1);
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
		buffer = !buffer;
	}

	// vertical
	glUniform1i(0, 1);
	for (int i = 0; i < log2size; i++)
	{
		glUniform1i(1, buffer);
		glUniform1i(2, i);
		glDispatchCompute(size / 16, size / 16, 1);
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
		buffer = !buffer;
	}

	// Execute inversion and permutation shader
	glUniform1i(0, 2);
	glDispatchCompute(size / 16, size / 16, 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

void Spectrum::cleanup()
{
	glDeleteProgram(jonswapShader);
	glDeleteProgram(conjugateShader);
	glDeleteProgram(butterflyShader);
	glDeleteProgram(timeSpectrumShader);
	glDeleteProgram(fftShader);
	glDeleteProgram(combineShader);

	glDeleteTextures(numTextures, textures);
	glDeleteTextures(1, &butterflyTexture);
	delete textures;

	glDeleteBuffers(1, &reverseIndexBuffer);

	delete[] initialSpectrum;
}

float jonswap(float omega)
{
	float fetch = 800.f;
	float windSpeed = 80.f;
	float gravity = 9.81f;

	float gamma = 3.3;
	float omega_peak = 22.f * pow(pow(gravity, 2.f) / (windSpeed * fetch), .33f);
	float alpha = 0.076f * pow(pow(windSpeed, 2.f) / (fetch * gravity), 0.22f);
	float sigma = omega <= omega_peak ? 0.07f : 0.09f;
	float r = exp(-pow(omega - omega_peak, 2.f) / (2.f * pow(sigma, 2.f) * pow(omega_peak, 2.f)));

	float res = alpha * pow(gravity, 2.f) / pow(sigma, 5.f);
	res *= pow(gamma, r);
	res *= exp(-(5.0 / 4.0) * pow(omega_peak / omega, 4));

	return res;
}

float dispersion(float k_mag)
{
	return sqrt(9.81f * k_mag);
}

float dispersiond(float k_mag)
{
	return sqrt(9.81f) / (2.f * sqrt(k_mag));
}

#define PI 3.1415

void Spectrum::calculateJonswapSpectrum()
{
	for (int y = 0; y < size; y++)
	{
		for (int x = 0; x < size; x++)
		{
			float n = x - size / 2.0;
			float m = y - size / 2.0;

			float delta = 2.f * PI / (1.f * patchSize);

			glm::vec2 k{n * delta, m * delta};
			float k_mag = glm::length(k);

			float omega = dispersion(k_mag);
			float sw = jonswap(omega) * dispersiond(k_mag) / k_mag / 2 / PI;

			float abar = sqrt(2.0 * sw * delta * delta);

			initialSpectrum[y * size + x].r *= abar;
			initialSpectrum[y * size + x].g *= abar;
		}
	}
}

void Spectrum::calculateReverseIndices()
{
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
		reverseIndices.push_back(reversed);
	}
}

void Spectrum::generateGaussianDist()
{
	std::random_device device;
	std::normal_distribution<GLfloat> dist{0, 1};
	std::mt19937 gen(device());

	auto randVal = [&dist, &gen]() -> GLfloat { return dist(gen); };

	for (int i = 0; i < size * size; i++)
	{
		initialSpectrum[i].r = randVal();
		initialSpectrum[i].g = randVal();
		initialSpectrum[i].b = 0;
		initialSpectrum[i].a = 0;
	}
}
