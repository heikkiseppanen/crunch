#include "Graphics/Mesh.hpp"

namespace Cr
{
    std::vector<Vertex> get_cube_vertices(f32 dimensions)
    {
        Vec3f min { -dimensions / 2, -dimensions / 2, -dimensions / 2 }; 
        Vec3f max {  dimensions / 2,  dimensions / 2,  dimensions / 2 }; 

        Vec3f luf { min.x, max.y, max.z };
        Vec3f lub { min.x, max.y, min.z };
        Vec3f ldf { min.x, min.y, max.z };
        Vec3f ldb { min.x, min.y, min.z };

        Vec3f ruf { max.x, max.y, max.z };
        Vec3f rub { max.x, max.y, min.z };
        Vec3f rdf { max.x, min.y, max.z };
        Vec3f rdb { max.x, min.y, min.z };

        Vec2f uv00 { 0.0f, 0.0f };
        Vec2f uv10 { 1.0f, 0.0f };
        Vec2f uv01 { 0.0f, 1.0f };
        Vec2f uv11 { 1.0f, 1.0f };

        std::vector<Vertex> vertices
        {
            // Front
            {ldb, uv00},
            {lub, uv01},
            {rub, uv11},
            {rdb, uv10},

            // Back
            {rdf, uv00},
            {ruf, uv01},
            {luf, uv11},
            {ldf, uv10},

            // Left
            {ldf, uv00},
            {luf, uv01},
            {lub, uv11},
            {ldb, uv10},

            // Right
            {rdb, uv00},
            {rub, uv01},
            {ruf, uv11},
            {rdf, uv10},

            // Top
            {lub, uv00},
            {luf, uv01},
            {ruf, uv11},
            {rub, uv10},

            // Bottom
            {ldf, uv00},
            {ldb, uv01},
            {rdb, uv11},
            {rdf, uv10},
        };

        return vertices;
    }

    std::vector<u32> get_cube_indices()
    {
        constexpr u32 FACE_INDICES[] { 0, 1, 2, 0, 2, 3 };
        constexpr u32 FACE_COUNT = 6;

        std::vector<u32> indices(36);

        for (u32 face = 0; face < FACE_COUNT; ++face)
        {
            for (u32 i = 0; i < CR_ARRAY_SIZE(FACE_INDICES); ++i)
            {
                indices[i + (face * 6)] = FACE_INDICES[i] + (face * 4);
            }
        }

        return indices;
    }
}
