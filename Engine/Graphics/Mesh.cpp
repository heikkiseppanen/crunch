#include "Graphics/Mesh.hpp"

#include <array>

namespace Cr::Graphics
{
    inline constexpr U32 QUAD_VERTEX_COUNT = 4;
    inline constexpr U32 CUBE_SIDES = 6;

    static void generate_quad_vertices(Vec3f top_left, Vec3f top_right, Vec3f bottom_left, U32 edge_vertices, std::vector<Vertex>& output) 
    {
        const Vec3f step_right = (top_right   - top_left) / F32(edge_vertices - 1);
        const Vec3f step_down  = (bottom_left - top_left) / F32(edge_vertices - 1);

        const float uv_step { 1.0f / F32(edge_vertices - 1) };

        const Vec3f normal = -glm::normalize(glm::cross(top_right - top_left, bottom_left - top_left));

        for (U32 y = 0; y < edge_vertices; ++y)
        {
            Vec3f position = top_left + (step_down * F32(y));
            Vec2f uv { 0.0f, uv_step * F32(y) };  

            output.emplace_back(position, uv, normal);

            for (U32 x = 0; x < edge_vertices - 2; ++x)
            {
                position += step_right;
                uv.x     += uv_step;
                output.emplace_back(position, uv, normal);
            }

            position = top_right + (step_down * F32(y));
            uv.x = 1.0f;
            output.emplace_back(position, uv, normal);
        }
    }

    static void generate_quad_indices(U32 edge_vertex_count, U32 start, std::vector<U32>& output) 
    {
        const U32 edge_count = edge_vertex_count - 1;
        
        for (U32 y = 0; y < edge_count ; ++y)
        {
            const U32 i0 = start + y * edge_vertex_count;
            const U32 i1 = i0 + 1;
            const U32 i2 = i0 + edge_vertex_count;
            const U32 i3 = i2 + 1;

            std::array quad_indices { i0, i1, i3, i0, i3, i2 };

            for (U32 x = 0; x < edge_count ; ++x)
            {
                for (U32& index : quad_indices)
                {
                    output.push_back(index++);
                }
            }
        }
    }

    std::vector<Vertex> get_cube_vertices(F32 size, U32 subdivision)
    {
        const Vec3f min { -size / 2, -size / 2, -size / 2 }; 
        const Vec3f max {  size / 2,  size / 2,  size / 2 }; 

        const Vec3f luf { min.x, max.y, max.z };
        const Vec3f lub { min.x, max.y, min.z };
        const Vec3f ldf { min.x, min.y, max.z };
        const Vec3f ldb { min.x, min.y, min.z };

        const Vec3f ruf { max.x, max.y, max.z };
        const Vec3f rub { max.x, max.y, min.z };
        const Vec3f rdf { max.x, min.y, max.z };
        const Vec3f rdb { max.x, min.y, min.z };

        const U32 edge_vertex_count = glm::pow(2, subdivision) + 1;

        std::vector<Vertex> vertices;
        vertices.reserve(CUBE_SIDES * QUAD_VERTEX_COUNT);

        generate_quad_vertices(luf, ruf, ldf, edge_vertex_count, vertices); // Front
        generate_quad_vertices(rub, lub, rdb, edge_vertex_count, vertices); // Back
        generate_quad_vertices(lub, luf, ldb, edge_vertex_count, vertices); // Left
        generate_quad_vertices(ruf, rub, rdf, edge_vertex_count, vertices); // Right
        generate_quad_vertices(ldf, rdf, ldb, edge_vertex_count, vertices); // Bottom
        generate_quad_vertices(lub, rub, luf, edge_vertex_count, vertices); // Top

        return vertices;
    }

    std::vector<U32> get_cube_indices(U32 subdivision)
    {
        const U32 edge_vertex_count = glm::pow(2, subdivision) + 1;
        const U32 side_vertex_count = edge_vertex_count * edge_vertex_count;

        std::vector<U32> indices;
        indices.reserve(36);

        for (U32 side = 0; side < CUBE_SIDES; ++side)
        {
            generate_quad_indices(edge_vertex_count, side * side_vertex_count, indices);
        }

        return indices;
    }

    std::vector<Vertex> get_quad_sphere_vertices(F32 size, U32 subdivision) 
    {
        const F32 radius = size * 0.5f;

        std::vector<Vertex> vertices { get_cube_vertices(size, subdivision) };

        for (auto& vertex : vertices)
        {
            vertex.position = glm::normalize(vertex.position) * radius;
            vertex.normal   = glm::normalize(vertex.position);
        }

        return vertices;
    }

    std::vector<U32> get_quad_sphere_indices(U32 subdivision) 
    {
        return get_cube_indices(subdivision);
    }
}
