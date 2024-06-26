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

    std::vector<Vertex> get_cube_vertices(f32 dimensions, u32 subdivision);
    std::vector<u32>    get_cube_indices(u32 subdivision);

    std::vector<Vertex> get_quad_sphere_vertices(f32 dimensions, u32 subdivision);
    std::vector<u32>    get_quad_sphere_indices(u32 subdivision);

}
