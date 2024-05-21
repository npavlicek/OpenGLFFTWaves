#pragma once

#include <glm/glm.hpp>

class Skybox
{
public:
	Skybox();
	void init();
	void render(glm::mat4 view, glm::mat4 proj);
	void cleanup();
};
