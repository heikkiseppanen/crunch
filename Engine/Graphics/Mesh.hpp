#pragma once

#include "Crunch.hpp"
#include "Shared/Math.hpp"

#include <vector>

namespace Cr
{
    struct Vertex
    {
        Vec3f position;
        Vec2f uv;
    };

    std::vector<Vertex> get_cube_vertices(f32 dimensions);
    std::vector<u32>    get_cube_indices();

}
