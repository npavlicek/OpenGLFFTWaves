#version 460 core

layout(location = 0) out vec4 FragColor;

layout(location = 0) in vec3 texCoord;

layout(location = 2) uniform samplerCube skybox;

void main() {
	FragColor = texture(skybox, texCoord);
}
