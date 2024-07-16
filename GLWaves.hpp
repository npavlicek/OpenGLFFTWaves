#pragma once

#include <glad/gl.h>

#include <glm/ext.hpp>
#include <glm/glm.hpp>

#include "Spectrum.hpp"
#include "WaveSettings.hpp"
#include "plane.hpp"

struct GLFWwindow;

struct input
{
	bool w, a, s, d, q, c, r;
	bool shift;
	bool captureCursor = true;
};

class GLWaves
{
public:
	void init();
	void loop();
	void destroy();

private:
	GLFWwindow *window;
	int windowWidth, windowHeight;
	WaveSettings ws;
	Spectrum cascade1;
	Spectrum cascade2;
	Spectrum cascade3;

	double yaw = 3.14, pitch = -3.14 / 4;
	glm::vec3 camPos{0.f, 4.f, 4.f};

	void initWindowAndContext();
	void drawUI();
	void drawUI(Plane &waterPlane, GLuint waterShaderProgram, glm::mat4 projection);
	glm::mat4 computeViewMatrix(GLFWwindow *win, float delta, float mouseSpeed, float camSpeed);
	static void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);
	static void debugCallback(GLenum source,
	                          GLenum type,
	                          GLuint id,
	                          GLenum severity,
	                          GLsizei length,
	                          const GLchar *message,
	                          const void *userParam);
};
