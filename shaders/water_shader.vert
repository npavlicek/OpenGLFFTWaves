#version 460 core

layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 texCoord;

layout(location = 0) out vec3 outNorm;

layout(location = 0) uniform mat4 model;
layout(location = 1) uniform mat4 view;
layout(location = 2) uniform mat4 projection;

layout(binding = 0) uniform sampler2D DisplacementsTex;
layout(binding = 1) uniform sampler2D DerivativesTex;

void main()
{
	vec4 displacement = texture(DisplacementsTex, texCoord);
	vec4 derivative = texture(DerivativesTex, texCoord);

	vec3 norm = normalize(vec3(derivative.x / (1 + derivative.z), 1, derivative.y / (1 + derivative.w)));

	outNorm = vec3(model * vec4(norm, 1));
	gl_Position = projection * view * model * vec4(pos + displacement.xyz, 1);
}
