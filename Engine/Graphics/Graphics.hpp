#pragma once

#include "Crunch/Crunch.hpp"
#include "Crunch/Math.hpp"

#include <vulkan/vulkan.h>

namespace Cr::Graphics {

enum class Format : U16
{
    UNDEFINED = 0,

    FLOAT_R32G32B32A32,
    FLOAT_R32G32B32,
    FLOAT_R32G32,
    FLOAT_R32,
};

enum class ShaderStage : U32
{
    UNDEFINED = 0,

    VERTEX,
    FRAGMENT,
};

// Has a 128-bytes minimum support
struct PushConstantObject
{
    alignas(16) Vec4f position;
    alignas(16) Vec4f scale;
    alignas(16) Quatf rotation;
};

struct UniformBufferObject
{
    alignas(16) Mat4f projected_view;
};

}; // Cr::Graphics
