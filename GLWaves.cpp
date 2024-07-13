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
#include "WaveSettings.hpp"
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
		loadShader(GL_VERTEX_SHADER, "./shaders/compiled/vertex/water_shader.spv"),
		loadShader(GL_FRAGMENT_SHADER, "./shaders/compiled/fragment/water_shader.spv"),
		loadShader(GL_TESS_CONTROL_SHADER, "./shaders/compiled/tcs/waterLOD.spv"),
		loadShader(GL_TESS_EVALUATION_SHADER, "./shaders/compiled/tes/waterLOD.spv")
	};
	// clang-format on

	return linkProgram(waterShaders);
}

void GLWaves::loop()
{
	WaveSettings ws;
	ws.load();

	Spectrum cascade1{};
	Spectrum cascade2{};
	Spectrum cascade3{};
	cascade1.init(ws.casc.resolution,
	              ws.casc.patch_size,
	              ws.casc.horizontal_displacement_scale,
	              ws.casc.depth,
	              ws.casc.fetch,
	              ws.casc.wind_speed,
	              ws.casc.cutoffLow,
	              ws.casc.cutoffHigh);

	GLuint displacementsTex = cascade1.getTexture(Displacements);
	GLuint derivatesTex = cascade1.getTexture(Derivates);

	GLuint waterShaderProgram = reloadShaders();

	Plane waterPlane(ws.plane.size, ws.plane.sqrt_num_instances, ws.plane.lod);
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
				speed = ws.cam.speed;
			else
				speed = ws.cam.speed * ws.cam.sprint_factor;

			view = computeViewMatrix(window, delta, 1.f, speed);
			ImGui::SetMouseCursor(ImGuiMouseCursor_None);
		}

		if (ws.debug.simulate_ocean)
		{
			cascade1.updateSpectrumTexture();
			cascade1.fft();
			cascade1.combineTextures(ws.casc.horizontal_displacement_scale);
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
			if (ImGui::Combo("Size", &ws.casc.select_tex_res, "256x256\000512x512\0001024x1024\0"))
			{
				switch (ws.casc.select_tex_res)
				{
				case 0:
					ws.casc.resolution = 256;
					break;
				case 1:
					ws.casc.resolution = 512;
					break;
				case 2:
					ws.casc.resolution = 1024;
				}
			}

			ImGui::InputScalar("Patch Size", ImGuiDataType_S32, &ws.casc.patch_size);
			ImGui::SliderFloat("Depth", &ws.casc.depth, 1.f, 100.f);
			ImGui::SliderFloat("Fetch", &ws.casc.fetch, 1.f, 100.f);
			ImGui::SliderFloat("Wind Speed", &ws.casc.wind_speed, 1.f, 100.f);
			ImGui::SliderFloat("Cutoff Low", &ws.casc.cutoffLow, 1.f, 100.f);
			ImGui::SliderFloat("Cutoff High", &ws.casc.cutoffHigh, 1.f, 100.f);

			if (ImGui::Button("Regenerate Textures"))
			{
				cascade1.regen(ws.casc.resolution,
				               ws.casc.patch_size,
				               ws.casc.horizontal_displacement_scale,
				               ws.casc.depth,
				               ws.casc.fetch,
				               ws.casc.wind_speed,
				               ws.casc.cutoffLow,
				               ws.casc.cutoffHigh);
			}
		}

		if (ImGui::CollapsingHeader("Geometry"))
		{
			ImGui::InputFloat("Chunk Length", &ws.plane.size);
			ImGui::InputInt("Sqrt # of Chunks", &ws.plane.sqrt_num_instances);
			ImGui::InputInt("LOD", &ws.plane.lod);

			if (ImGui::Button("Regenerate Geometry"))
			{
				waterPlane.regenGeometry(ws.plane.size, ws.plane.sqrt_num_instances, ws.plane.lod);
			}
		}

		if (ImGui::CollapsingHeader("Tessellation"))
		{
			ImGui::SliderInt("Minimum Level", &ws.tess.min_level, 1, 64);
			ImGui::SliderInt("Maximum Level", &ws.tess.max_level, 1, 64);

			ImGui::SliderFloat("Minimum Distance", &ws.tess.min_distance, 0, 1000);
			ImGui::SliderFloat("Maximum Distance", &ws.tess.max_distance, 0, 1000);

			ImGui::Checkbox("Tess Follow Cam", reinterpret_cast<bool *>(&ws.debug.tess_follow_cam));
		}

		if (ImGui::CollapsingHeader("Camera"))
		{
			ImGui::SliderFloat("Speed", &ws.cam.speed, 50.f, 500.f);
			ImGui::SliderFloat("Sprint Factor", &ws.cam.sprint_factor, 1.f, 50.f);
		}

		if (ImGui::CollapsingHeader("Misc"))
		{
			ImGui::SliderFloat("Scale", &ws.casc.horizontal_displacement_scale, 0.f, 10.f);
			ImGui::SliderFloat("Normal Strength", &ws.render.normal_strength, 0.f, 50.f);
			ImGui::SliderFloat("Tex Coord Scale", &ws.render.tex_coord_scale, 1.f, 10000.f);
			ImGui::SliderFloat("Displacement Scale Factor", &ws.casc.vertical_displacement_scale, 0.001f, 100.f);
			ImGui::Checkbox("Simulate", reinterpret_cast<bool *>(&ws.debug.simulate_ocean));
			ImGui::Checkbox("Render Cubemap", reinterpret_cast<bool *>(&ws.debug.render_skybox));
			ImGui::Checkbox("Render Water", reinterpret_cast<bool *>(&ws.debug.render_ocean));
			ImGui::Checkbox("Render Wireframe", reinterpret_cast<bool *>(&ws.debug.render_wireframe));

			if (ImGui::Button("Reload Shaders"))
			{
				std::cout << "Reloading shaders..." << std::endl;
				if (waterShaderProgram)
				{
					glDeleteProgram(waterShaderProgram);
				}
				waterShaderProgram = reloadShaders();
				glUseProgram(waterShaderProgram);
				glUniformMatrix4fv(2, 1, GL_FALSE, glm::value_ptr(projection));
				cascade1.loadShaders();
			}
		}

		ImGui::End();

		const char *selections = "DyDx\0DzDzx\0DyxDyz\0DxxDzz\0Buffer\0Displacements\0Derivatices\0Initial Spectrum\0";

		ImGui::Begin("Debug Image", NULL, ImGuiWindowFlags_AlwaysAutoResize);
		ImGui::Combo("Select Image", &ws.debug.image_select, selections);
		ImGui::Image(reinterpret_cast<void *>(cascade1.getTexture((SpectrumTextures)ws.debug.image_select)),
		             ImVec2(512.f, 512.f));
		ImGui::End();

		ImGui::Render();

		// Begin water plane
		glUseProgram(waterShaderProgram);
		glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(waterModel));
		glUniformMatrix4fv(1, 1, GL_FALSE, glm::value_ptr(view));
		if (ws.debug.tess_follow_cam)
			glUniform3f(3, camPos.x, camPos.y, camPos.z);
		else
			glUniform3f(3, 0.f, 0.f, 0.f);
		glUniform3f(4, 20.f, 5.f, 2.f);
		glUniform1f(5, ws.render.normal_strength);
		glUniform1f(6, ws.render.tex_coord_scale);

		glUniform1f(7, ws.tess.min_distance);
		glUniform1f(8, ws.tess.max_distance);
		glUniform1i(9, ws.tess.min_level);
		glUniform1i(10, ws.tess.max_level);

		glUniform1f(11, ws.casc.vertical_displacement_scale);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, displacementsTex);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, derivatesTex);

		if (ws.debug.render_wireframe)
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		else
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		if (ws.debug.render_ocean)
			waterPlane.draw();
		// end water plane

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		if (ws.debug.render_skybox)
			sb.render(view, projection);

		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(window);
	}

	ws.save();

	sb.cleanup();
	cascade1.cleanup();
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

void GLWaves::debugCallback(GLenum source,
                            GLenum type,
                            GLuint id,
                            GLenum severity,
                            GLsizei length,
                            const GLchar *message,
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
