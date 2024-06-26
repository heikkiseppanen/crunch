#include "Crunch.hpp"

#include "Window/Window.hpp"

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

        Cr::Window window{WINDOW_WIDTH, WINDOW_HEIGHT, "Crunch"};
        
        Cr::Graphics::Vulkan::API vk{ window.get_handle() };

        // Temporary assets

        auto vertex_shader_binary   = Cr::read_binary_file("Assets/Shaders/triangle.vert.spv");
        auto fragment_shader_binary = Cr::read_binary_file("Assets/Shaders/triangle.frag.spv");

        auto texture = vk.texture_create("Assets/Textures/T_CrunchLogo_D.ktx2"); // Need to make texture before shader for now...
        (void)texture;

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

        auto shader = vk.shader_create(vertex_shader_binary, fragment_shader_binary);

        Cr::Vec3f mesh_position_1 {-1.0f, 0.0f, 0.0f };
        Cr::Vec3f mesh_position_2 { 1.0f, 0.0f, 0.0f };
        // Temporary entities and components

        Cr::Vec3f camera_position { 2.0f, 2.0f, -2.0f };

        auto model_matrix_1 = glm::translate(Cr::Mat4f{1.0f}, mesh_position_1);
        auto model_matrix_2 = glm::translate(Cr::Mat4f{1.0f}, mesh_position_2);

        f32 aspect_ratio = f32(WINDOW_WIDTH) / f32(WINDOW_HEIGHT);

        Cr::Graphics::Vulkan::UniformBufferObject frame_data
        {
            .projected_view = glm::perspective(glm::radians(70.0f), aspect_ratio, 0.1f, 100.0f)
                            * glm::lookAt(camera_position, {0.0f, 0.0f, 0.0f}, Cr::VEC3F_UP)
        };

        // Main Loop

        float time = window.get_time();

        while (!window.should_close())
        {
            float time_delta = window.get_time() - time;
            time += time_delta;

            window.poll_events();

            if (glfwGetKey(window.get_handle(), GLFW_KEY_ESCAPE) == GLFW_PRESS)
            {
                window.set_to_close();
            }

            vk.shader_set_uniform(shader, frame_data);

            vk.begin_render();

            f32 rotation_velocity = 50.0f * time_delta;

            Cr::Graphics::Vulkan::PushConstantObject instance_data;

            model_matrix_1 = glm::rotate(model_matrix_1, glm::radians(rotation_velocity), Cr::VEC3F_UP);
            instance_data.model = model_matrix_1;
            vk.draw(cube_mesh, shader, instance_data);

            model_matrix_2 = glm::rotate(model_matrix_2, glm::radians(-rotation_velocity), Cr::VEC3F_UP);
            instance_data.model = model_matrix_2;
            vk.draw(sphere_mesh, shader, instance_data);

            vk.end_render();
        }
    }
    catch (std::exception& e)
    {
        std::cout << e.what();
    }

    return (0);
}
