#pragma once

#include <glad/gl.h>

#include <glm/ext.hpp>
#include <glm/glm.hpp>

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

	struct
	{
		struct
		{
			float scale = 1.f;
			float specScale = 1.f;
			float normalStrength = 1.5f;
			bool renderWater = true;
			bool simulate = true;
			bool cubemap = true;
			int selection = 0;
			int inputFormat = 0;
			int inputTexSize = 2;
			int inputPatchSize = 250;
			float texCoordScale = 1.f;
			bool renderWireframe = false;
			int newSize = 512;
		} wave;
		struct
		{

		} cascade1;
		struct
		{

		} cascade2;
		struct
		{

		} cascade3;
		struct plane_settings
		{
			float size = 1000.f;
			int sqrtOfInstances = 3;
			int lod = 55;
		} ps;
		struct
		{
			float speed = 50.f;
			float sprintFactor = 7.f;
		} cam;
		struct
		{
			int minTessLevel = 1;
			int maxTessLevel = 25;
			float minDistance = 75.f;
			float maxDistance = 250.f;
			bool tessFollowCam = true;
		} tess;
	} settings;

	double yaw = 3.14, pitch = -3.14 / 4;
	glm::vec3 camPos{0.f, 4.f, 4.f};

	void initWindowAndContext();
	glm::mat4 computeViewMatrix(GLFWwindow *win, float delta, float mouseSpeed, float camSpeed);
	static void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);
	static void debugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
														const GLchar *message, const void *userParam);
};
