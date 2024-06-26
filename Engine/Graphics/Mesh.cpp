#include "Graphics/Mesh.hpp"

#include <array>

namespace Cr
{
    inline constexpr u32 QUAD_VERTEX_COUNT = 4;
    inline constexpr u32 CUBE_SIDES = 6;

    static void generate_quad_vertices(Vec3f top_left, Vec3f top_right, Vec3f bottom_left, u32 edge_vertices, std::vector<Vertex>& output) 
    {
        const Vec3f step_right = (top_right   - top_left) / f32(edge_vertices - 1);
        const Vec3f step_down  = (bottom_left - top_left) / f32(edge_vertices - 1);

        const float uv_step { 1.0f / f32(edge_vertices - 1) };

        for (u32 y = 0; y < edge_vertices; ++y)
        {
            Vec3f position = top_left + (step_down * f32(y));
            Vec2f uv { 0.0f, uv_step * f32(y) };  

            output.emplace_back(position, uv);

            for (u32 x = 0; x < edge_vertices - 2; ++x)
            {
                position += step_right;
                uv.x     += uv_step;
                output.emplace_back(Vertex{ position, uv });
            }

            position = top_right + (step_down * f32(y));
            uv.x = 1.0f;
            output.emplace_back(position, uv);
        }
    }

    static void generate_quad_indices(u32 edge_vertex_count, u32 start, std::vector<u32>& output) 
    {
        const u32 edge_count = edge_vertex_count - 1;
        
        for (u32 y = 0; y < edge_count ; ++y)
        {
            const u32 i0 = start + y * edge_vertex_count;
            const u32 i1 = i0 + 1;
            const u32 i2 = i0 + edge_vertex_count;
            const u32 i3 = i2 + 1;

            std::array quad_indices { i0, i1, i3, i0, i3, i2 };

            for (u32 x = 0; x < edge_count ; ++x)
            {
                for (u32& index : quad_indices)
                {
                    output.push_back(index++);
                }
            }
        }
    }

    std::vector<Vertex> get_cube_vertices(f32 size, u32 subdivision)
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

        const u32 edge_vertex_count = glm::pow(2, subdivision) + 1;

        std::vector<Vertex> vertices;
        vertices.reserve(CUBE_SIDES * QUAD_VERTEX_COUNT);

        generate_quad_vertices(lub, rub, ldb, edge_vertex_count, vertices); // Front
        generate_quad_vertices(ruf, luf, rdf, edge_vertex_count, vertices); // Back
        generate_quad_vertices(luf, lub, ldf, edge_vertex_count, vertices); // Left
        generate_quad_vertices(rub, ruf, rdb, edge_vertex_count, vertices); // Right
        generate_quad_vertices(ldb, rdb, ldf, edge_vertex_count, vertices); // Bottom
        generate_quad_vertices(luf, ruf, lub, edge_vertex_count, vertices); // Top
//        {
//            // Front
//            {lub, uv00},
//            {rub, uv10},
//            {rdb, uv11},
//            {ldb, uv01},
//
//            // Back
//            {ruf, uv00},
//            {luf, uv10},
//            {ldf, uv11},
//            {rdf, uv01},
//
//            // Left
//            {luf, uv00},
//            {lub, uv10},
//            {ldb, uv11},
//            {ldf, uv01},
//
//            // Right
//            {rub, uv00},
//            {ruf, uv10},
//            {rdf, uv11},
//            {rdb, uv01},
//
//            // Top
//            {luf, uv00},
//            {ruf, uv10},
//            {rub, uv11},
//            {lub, uv01},
//
//            // Bottom
//            {ldb, uv00},
//            {rdb, uv10},
//            {rdf, uv11},
//            {ldf, uv01},
//        };


        return vertices;
    }

    std::vector<u32> get_cube_indices(u32 subdivision)
    {
        const u32 edge_vertex_count = glm::pow(2, subdivision) + 1;
        const u32 side_vertex_count = edge_vertex_count * edge_vertex_count;

        std::vector<u32> indices;
        indices.reserve(36);

        for (u32 side = 0; side < CUBE_SIDES; ++side)
        {
            generate_quad_indices(edge_vertex_count, side * side_vertex_count, indices);
        }

        return indices;
    }

    std::vector<Vertex> get_quad_sphere_vertices(f32 size, u32 subdivision) 
    {
        const f32 radius = size * 0.5f;

        std::vector<Vertex> vertices { get_cube_vertices(radius, subdivision) };

        for (auto& vertex : vertices)
        {
            vertex.position = glm::normalize(vertex.position) * radius;
        }

        return vertices;
    }

    std::vector<u32> get_quad_sphere_indices(u32 subdivision) 
    {
        return get_cube_indices(subdivision);
    }
}
