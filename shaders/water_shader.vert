#version 460 core

layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 texCoord;

layout(location = 0) out vec3 outNorm;
layout(location = 1) out vec3 fragPos;
layout(location = 2) out vec2 outTexCoord;

layout(location = 0) uniform mat4 model;
layout(location = 1) uniform mat4 view;
layout(location = 2) uniform mat4 projection;


layout(binding = 0) uniform sampler2D DisplacementsTex;

void main()
{
	vec4 displacement = texture(DisplacementsTex, texCoord);

	outTexCoord = texCoord;
	fragPos = vec3(model * vec4(pos, 1));
	gl_Position = projection * view * model * vec4(pos + displacement.xyz, 1);
}
