#version 460 core

layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 inTexCoord;

layout(location = 0) uniform mat4 model;
layout(location = 1) uniform mat4 view;
layout(location = 2) uniform mat4 projection;

layout(location = 0) out vec2 texCoord;

void main(void)
{
	texCoord = inTexCoord;
	gl_Position = projection * view * model * vec4(pos, 1);
}
