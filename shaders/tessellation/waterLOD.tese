#version 460 core

layout(quads, equal_spacing, ccw) in;

layout(location = 0) in vec2 inTexCoord[];
layout(location = 1) in vec3 inFragPos[];

layout(location = 0) out vec2 texCoord;
layout(location = 1) out vec3 fragPos;

layout(location = 0) uniform mat4 model;
layout(location = 1) uniform mat4 view;
layout(location = 2) uniform mat4 projection;

layout(location = 6) uniform float texCoordScale;

layout(location = 11) uniform float displacementScaleFactor;

layout(binding = 0) uniform sampler2D DisplacementsTex;

void main()
{
	float tc_x = gl_TessCoord.x;
	float tc_y = gl_TessCoord.y;

	vec3 pos1 = gl_in[0].gl_Position.xyz; // Top left
	vec3 pos2 = gl_in[1].gl_Position.xyz; // Bottom left
	vec3 pos3 = gl_in[2].gl_Position.xyz; // Bottom right
	vec3 pos4 = gl_in[3].gl_Position.xyz; // Top right

	vec3 i_pos1 = (pos2 - pos1) * tc_y + pos1;
	vec3 i_pos2 = (pos3 - pos4) * tc_y + pos4;
	vec3 newPos = (i_pos2 - i_pos1) * tc_x + i_pos1;

	vec2 tc1 = inTexCoord[0];
	vec2 tc2 = inTexCoord[1];
	vec2 tc3 = inTexCoord[2];
	vec2 tc4 = inTexCoord[3];

	vec2 i_tc1 = (tc4 - tc1) * tc_x + tc1;
	vec2 i_tc2 = (tc3 - tc2) * tc_x + tc2;
	texCoord = (i_tc2 - i_tc1) * tc_y + i_tc1;

	fragPos = vec3(model * vec4(newPos, 1));

	vec4 displacement = inverse(model) * texture(DisplacementsTex, fragPos.xz / texCoordScale);
	displacement.y *= displacementScaleFactor;

	gl_Position = projection * view * model * vec4(newPos + displacement.xyz, 1);
}
