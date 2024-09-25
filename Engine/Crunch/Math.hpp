#pragma once

#include "Crunch.hpp"

#include <cmath>

#define GLM_FORCE_DEPTH_ONE_TO_ZERO
#define GLM_FORCE_RADIANS

#include "glm/common.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include <glm/mat3x3.hpp>
#include <glm/mat4x4.hpp>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

// Lightly wrap GLM if hit by a sudden urge to get rid of it.

namespace Cr
{
    using Vec2f = glm::vec<2, F32, glm::defaultp>;
    using Vec3f = glm::vec<3, F32, glm::defaultp>;
    using Vec4f = glm::vec<4, F32, glm::defaultp>;

    using Mat3f = glm::mat<3, 3, F32, glm::defaultp>;
    using Mat4f = glm::mat<4, 4, F32, glm::defaultp>;

    using Quatf = glm::quat;

    constexpr Vec2f VEC2_UP    { 0.0f, 1.0f };
    constexpr Vec2f VEC2_DOWN  { 0.0f,-1.0f };
    constexpr Vec2f VEC2_LEFT  {-1.0f, 0.0f }; 
    constexpr Vec2f VEC2_RIGHT { 1.0f, 0.0f };

    constexpr Vec3f VEC3F_UP       { 0.0f, 1.0f, 0.0f };
    constexpr Vec3f VEC3F_DOWN     { 0.0f,-1.0f, 0.0f };
    constexpr Vec3f VEC3F_RIGHT    { 1.0f, 0.0f, 0.0f };
    constexpr Vec3f VEC3F_LEFT     {-1.0f, 0.0f, 0.0f }; 
    constexpr Vec3f VEC3F_FORWARD  { 0.0f, 0.0f, 1.0f }; 
    constexpr Vec3f VEC3F_BACKWARD { 0.0f, 0.0f,-1.0f };

    constexpr Mat3f MAT3F_ID {1.0f};

    constexpr F32 PI = glm::pi<F32>();
}
