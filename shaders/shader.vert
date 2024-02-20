#version 460 core

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 norm;

layout(location = 0) out vec3 outNorm;

layout(std140, binding = 0) uniform mvp {
	mat4 model;
	mat4 view;
	mat4 projection;

	float time;
};

void main() {
	// float amplitude = 0.5;
	// float period = 5.0;
	// float freq = 2.0 * 3.14 / period;
	// float speed = 10.0;

	// float newY = 0;
	// float dy_dx = 0;

	// for (int i = 0; i < 100; i++) {
	// 	amplitude *= 0.8;
	// 	freq *= 1.12;

	// 	vec3 dir = vec3(cos(i * 3.14 / 180), sin(i * 3.14 / 180), 0);

	// 	float rotatedX = ((dot(dir, pos) / dot(dir, dir)) * dir).x;

	// 	newY += amplitude * sin(rotatedX * freq + 2 * time * speed / period);
	// 	dy_dx += amplitude * cos(rotatedX * freq + 2 * time * speed / period);
	// }

	// vec3 tangent = vec3(1.0, dy_dx, 0.0);
	// vec3 bitangent = vec3(0.0, 0.0, 1.0);

	// vec3 normal = normalize(cross(tangent, bitangent));

	outNorm = vec3(model * vec4(norm, 1));
	gl_Position = projection * view * model * vec4(pos, 1);
}
