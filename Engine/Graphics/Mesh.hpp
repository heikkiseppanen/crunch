#pragma once

#include "Graphics/Graphics.hpp"

#include "Crunch/Math.hpp"
#include "Crunch/ClassUtility.hpp"

#include <span>

namespace Cr::Graphics
{

namespace Vulkan { class Buffer; }

struct Vertex
{
    Vec3f position;
    Vec2f uv;
    Vec3f normal;
};

//    class Mesh : public NoCopy
//    {
//        public:
//            Mesh(Unique<Vulkan::Buffer> buffer, u32 index_count);
//            ~Mesh();
//
//        private:
//            Unique<Vulkan::Buffer> m_buffer;
//
//            u32 m_index_start;
//            u32 m_index_count;
//    };

    std::vector<Vertex> get_cube_vertices(F32 dimensions, U32 subdivision);
    std::vector<U32>    get_cube_indices(U32 subdivision);

    std::vector<Vertex> get_quad_sphere_vertices(F32 dimensions, U32 subdivision);
    std::vector<U32>    get_quad_sphere_indices(U32 subdivision);
}
