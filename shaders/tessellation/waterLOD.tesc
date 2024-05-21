#version 460 core

layout(vertices = 4) out;

layout(location = 0) in vec2 inTexCoord[];
layout(location = 1) in vec3 inFragPos[];

layout(location = 0) out vec2 texCoord[];
layout(location = 1) out vec3 fragPos[];

layout(location = 3) uniform vec3 camPos;

layout(location = 0) uniform mat4 model;
layout(location = 1) uniform mat4 view;
layout(location = 2) uniform mat4 projection;

layout(location = 7) uniform float minDistance;
layout(location = 8) uniform float maxDistance;
layout(location = 9) uniform int minTessLevel;
layout(location = 10) uniform int maxTessLevel;

void main()
{
	gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
	texCoord[gl_InvocationID] = inTexCoord[gl_InvocationID];
	fragPos[gl_InvocationID] = inFragPos[gl_InvocationID];

	if (gl_InvocationID == 0)
	{
		const int MIN_TESS_LEVEL = minTessLevel;
		const int MAX_TESS_LEVEL = maxTessLevel;
		const float MIN_DISTANCE = minDistance;
		const float MAX_DISTANCE = maxDistance;

		vec3 worldSpace1 = inFragPos[0];
		vec3 worldSpace2 = inFragPos[1];
		vec3 worldSpace3 = inFragPos[2];
		vec3 worldSpace4 = inFragPos[3];

		float distance1 = length(worldSpace1 - camPos);
		float distance2 = length(worldSpace2 - camPos);
		float distance3 = length(worldSpace3 - camPos);
		float distance4 = length(worldSpace4 - camPos);

		distance1 = clamp((distance1-MIN_DISTANCE) / (MAX_DISTANCE - MIN_DISTANCE), 0.0, 1.0);
		distance2 = clamp((distance2-MIN_DISTANCE) / (MAX_DISTANCE - MIN_DISTANCE), 0.0, 1.0);
		distance3 = clamp((distance3-MIN_DISTANCE) / (MAX_DISTANCE - MIN_DISTANCE), 0.0, 1.0);
		distance4 = clamp((distance4-MIN_DISTANCE) / (MAX_DISTANCE - MIN_DISTANCE), 0.0, 1.0);

		float tessLevel1 = mix(MAX_TESS_LEVEL, MIN_TESS_LEVEL, min(distance1, distance2));
		float tessLevel2 = mix(MAX_TESS_LEVEL, MIN_TESS_LEVEL, min(distance1, distance4));
		float tessLevel3 = mix(MAX_TESS_LEVEL, MIN_TESS_LEVEL, min(distance3, distance4));
		float tessLevel4 = mix(MAX_TESS_LEVEL, MIN_TESS_LEVEL, min(distance2, distance3));

		gl_TessLevelOuter[0] = tessLevel1;
		gl_TessLevelOuter[1] = tessLevel2;
		gl_TessLevelOuter[2] = tessLevel3;
		gl_TessLevelOuter[3] = tessLevel4;

		gl_TessLevelInner[0] = max(tessLevel2, tessLevel4);
		gl_TessLevelInner[1] = max(tessLevel1, tessLevel3);
	}
}

