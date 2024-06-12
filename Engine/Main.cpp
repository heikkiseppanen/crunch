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

        std::vector<Cr::Vertex> vertices = Cr::get_cube_vertices(1.0f);
        std::vector<u32>        indices  = Cr::get_cube_indices(); 

        auto vertex_shader_binary   = Cr::read_binary_file("Assets/Shaders/triangle.vert.spv");
        auto fragment_shader_binary = Cr::read_binary_file("Assets/Shaders/triangle.frag.spv");

        auto texture = vk.texture_create("Assets/Textures/T_CrunchLogo_D.ktx2"); // Need to make texture before shader for now...
        (void)texture;

        auto mesh   = vk.mesh_create(vertices, indices);
        auto shader = vk.shader_create(vertex_shader_binary, fragment_shader_binary);

        Cr::Vec3f mesh_position_1 {-1.0f, 0.0f, 0.0f };
        Cr::Vec3f mesh_position_2 { 1.0f, 0.0f, 0.0f };
        // Temporary entities and components

        Cr::Vec3f camera_position { 2.0f, 2.0f, -2.0f };

        auto model_matrix_1 = glm::translate(Cr::Mat4f{1.0f}, mesh_position_1);
        auto model_matrix_2 = glm::translate(Cr::Mat4f{1.0f}, mesh_position_2);

        f32 aspect_ratio = f32(WINDOW_WIDTH) / f32(WINDOW_HEIGHT);

        Cr::Graphics::Vulkan::UniformBufferObject uniforms;

        uniforms.project = glm::perspective(glm::radians(70.0f), aspect_ratio, 0.1f, 100.0f);
        uniforms.view    = glm::lookAt(camera_position, {0.0f, 0.0f, 0.0f}, Cr::VEC3F_UP);

        // Main Loop

        float time = window.get_time();

        while (!window.should_close())
        {
            float time_delta = window.get_time() - time;
            time += time_delta;

            window.poll_events();

            if (glfwGetKey(window.get_handle(), GLFW_KEY_ESCAPE) == GLFW_PRESS)
                window.set_to_close();

            vk.begin_render();

            f32 rotation_velocity = 50.0f * time_delta;

            model_matrix_1 = glm::rotate(model_matrix_1, glm::radians(rotation_velocity), Cr::VEC3F_UP);
            uniforms.model = model_matrix_1;
            vk.draw(mesh, shader, uniforms);

            model_matrix_2 = glm::rotate(model_matrix_2, glm::radians(-rotation_velocity), Cr::VEC3F_UP);
            uniforms.model = model_matrix_2;
            vk.draw(mesh, shader, uniforms);

            vk.end_render();
        }
    }
    catch (std::exception& e)
    {
        std::cout << e.what();
    }

    return (0);
}
