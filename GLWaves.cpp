#include "GLWaves.hpp"

#include <cstdint>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/trigonometric.hpp>
#include <iostream>
#include <stdexcept>
#include <stdint.h>
#include <string>

#include <GLFW/glfw3.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "Shader.hpp"
#include "Spectrum.hpp"
#include "plane.hpp"

input i;

void GLWaves::init()
{
	windowWidth = 1600;
	windowHeight = 900;

	initWindowAndContext();

	// Initialzie IMGUI
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init();

	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageCallback(debugCallback, nullptr);

	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
}

void GLWaves::initWindowAndContext()
{
	glfwInit();

	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
	glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
	glfwWindowHint(GLFW_SAMPLES, 4);

	window = glfwCreateWindow(windowWidth, windowHeight, "Test", nullptr, nullptr);
	if (window == NULL)
	{
		glfwTerminate();
		throw std::runtime_error("Failed to create GLFW window");
	}

	glfwSetKeyCallback(window, keyCallback);

	int count;
	GLFWmonitor **mon = glfwGetMonitors(&count);
	const GLFWvidmode *vidmode = glfwGetVideoMode(mon[0]);
	glfwSetWindowPos(window, (vidmode->width - windowWidth) / 2, (vidmode->height - windowHeight) / 2);

	glfwShowWindow(window);

	glfwMakeContextCurrent(window);

	int version = gladLoadGL(glfwGetProcAddress);
	if (version == 0)
	{
		glfwTerminate();
		throw std::runtime_error("Failed to initialize OpenGL context");
	}

	std::cout << "Loaded OpenGL " << GLAD_VERSION_MAJOR(version) << "." << GLAD_VERSION_MINOR(version) << std::endl;
}

