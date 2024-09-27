#version 450

layout(binding = 0) uniform FrameData
{
    mat4 projected_view;
} frame;

layout(push_constant) uniform InstanceData
{
    vec4 position;
    vec4 scale;
    vec4 rotation_quaternion;
} instance;

layout(location = 0) in vec3 local_position_in;
layout(location = 1) in vec2 uv_in;
layout(location = 2) in vec3 normal_in;

layout(location = 0) out vec2 uv_out;
layout(location = 1) out vec3 world_position_out;
layout(location = 2) out vec3 normal_out;

vec3 rotate_with_quaternion(vec3 v, vec4 q)
{
    const vec3 t = 2.0f * cross(q.xyz, v);
    return v + q.w * t + cross(q.xyz, t);
}

void main()
{
    const mat4 translate_scale = mat4(
        vec4(instance.scale.x, 0.0f,             0.0f,             0.0f),
        vec4(0.0f,             instance.scale.y, 0.0f,             0.0f),
        vec4(0.0f,             0.0f,             instance.scale.z, 0.0f),
        instance.position
    );

    const vec4 world_position = translate_scale * vec4(rotate_with_quaternion(local_position_in, instance.rotation_quaternion), 1.0f);

    gl_Position = frame.projected_view * world_position;

    world_position_out = world_position.xyz;
    normal_out = rotate_with_quaternion(normal_in, instance.rotation_quaternion);
    uv_out = uv_in;
}

