#version 460 core

layout(location = 0) in vec2 texCoord;

layout(location = 0) out vec4 color;

layout(binding = 0) uniform sampler2D tex;

layout(location = 3) uniform float scale;

void main()
{
	color = scale * texture(tex, texCoord);
}
