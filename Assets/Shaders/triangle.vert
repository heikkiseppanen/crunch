#version 450

layout(binding = 0) uniform FrameData
{
    mat4 projected_view;
} frame;

layout(push_constant) uniform InstanceData
{
    mat4 model;
} instance;

layout(location = 0) in vec3 local_position_in;
layout(location = 1) in vec2 uv_in;

layout(location = 0) out vec2 uv_out;
layout(location = 1) out vec3 world_position_out;

void main()
{
    const vec4 world_position = instance.model * vec4(local_position_in, 1.0f);

    gl_Position = frame.projected_view * world_position;

    world_position_out = world_position.xyz;
    uv_out = uv_in;
}

