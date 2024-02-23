#version 460 core

layout(location = 0) in vec3 norm;

layout(location = 0) out vec4 color;

void main() {
	vec3 lightDir = normalize(vec3(13.5, -1.0, 0));

	vec3 ambient = vec3(0.0, 0.15, 0.6) * 0.25;
	vec3 diffuse = clamp(dot(-lightDir, norm), 0.0, 1.0) * vec3(2);

	color = vec4(diffuse + ambient, 1);
}
