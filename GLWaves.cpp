#include "GLWaves.hpp"

#include <cstdint>
#include <fstream>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
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
#include "Skybox.hpp"
#include "Spectrum.hpp"
#include "plane.hpp"

input i;

Skybox sb;

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

	sb.init();

	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageCallback(debugCallback, nullptr);

	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CW);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
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

	window = glfwCreateWindow(windowWidth, windowHeight, "OpenGL FFT Waves", nullptr, nullptr);
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

GLuint reloadShaders()
{
	// clang-format off
	std::vector<GLuint> waterShaders = {
		loadShader(GL_VERTEX_SHADER, "shaders/compiled/vertex/water_shader.spv"),
		loadShader(GL_FRAGMENT_SHADER, "shaders/compiled/fragment/water_shader.spv"),
		loadShader(GL_TESS_CONTROL_SHADER, "shaders/compiled/tcs/waterLOD.spv"),
		loadShader(GL_TESS_EVALUATION_SHADER, "shaders/compiled/tes/waterLOD.spv")
	};
	// clang-format on

	return linkProgram(waterShaders);
}

void GLWaves::loop()
{
	Spectrum spec{};
	spec.init(1024, 250);

	GLuint displacementsTex = spec.getTexture(Displacements);
	GLuint derivatesTex = spec.getTexture(Derivates);

	GLuint waterShaderProgram = reloadShaders();

	Plane waterPlane(settings.ps.size, settings.ps.sqrtOfInstances, settings.ps.lod);
	waterPlane.init();

	glfwSetCursorPos(window, 1280.f / 2, 720.f / 2);

	auto constStartTime = std::chrono::system_clock::now();
	auto startTime = std::chrono::system_clock::now();

	glm::mat4 view;
	glm::mat4 waterModel = glm::identity<glm::mat4>();
	glm::mat4 projection = glm::perspective(glm::radians(60.f), windowWidth * 1.f / windowHeight, 0.1f, 10000.f);

	glUseProgram(waterShaderProgram);
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
			float speed = 0.f;
			if (!i.shift)
				speed = settings.cam.speed;
			else
				speed = settings.cam.speed * settings.cam.sprintFactor;

			view = computeViewMatrix(window, delta, 1.f, speed);
			ImGui::SetMouseCursor(ImGuiMouseCursor_None);
		}

		if (settings.wave.simulate)
		{
			spec.updateSpectrumTexture();
			spec.fft();
			spec.combineTextures(settings.wave.scale);
		}

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		ImGui_ImplGlfw_NewFrame();
		ImGui_ImplOpenGL3_NewFrame();

		ImGui::NewFrame();

		ImGui::Begin("Settings", NULL, ImGuiWindowFlags_AlwaysAutoResize);

		ImGui::Text("Press E to release the mouse cursor");

		ImGui::Text("Cam Pos: %f %f %f", camPos.x, camPos.y, camPos.z);

		if (ImGui::CollapsingHeader("Spectrum Textures"))
		{
			if (ImGui::Combo("Size", &settings.wave.inputTexSize, "256x256\000512x512\0001024x1024\0"))
			{
				switch (settings.wave.inputTexSize)
				{
				case 0:
					settings.wave.newSize = 256;
					break;
				case 1:
					settings.wave.newSize = 512;
					break;
				case 2:
					settings.wave.newSize = 1024;
				}
			}

			ImGui::InputScalar("Patch Size", ImGuiDataType_S32, &settings.wave.inputPatchSize);

			ImGui::SliderFloat("Spectrum Scale", &settings.wave.specScale, 1.f, 100.f);

			if (ImGui::Button("Regenerate Textures"))
			{
				spec.regen(settings.wave.newSize, settings.wave.inputPatchSize, settings.wave.specScale);
			}
		}

		if (ImGui::CollapsingHeader("Geometry"))
		{
			ImGui::InputFloat("Chunk Length", &settings.ps.size);
			ImGui::InputInt("Sqrt # of Chunks", &settings.ps.sqrtOfInstances);
			ImGui::InputInt("LOD", &settings.ps.lod);

			if (ImGui::Button("Regenerate Geometry"))
			{
				waterPlane.regenGeometry(settings.ps.size, settings.ps.sqrtOfInstances, settings.ps.lod);
			}
		}

		if (ImGui::CollapsingHeader("Tessellation"))
		{
			ImGui::SliderInt("Minimum Level", &settings.tess.minTessLevel, 1, 64);
			ImGui::SliderInt("Maximum Level", &settings.tess.maxTessLevel, 1, 64);

			ImGui::SliderFloat("Minimum Distance", &settings.tess.minDistance, 0, 1000);
			ImGui::SliderFloat("Maximum Distance", &settings.tess.maxDistance, 0, 1000);

			ImGui::Checkbox("Tess Follow Cam", &settings.tess.tessFollowCam);
		}

		if (ImGui::CollapsingHeader("Camera")) {
			ImGui::SliderFloat("Speed", &settings.cam.speed, 50.f, 500.f);
			ImGui::SliderFloat("Sprint Factor", &settings.cam.sprintFactor, 1.f, 50.f);
		}

		if (ImGui::CollapsingHeader("Misc"))
		{
			ImGui::SliderFloat("Scale", &settings.wave.scale, 0.f, 10.f);
			ImGui::SliderFloat("Normal Strength", &settings.wave.normalStrength, 0.f, 50.f);
			ImGui::SliderFloat("Tex Coord Scale", &settings.wave.texCoordScale, 1.f, 50.f);
			ImGui::Checkbox("Simulate", &settings.wave.simulate);
			ImGui::Checkbox("Render Cubemap", &settings.wave.cubemap);
			ImGui::Checkbox("Render Water", &settings.wave.renderWater);
			ImGui::Checkbox("Render Wireframe", &settings.wave.renderWireframe);

			if (ImGui::Button("Reload Shaders"))
			{
				std::cout << "Reloading shaders..." << std::endl;
				if (waterShaderProgram) {
					glDeleteProgram(waterShaderProgram);
				}
				waterShaderProgram = reloadShaders();
				glUseProgram(waterShaderProgram);
				glUniformMatrix4fv(2, 1, GL_FALSE, glm::value_ptr(projection));
				spec.loadShaders();
			}
		}

		ImGui::End();

		const char *selections = "DyDx\0DzDzx\0DyxDyz\0DxxDzz\0Buffer\0Displacements\0Derivatices\0Initial Spectrum\0";

		ImGui::Begin("Debug Image", NULL, ImGuiWindowFlags_AlwaysAutoResize);
		ImGui::Combo("Select Image", &settings.wave.selection, selections);
		ImGui::Image(reinterpret_cast<void *>(spec.getTexture((SpectrumTextures)settings.wave.selection)),
								 ImVec2(512.f, 512.f));
		ImGui::End();

		ImGui::Render();

		// Begin water plane
		glUseProgram(waterShaderProgram);
		glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(waterModel));
		glUniformMatrix4fv(1, 1, GL_FALSE, glm::value_ptr(view));
		if (settings.tess.tessFollowCam)
			glUniform3f(3, camPos.x, camPos.y, camPos.z);
		else
			glUniform3f(3, 0.f, 0.f, 0.f);
		glUniform3f(4, 20.f, 5.f, 2.f);
		glUniform1f(5, settings.wave.normalStrength);
		glUniform1f(6, settings.wave.texCoordScale);

		glUniform1f(7, settings.tess.minDistance);
		glUniform1f(8, settings.tess.maxDistance);
		glUniform1i(9, settings.tess.minTessLevel);
		glUniform1i(10, settings.tess.maxTessLevel);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, displacementsTex);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, derivatesTex);

		if (settings.wave.renderWireframe)
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		else
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		if (settings.wave.renderWater)
			waterPlane.draw();
		// end water plane

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		if (settings.wave.cubemap)
			sb.render(view, projection);

		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(window);
	}

	sb.cleanup();
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

glm::mat4 GLWaves::computeViewMatrix(GLFWwindow *win, float delta, float mouseSpeed, float speed)
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
		camPos += dir * speed * delta;
	if (i.s)
		camPos -= dir * speed * delta;
	if (i.a)
		camPos -= glm::cross(dir, up) * speed * delta;
	if (i.d)
		camPos += glm::cross(dir, up) * speed * delta;
	if (i.q)
		camPos += up * speed * delta;
	if (i.c)
		camPos += -up * speed * delta;

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

	if (key == GLFW_KEY_R && action == GLFW_PRESS)
		i.r = true;
	else if (key == GLFW_KEY_R && action == GLFW_RELEASE)
		i.r = false;

	if (mods == GLFW_MOD_SHIFT)
		i.shift = true;
	else
		i.shift = false;

	if (key == GLFW_KEY_E && action == GLFW_PRESS)
	{
		i.captureCursor = !i.captureCursor;
		glfwSetCursorPos(window, 1280.f / 2, 720.f / 2);
	}
}
