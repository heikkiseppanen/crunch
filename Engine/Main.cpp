#include "Crunch.hpp"

#include "Window/Window.hpp"

// Temp headers
#include "glm/common.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include "Graphics/Vulkan/API.hpp"
#include "Shared/Filesystem.hpp"
#include <iostream>

constexpr u32 WINDOW_WIDTH = 1280;
constexpr u32 WINDOW_HEIGHT = 720;

int main(int argc, char *argv[])
{
	(void)argc;
	(void)argv;

	try
	{
		Cr::Window window{WINDOW_WIDTH, WINDOW_HEIGHT, "Crunch"};
		
		Cr::Vk::API vk{window.get_handle()};

		std::vector<Cr::Vertex> vertices 
		{ 
			{{-0.5f, -0.5f, 0.5}, {0.0f, 0.0f}},
			{{-0.5f,  0.5f, 0.5}, {0.0f, 1.0f}},
			{{ 0.5f,  0.5f, 0.5}, {1.0f, 1.0f}},
			{{ 0.5f, -0.5f, 0.5}, {1.0f, 0.0f}},
		};

		std::vector<u32> indices { 0, 1, 2, 0, 2, 3};

		// Temporary assets

		//std::vector<Cr::Vertex> vertices = Cr::get_cube_vertices(1.0f);
		//std::vector<u32> indices = Cr::get_cube_indices(); 

		auto vert_spirv = Cr::read_binary_file("Assets/Shaders/triangle.vert.spv");
		auto frag_spirv = Cr::read_binary_file("Assets/Shaders/triangle.frag.spv");

		auto mesh   = vk.create_mesh(vertices, indices);
		auto shader = vk.create_shader(vert_spirv, frag_spirv);

		// Temporary entities and components
//		using PositionComponent = Cr::Vec3f;
//
//		PositionComponent mesh_position   {0.0f, 0.0f, 0.0f};
//		PositionComponent camera_position {3.0f, 3.0f, 3.0f};
//
//		auto aspect_ratio = float(WINDOW_WIDTH) / float(WINDOW_HEIGHT);
//
//		Cr::Vk::UniformBufferObject uniforms;
//		uniforms.model   = glm::translate(Cr::Mat4f(), mesh_position);
//		uniforms.view    = glm::lookAt(camera_position, mesh_position, Cr::VEC3F_UP);
//		uniforms.project = glm::perspective(glm::radians(90.0f), aspect_ratio, 0.0f, 100.0f);

		std::cout << mesh << ' ' << shader << '\n';

		while (!window.should_close())
		{
			window.poll_events();

			if (glfwGetKey(window.get_handle(), GLFW_KEY_ESCAPE) == GLFW_PRESS)
				window.set_to_close();

			vk.draw(mesh, shader);
		}

		vk.destroy_mesh(mesh);
		vk.destroy_shader(mesh);
	}
	catch (std::exception& e)
	{
		std::cout << e.what();
	}

	return (0);
}
