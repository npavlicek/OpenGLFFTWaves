#version 460 core

layout(location = 0) in vec2 texCoord;
layout(location = 1) in vec3 fragPos;

layout(location = 0) out vec4 color;

layout(binding = 1) uniform sampler2D DerivativesTex;
layout(binding = 3) uniform sampler2D DerivativesTex2;
layout(binding = 5) uniform sampler2D DerivativesTex3;

// World Pos
layout(location = 3) uniform vec3 camPos;
layout(location = 4) uniform vec3 lightPos;
layout(location = 5) uniform float normalStrength;
layout(location = 6) uniform float texCoordScale;
layout(location = 12) uniform float texCoordScale2;
layout(location = 13) uniform float texCoordScale3;

void main() {
	vec4 derivative = texture(DerivativesTex, fragPos.xz / texCoordScale);
	vec4 derivative2 = texture(DerivativesTex2, fragPos.xz / texCoordScale2);
	vec4 derivative3 = texture(DerivativesTex3, fragPos.xz / texCoordScale3);
	vec3 norm1 = normalize(vec3(normalStrength * derivative.x / (1 + derivative.z), 1, normalStrength * derivative.y / (1 + derivative.w)));
	vec3 norm2 = normalize(vec3(normalStrength * derivative2.x / (1 + derivative2.z), 1, normalStrength * derivative2.y / (1 + derivative2.w)));
	vec3 norm3 = normalize(vec3(normalStrength * derivative3.x / (1 + derivative3.z), 1, normalStrength * derivative3.y / (1 + derivative3.w)));

	vec3 norm = norm1 + norm2 + norm3;
	norm = normalize(norm);

	float J1 = (1 + derivative.z) * (1 + derivative.w) - derivative.y * derivative.y;
	float J2 = (1 + derivative2.z) * (1 + derivative2.w) - derivative2.y * derivative2.y;
	float J3 = (1 + derivative3.z) * (1 + derivative3.w) - derivative3.y * derivative3.y;
	float J = J1 + J2 + J3;
	J *= -1;
	J += 0.2;
	J = clamp(J, 0.0, 1.0);

	// Frag Space
	vec3 lightDir = normalize(lightPos - fragPos);
	vec3 camDir = normalize(camPos - fragPos);
	vec3 halfway = normalize(lightDir + camDir);

	vec3 diffuseDir = normalize(vec3(10.0, 10.0, 0.0));

	vec3 spec = pow(clamp(dot(norm, halfway), 0.0, 1.0), 16) * vec3(0.8);
	vec3 ambient = vec3(0.0, 0.15, 0.6) * 0.2;
	vec3 diffuse = clamp(dot(diffuseDir, norm), 0.0, 1.0) * vec3(0.5);

	//vec3 light_color = vec3(0.5, 0.3, 0.0);
	vec3 light_color = vec3(0.5);

	spec *= light_color;
	ambient *= light_color;
	diffuse *= light_color;

	vec4 foamColor = vec4(1, 1, 1, 1);
	vec4 tmpCol = vec4(vec3(0.0, 0.07, 0.20) + diffuse + ambient + spec, 1);
	color = mix(tmpCol, foamColor, J);

	//color = vec4(vec3(0.0, 0.15, 0.3) + diffuse + ambient + spec, 1);
	//color = vec4(15 * norm.x, norm.y * 0.4, 15 * norm.z, 1);
	// color = vec4(norm.xyz, 1);
}
