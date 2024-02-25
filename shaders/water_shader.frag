#version 460 core

layout(location = 0) in vec3 norm;
layout(location = 1) in vec3 fragPos;

layout(location = 0) out vec4 color;

// World Pos
layout(location = 3) uniform vec3 camPos;
layout(location = 4) uniform vec3 lightPos;

void main() {
	// Frag Space
	vec3 lightDir = normalize(lightPos - fragPos);
	vec3 camDir = normalize(camPos - fragPos);
	vec3 halfway = normalize(lightDir + camDir);

	vec3 spec = pow(clamp(dot(norm, halfway), 0.0, 1.0), 64) * vec3(0.8);
	vec3 ambient = vec3(0.0, 0.15, 0.6) * 0.2;
	vec3 diffuse = clamp(dot(lightDir, norm), 0.0, 1.0) * vec3(0.5);

	//vec3 light_color = vec3(0.5, 0.3, 0.0);
	vec3 light_color = vec3(0.5);

	spec *= light_color;
	ambient *= light_color;
	diffuse *= light_color;

	color = vec4(vec3(0.0, 0.15, 0.3) + diffuse + ambient + spec, 1);
	//color = vec4(15 * norm.x, norm.y * 0.4, 15 * norm.z, 1);
}
