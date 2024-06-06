#pragma once

#include <Crunch.hpp>

namespace Cr::Graphics {

// TODO On longterm, adding a typesafe wrapping would be wise.
using MeshID    = u32;
using ShaderID  = u32;
using TextureID = u32;

using BufferID  = u32;
using ImageID   = u32;

enum class BufferType { VERTEX, INDEX, STAGING };

}; // Cr::Graphics
