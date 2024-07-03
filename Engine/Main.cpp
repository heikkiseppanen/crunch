#include "Crunch.hpp"

#include "Core/Window.hpp"
#include "Core/Input.hpp"

// Temp headers and values
#include "Shared/Math.hpp"
#include <glm/gtx/rotate_vector.hpp>

#include "Graphics/Vulkan/API.hpp"
#include "Shared/Filesystem.hpp"

#include <iostream>

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    try
    {
        constexpr u32 WINDOW_WIDTH = 1280;
        constexpr u32 WINDOW_HEIGHT = 720;
        constexpr f32 FOV = 90.0f;
        constexpr f32 ASPECT_RATIO = f32(WINDOW_WIDTH) / f32(WINDOW_HEIGHT);

        constexpr f32 MOUSE_SENSITIVITY = 0.5f;

        Cr::Core::Window window { WINDOW_WIDTH, WINDOW_HEIGHT, "Crunch" };
        Cr::Core::Input  input  { window.context };

        Cr::Graphics::Vulkan::API vk{ window };

        // Temporary assets

        const auto texture_id = vk.texture_create("Assets/Textures/T_CrunchLogo_D.ktx2"); // TODO: manual texture binding and other shenanigans going on with this and the shader
        (void)texture_id;

        const Cr::Graphics::ShaderID shader_id = vk.shader_create(
            Cr::read_binary_file("Assets/Shaders/triangle.vert.spv"),
            Cr::read_binary_file("Assets/Shaders/triangle.frag.spv")
        );

        auto cube_mesh   = vk.mesh_create
        (
            Cr::get_cube_vertices(1.0f, 0),
            Cr::get_cube_indices(0) 
        );

        auto sphere_mesh = vk.mesh_create
        (
            Cr::get_quad_sphere_vertices(1.0f, 3),
            Cr::get_quad_sphere_indices(3) 
        );

        Cr::Vec3f cube_position   {};
        //Cr::Vec3f cube_position   { 1.0f, 1.0f, 1.0f };
        Cr::Vec3f sphere_position {-1.0f,-1.0f,-1.0f };

        auto cube_matrix   = glm::translate(Cr::Mat4f{1.0f}, cube_position);
        auto sphere_matrix = glm::translate(Cr::Mat4f{1.0f}, sphere_position);

        // Camera
        Cr::Vec3f camera_position  { 0.0f, 0.0f, 2.0f };
        Cr::Vec3f camera_xyz_euler { 0.0f, 0.0f,  0.0f };

        Cr::Vec3f camera_forward = Cr::Quatf{camera_xyz_euler} * Cr::VEC3F_FORWARD;
        Cr::Vec3f camera_right   = glm::normalize(glm::cross(-camera_forward, Cr::VEC3F_UP));
        Cr::Vec3f camera_up      = glm::normalize(glm::cross(-camera_forward, camera_right));

        //std::cout << v.x << ' ' << v.y << ' ' << v.z << '\n';

        Cr::Vec2f mouse_last_position = input.getMousePosition();

        // Main Loop

        f32 time = window.get_time();

        while (!window.should_close())
        {
            f32 time_delta = window.get_time() - time;
            time += time_delta;

            window.poll_events();

            if (input.isKeyDown(Cr::Key::ESCAPE))
            {
                window.set_to_close();
            }

            const Cr::Vec3f axis
            {
                f32(-input.isKeyDown(Cr::Key::A)            + input.isKeyDown(Cr::Key::D)),
                f32(-input.isKeyDown(Cr::Key::LEFT_CONTROL) + input.isKeyDown(Cr::Key::SPACE)),
                f32(-input.isKeyDown(Cr::Key::S)            + input.isKeyDown(Cr::Key::W)),
            };

            const Cr::Vec3f camera_axis_offset = axis * time_delta;

            camera_position += camera_axis_offset.x * camera_right;
            camera_position += camera_axis_offset.y * camera_up;
            camera_position += camera_axis_offset.z * -camera_forward;

            const auto mouse_new_position = input.getMousePosition();
            const auto mouse_offset = mouse_new_position - mouse_last_position;
            mouse_last_position = mouse_new_position;

            camera_xyz_euler.x -= MOUSE_SENSITIVITY * mouse_offset.y * time_delta;
            camera_xyz_euler.y -= MOUSE_SENSITIVITY * mouse_offset.x * time_delta;

            camera_xyz_euler.x = glm::clamp(camera_xyz_euler.x, -Cr::PI / 2 + 0.0001f, Cr::PI / 2 - 0.0001f);

            camera_forward = Cr::Quatf{camera_xyz_euler} * Cr::VEC3F_FORWARD;
            camera_right   = glm::normalize(glm::cross(-camera_forward, Cr::VEC3F_UP));
            camera_up      = glm::normalize(glm::cross(camera_forward, camera_right));

            std::cout << camera_forward.x << ' ' << camera_forward.y << ' ' << camera_forward.z << '\n';
            std::cout << camera_right.x << ' ' << camera_right.y << ' ' << camera_right.z << '\n';
            std::cout << camera_up.x << ' ' << camera_up.y << ' ' << camera_up.z << '\n' << '\n';

            const Cr::Mat4f perspective_matrix = glm::perspective(glm::radians(FOV), ASPECT_RATIO, 0.1f, 100.0f);
            const Cr::Mat4f view_matrix = glm::lookAt(camera_position, camera_position - camera_forward, camera_up);
            
            Cr::Graphics::Vulkan::UniformBufferObject frame_data
            {
                .projected_view = perspective_matrix * view_matrix
            };

            vk.shader_set_uniform(shader_id, frame_data);

            vk.begin_render();

            const f32 rotation_velocity = 50.0f * time_delta;

            //cube_matrix   = glm::rotate(cube_matrix,   glm::radians( rotation_velocity), Cr::VEC3F_UP);
            sphere_matrix = glm::rotate(sphere_matrix, glm::radians(-rotation_velocity), Cr::VEC3F_UP);

            Cr::Graphics::Vulkan::PushConstantObject instance_data;

            instance_data.model = cube_matrix;
            vk.draw(cube_mesh, shader_id, instance_data);

            instance_data.model = sphere_matrix;
            vk.draw(sphere_mesh, shader_id, instance_data);

            vk.end_render();
        }
    }
    catch (std::exception& e)
    {
        std::cout << e.what();
    }

    return (0);
}