void GLWaves::loop()
{
	Spectrum spec{};
	spec.init(512, 1500);

	spec.updateSpectrumTexture();
	spec.fft();
	spec.combineTextures(1.0);

	GLuint displacementsTex = spec.getTexture(Displacements);
	GLuint derivatesTex = spec.getTexture(Derivates);

	Plane waterPlane(256.f, 2);
	waterPlane.init();

	GLuint waterShader = linkProgram({loadShader(GL_VERTEX_SHADER, "shaders/compiled/vertex/water_shader.spv"),
																		loadShader(GL_FRAGMENT_SHADER, "shaders/compiled/fragment/water_shader.spv")});

	glfwSetCursorPos(window, 1280.f / 2, 720.f / 2);

	auto constStartTime = std::chrono::system_clock::now();
	auto startTime = std::chrono::system_clock::now();

	float scale = 1.f;
	float normalStrength = 12.f;
	bool simulate = true;
	int selection = 0;

	int inputFormat = 0;
	int inputTexSize = 1;
	int inputPatchSize = 1500;

	float texCoordScale = 1.f;

	GLenum format = GL_LINEAR;
	int newSize = 512;

	struct plane_settings
	{
		int numX = 256, numY = 256;
		float interval = 0.09f;
	} ps;

	glm::mat4 view;
	glm::mat4 waterModel{1.f};
	glm::mat4 projection = glm::perspective(glm::radians(60.f), windowWidth * 1.f / windowHeight, 0.1f, 1000.f);

	glUseProgram(waterShader);
	glUniformMatrix4fv(2, 1, GL_FALSE, glm::value_ptr(projection));

	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();

		auto endTime = std::chrono::system_clock::now();
		std::chrono::duration<float> elapsed = endTime - startTime;
		auto delta = elapsed.count();
		startTime = endTime;

		if (i.captureCursor)
		{
			view = computeViewMatrix(window, delta, 1.f, 10.f);
			ImGui::SetMouseCursor(ImGuiMouseCursor_None);
		}

		if (simulate)
		{
			spec.updateSpectrumTexture();
			spec.fft();
			spec.combineTextures(scale);
		}

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		ImGui_ImplGlfw_NewFrame();
		ImGui_ImplOpenGL3_NewFrame();

		ImGui::NewFrame();

		ImGui::Begin("Settings", NULL, ImGuiWindowFlags_AlwaysAutoResize);

		ImGui::Text("Press E to release the mouse cursor");

		if (ImGui::CollapsingHeader("Spectrum Textures"))
		{
			if (ImGui::Combo("Min/Mag Filter", &inputFormat, "GL_LINEAR\0GL_NEAREST\0"))
			{
				switch (inputFormat)
				{
				case 0:
					format = GL_LINEAR;
					break;
				case 1:
					format = GL_NEAREST;
				}
			}

			if (ImGui::Combo("Size", &inputTexSize, "256x256\000512x512\0001024x1024\0"))
			{
				switch (inputTexSize)
				{
				case 0:
					newSize = 256;
					break;
				case 1:
					newSize = 512;
					break;
				case 2:
					newSize = 1024;
				}
			}

			ImGui::InputScalar("Patch Size", ImGuiDataType_S32, &inputPatchSize);

			if (ImGui::Button("Regenerate Textures"))
			{
				spec.regen(newSize, inputPatchSize);
			}
		}

		if (ImGui::CollapsingHeader("Geometry"))
		{
			ImGui::Text("Size of one chunk");
			ImGui::InputScalar("Width", ImGuiDataType_S32, &ps.numX);
			ImGui::InputScalar("Height", ImGuiDataType_S32, &ps.numY);
			ImGui::InputFloat("Interval", &ps.interval);

			if (ImGui::Button("Regenerate Geometry"))
			{
				// waterPlane.regenGeometry(ps.numX, ps.numY, ps.interval);
			}
		}

		if (ImGui::CollapsingHeader("Misc"))
		{
			ImGui::SliderFloat("Scale", &scale, 0.f, 10.f);
			ImGui::SliderFloat("Normal Strength", &normalStrength, 0.f, 50.f);
			ImGui::SliderFloat("Tex Coord Scale", &texCoordScale, 1.f, 50.f);
			ImGui::Checkbox("Simulate", &simulate);
		}
		ImGui::End();

		const char *selections = "DyDx\0DzDzx\0DyxDyz\0DxxDzz\0Buffer\0Displacements\0Derivatices\0Initial Spectrum\0";

		ImGui::Begin("Debug Image", NULL, ImGuiWindowFlags_AlwaysAutoResize);
		ImGui::Combo("Select Image", &selection, selections);
		ImGui::Image(reinterpret_cast<void *>(spec.getTexture((SpectrumTextures)selection)), ImVec2(256.f, 256.f));
		ImGui::End();

		ImGui::Render();

		// Begin water plane
		glUseProgram(waterShader);
		glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(waterModel));
		glUniformMatrix4fv(1, 1, GL_FALSE, glm::value_ptr(view));
		glUniform3f(3, camPos.x, camPos.y, camPos.z);
		glUniform3f(4, 20.f, 5.f, 2.f);
		glUniform1f(5, normalStrength);
		glUniform1f(6, texCoordScale);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, displacementsTex);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, derivatesTex);

		waterPlane.draw();
		// end water plane

		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(window);
	}

	spec.cleanup();
	waterPlane.destroy();
}

void GLWaves::destroy()
{
	ImGui_ImplGlfw_Shutdown();
	ImGui_ImplOpenGL3_Shutdown();
	ImGui::DestroyContext();

	glfwTerminate();
}

glm::mat4 GLWaves::computeViewMatrix(GLFWwindow *win, float delta, float mouseSpeed, float camSpeed)
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
	if (i.q)
		camPos += up * camSpeed * delta;
	if (i.c)
		camPos += -up * camSpeed * delta;

	return glm::lookAt(camPos, camPos + dir, up);
}

void GLWaves::debugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
														const GLchar *message, const void *userParam)
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
	else if (severity == GL_DEBUG_SEVERITY_HIGH || severity == GL_DEBUG_SEVERITY_MEDIUM)
	{
		std::cout << message << std::endl << std::endl;
	}
}

void GLWaves::keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
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

	if (key == GLFW_KEY_Q && action == GLFW_PRESS)
		i.q = true;
	else if (key == GLFW_KEY_Q && action == GLFW_RELEASE)
		i.q = false;

	if (key == GLFW_KEY_C && action == GLFW_PRESS)
		i.c = true;
	else if (key == GLFW_KEY_C && action == GLFW_RELEASE)
		i.c = false;

	if (key == GLFW_KEY_E && action == GLFW_PRESS)
	{
		i.captureCursor = !i.captureCursor;
		glfwSetCursorPos(window, 1280.f / 2, 720.f / 2);
	}
}
