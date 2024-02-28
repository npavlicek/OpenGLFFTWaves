#version 460 core

layout(vertices = 4) out;

layout(location = 0) in vec2 inTexCoord[];
layout(location = 1) in vec3 inFragPos[];

layout(location = 0) out vec2 texCoord[];
layout(location = 1) out vec3 fragPos[];

void main()
{
	gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
	texCoord[gl_InvocationID] = inTexCoord[gl_InvocationID];
	fragPos[gl_InvocationID] = inFragPos[gl_InvocationID];

	if (gl_InvocationID == 0)
	{
		gl_TessLevelOuter[0] = 5;
		gl_TessLevelOuter[1] = 5;
		gl_TessLevelOuter[2] = 5;
		gl_TessLevelOuter[3] = 5;

		gl_TessLevelInner[0] = 5;
		gl_TessLevelInner[1] = 5;
	}
}

