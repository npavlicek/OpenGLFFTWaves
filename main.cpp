#include <chrono>
#include <csignal>
#include <fstream>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/quaternion_geometric.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/trigonometric.hpp>
#include <iostream>
#include <ratio>
#include <stdexcept>
#include <stdlib.h>
#include <string>
#include <vector>

#include "glad/gl.h"

#include "GLFW/glfw3.h"

#include "glm/common.hpp"
#include "glm/ext.hpp"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "Rect.hpp"
#include "Shader.hpp"
#include "Spectrum.hpp"
#include "plane.hpp"

double yaw = 3.14, pitch = -3.14 / 4;
glm::vec3 camPos{0.f, 4.f, 4.f};

struct input
{
	bool w, a, s, d;
	bool captureCursor = true;
} i;

glm::mat4 computeViewMatrix(GLFWwindow *win, float delta, float mouseSpeed, float camSpeed)
{
	double x, y;
	glfwGetCursorPos(win, &x, &y);

	glfwSetCursorPos(win, 1280.f / 2, 720.f / 2);

	yaw += (1280.f / 2 - x) * delta * mouseSpeed;
	pitch += (720.f / 2 - y) * delta * mouseSpeed;

	if (pitch < -3.14f / 2)
		pitch = -3.14f / 2;
	if (pitch > 3.14f / 2)
		pitch = 3.14f / 2;

	glm::vec3 dir{cos(pitch) * sin(yaw), sin(pitch), cos(pitch) * cos(yaw)};
	glm::vec3 right{sin(yaw - 3.14f / 2), 0, cos(yaw - 3.14f / 2)};
	glm::vec3 up = glm::cross(right, dir);

	if (i.w)
		camPos += dir * camSpeed * delta;
	if (i.s)
		camPos -= dir * camSpeed * delta;
	if (i.a)
		camPos -= glm::cross(dir, up) * camSpeed * delta;
	if (i.d)
		camPos += glm::cross(dir, up) * camSpeed * delta;

	return glm::lookAt(camPos, camPos + dir, up);
}

void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, GLFW_TRUE);
	}

	if (key == GLFW_KEY_W && action == GLFW_PRESS)
		i.w = true;
	else if (key == GLFW_KEY_W && action == GLFW_RELEASE)
		i.w = false;

	if (key == GLFW_KEY_S && action == GLFW_PRESS)
		i.s = true;
	else if (key == GLFW_KEY_S && action == GLFW_RELEASE)
		i.s = false;

	if (key == GLFW_KEY_A && action == GLFW_PRESS)
		i.a = true;
	else if (key == GLFW_KEY_A && action == GLFW_RELEASE)
		i.a = false;

	if (key == GLFW_KEY_D && action == GLFW_PRESS)
		i.d = true;
	else if (key == GLFW_KEY_D && action == GLFW_RELEASE)
		i.d = false;

	if (key == GLFW_KEY_E && action == GLFW_PRESS)
	{
		i.captureCursor = !i.captureCursor;
		glfwSetCursorPos(window, 1280.f / 2, 720.f / 2);
	}
}

void debugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message,
									 const void *userParam)
{
	if (type == GL_DEBUG_TYPE_ERROR)
	{
		std::string err;
		err.append("[");
		err.append(std::to_string(id));
		err.append("] ");
		err.append(message);
		throw std::runtime_error(err);
	}
	else
	{
		std::cout << message << std::endl << std::endl;
	}
}

int main()
{
	glfwInit();

	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);

	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
	glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
	glfwWindowHint(GLFW_SAMPLES, 4);

	GLFWwindow *win = glfwCreateWindow(1280, 720, "Test", nullptr, nullptr);
	if (win == NULL)
	{
		std::cerr << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return EXIT_FAILURE;
	}

	glfwSetKeyCallback(win, keyCallback);

	int count;
	GLFWmonitor **mon = glfwGetMonitors(&count);
	const GLFWvidmode *vidmode = glfwGetVideoMode(mon[0]);
	glfwSetWindowPos(win, (vidmode->width - 1280) / 2, (vidmode->height - 720) / 2);

	glfwShowWindow(win);

	glfwMakeContextCurrent(win);

	int version = gladLoadGL(glfwGetProcAddress);
	if (version == 0)
	{
		std::cerr << "Failed to initialize OpenGL context" << std::endl;
		glfwTerminate();
		return EXIT_FAILURE;
	}

	std::cout << "Loaded OpenGL " << GLAD_VERSION_MAJOR(version) << "." << GLAD_VERSION_MINOR(version) << std::endl;

	// Enable gl features
	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageCallback(debugCallback, nullptr);

	// glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);

	// glFrontFace(GL_CW);

	// Initialzie IMGUI
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGui_ImplGlfw_InitForOpenGL(win, true);
	ImGui_ImplOpenGL3_Init();

	Spectrum spec{};
	spec.init(256, 400);

	GLuint texture = spec.getTexture();

	Rect rect;
	rect.init();

	// Generate vaos and bind
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// Specify vertex attributes
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 5, 0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 5, reinterpret_cast<void *>(sizeof(GLfloat) * 3));

	// Load shader
	GLuint texturedRectShader =
			linkProgram({loadShader(GL_VERTEX_SHADER, "shaders/compiled/vertex/textured_rect.spv"),
									 loadShader(GL_FRAGMENT_SHADER, "shaders/compiled/fragment/textured_rect.spv")});

	glUseProgram(texturedRectShader);

	glm::mat4 model = glm::scale(glm::mat4(1.f), glm::vec3(5.f));
	glm::mat4 view{1.f};
	glm::mat4 projection = glm::perspective(glm::radians(60.f), 1280.f / 720.f, 0.1f, 100.f);

	glUniformMatrix4fv(2, 1, GL_FALSE, glm::value_ptr(projection));

	float degreesPerSec = 0.f;

	glfwSetCursorPos(win, 1280.f / 2, 720.f / 2);

	auto startTime = std::chrono::system_clock::now();

	glClearColor(0.f, 0.3f, 0.8f, 1.f);

	while (!glfwWindowShouldClose(win))
	{
		glfwPollEvents();

		auto endTime = std::chrono::system_clock::now();
		std::chrono::duration<float> elapsed = endTime - startTime;
		auto delta = elapsed.count();
		startTime = endTime;

		if (i.captureCursor)
		{
			view = computeViewMatrix(win, delta, 1.f, 5.f);
			ImGui::SetMouseCursor(ImGuiMouseCursor_None);
		}

		model = glm::rotate(model, glm::radians(degreesPerSec * delta), glm::vec3(0, 1, 0));

		spec.updateSpectrumTexture();
		spec.conductFFT();

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		ImGui_ImplGlfw_NewFrame();
		ImGui_ImplOpenGL3_NewFrame();

		ImGui::NewFrame();

		ImGui::Begin("Model settings");
		ImGui::SliderFloat("Degrees Per Second", &degreesPerSec, 0.f, 360.f);
		ImGui::End();

		ImGui::Render();

		glBindVertexArray(vao);

		glUseProgram(texturedRectShader);

		glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(model));
		glUniformMatrix4fv(1, 1, GL_FALSE, glm::value_ptr(view));

		rect.bind();

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);

		rect.draw();

		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(win);
	}

	rect.cleanup();
	spec.cleanup();

	glDeleteVertexArrays(1, &vao);

	ImGui_ImplGlfw_Shutdown();
	ImGui_ImplOpenGL3_Shutdown();
	ImGui::DestroyContext();

	glfwTerminate();

	return EXIT_SUCCESS;
}
