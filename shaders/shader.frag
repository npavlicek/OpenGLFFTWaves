#version 460 core

layout(location = 0) in vec3 outNorm;

layout(location = 0) out vec4 color;

void main() {
	vec3 lightDir = normalize(vec3(5.0, -1.0, 0));

	vec3 ambient = vec3(0.0, 0.35, 0.8) * 0.5;
	vec3 diffuse = clamp(dot(-lightDir, outNorm), 0.0, 1.0) * vec3(0.5);

	color = vec4(ambient + diffuse, 1);
}
