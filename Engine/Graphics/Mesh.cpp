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
            {ldb, uv01},
            {lub, uv00},
            {rub, uv10},
            {rdb, uv11},

            // Back
            {rdf, uv01},
            {ruf, uv00},
            {luf, uv10},
            {ldf, uv11},

            // Left
            {ldf, uv01},
            {luf, uv00},
            {lub, uv10},
            {ldb, uv11},

            // Right
            {rdb, uv01},
            {rub, uv00},
            {ruf, uv10},
            {rdf, uv11},

            // Top
            {lub, uv01},
            {luf, uv00},
            {ruf, uv10},
            {rub, uv11},

            // Bottom
            {ldf, uv01},
            {ldb, uv00},
            {rdb, uv10},
            {rdf, uv11},
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
