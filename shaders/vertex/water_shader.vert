#version 460 core

layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 texCoord;
layout(location = 2) in vec2 offset;

layout(location = 0) out vec2 outTexCoord;
layout(location = 1) out vec3 fragPos;

layout(location = 0) uniform mat4 model;
layout(location = 1) uniform mat4 view;
layout(location = 2) uniform mat4 projection;

layout(location = 6) uniform float texCoordScale;

void main()
{
	vec3 offsetPos = vec3(pos.x + offset.x, pos.y, pos.z + offset.y);

	outTexCoord = texCoord / texCoordScale;
	fragPos = vec3(model * vec4(offsetPos, 1));
	gl_Position = vec4(offsetPos, 1);
}
