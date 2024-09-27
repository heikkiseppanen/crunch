#version 450

layout(binding = 1) uniform sampler2D tex;

layout(location = 0) in vec2 texture_coordinate;
layout(location = 1) in vec3 world_position;
layout(location = 2) in vec3 frag_normal;

layout(location = 0) out vec4 color_out;

void main()
{
    vec3 color = texture(tex, texture_coordinate).xyz * dot(frag_normal, normalize(vec3(5.0f, 5.0f, 0.0f)));

    color_out = vec4(color, 1.0f);
}
