#version 450

layout(binding = 1) uniform sampler2D tex;
layout(location = 0) in vec2 uv_in;

layout(location = 0) out vec4 color_out;

void main()
{
	color_out = texture(tex, uv_in);
}
