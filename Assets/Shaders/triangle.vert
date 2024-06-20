#version 450

layout(push_constant) uniform InstanceData
{
    mat4 model;
} instance;

layout(binding = 0) uniform FrameData
{
    mat4 projected_view;
} frame;

layout(location = 0) in vec3 position_in;
layout(location = 1) in vec2 uv_in;

layout(location = 0) out vec2 uv_out;

void main()
{
    gl_Position = frame.projected_view * instance.model * vec4(position_in, 1.0f);
    uv_out = uv_in;
}

