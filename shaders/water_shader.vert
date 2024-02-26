#version 460 core

layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 texCoord;

layout(location = 0) out vec3 outNorm;
layout(location = 1) out vec3 fragPos;
layout(location = 2) out vec2 outTexCoord;

layout(location = 0) uniform mat4 model;
layout(location = 1) uniform mat4 view;
layout(location = 2) uniform mat4 projection;

layout(location = 6) uniform float texCoordScale;

layout(location = 7) uniform int sqrtInstanceCount;
layout(location = 8) uniform float sqrtChunkSize;

layout(binding = 0) uniform sampler2D DisplacementsTex;

void main()
{
	vec4 displacement = texture(DisplacementsTex, texCoord * texCoordScale);

	int row = gl_InstanceID % sqrtInstanceCount;
	int column = gl_InstanceID / sqrtInstanceCount;

	float offset = -sqrtChunkSize * sqrtInstanceCount / 2.0;
	float xOff = row * sqrtChunkSize + offset;
	float yOff = column * sqrtChunkSize + offset;

	vec3 offsetPos = vec3(pos.x + xOff, pos.y, pos.z + yOff);

	outTexCoord = texCoord * texCoordScale;
	fragPos = vec3(model * vec4(offsetPos, 1));
	gl_Position = projection * view * model * vec4(offsetPos + displacement.xyz, 1);
}
