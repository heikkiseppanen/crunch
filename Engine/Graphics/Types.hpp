#pragma once

#include "Crunch.hpp"

#include "Shared/Math.hpp"

namespace Cr::Graphics {

// TODO On longterm, adding a typesafe wrapping would be wise.
using MeshID    = u32;
using ShaderID  = u32;
using TextureID = u32;

using BufferID  = u32;
using ImageID   = u32;

struct Mesh
{
    BufferID vertex_buffer_id;
    BufferID index_buffer_id;
    u32 index_count;
};

struct Vertex
{
    Vec3f position;
    Vec2f uv;
};

struct Texture
{
    ImageID image_id;
    u16 width;
    u16 height;
};

// Has a 128-bytes minimum support
struct PushConstantObject
{
    alignas(16) Mat4f model; // 64-bytes
};

struct UniformBufferObject
{
    alignas(16) Mat4f projected_view;
};

}; // Cr::Graphics
