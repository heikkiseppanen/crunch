#include "Crunch.hpp"

#include "Window/Window.hpp"

// Temp headers and values
#include "Shared/Math.hpp"
#include <glm/gtx/rotate_vector.hpp>

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

		// Temporary assets

		std::vector<Cr::Vertex> vertices = Cr::get_cube_vertices(1.0f);
		std::vector<u32> indices = Cr::get_cube_indices(); 

		auto vert_spirv = Cr::read_binary_file("Assets/Shaders/triangle.vert.spv");
		auto frag_spirv = Cr::read_binary_file("Assets/Shaders/triangle.frag.spv");

		auto mesh   = vk.create_mesh(vertices, indices);
		auto shader = vk.create_shader(vert_spirv, frag_spirv);

		// Temporary entities and components
		using PositionComponent = Cr::Vec3f;

		PositionComponent mesh_position1   {-1.0f, 0.0f, 0.0f };
		PositionComponent mesh_position2   { 1.0f, 0.0f, 0.0f };

		PositionComponent camera_position  { 0.0f, 0.8f, 2.0f };

		f32 aspect_ratio = f32(WINDOW_WIDTH) / f32(WINDOW_HEIGHT);

		Cr::Vk::PushConstantObject uniforms;

		auto model1   = glm::translate(Cr::Mat4f{1.0f}, mesh_position1);
		auto model2   = glm::translate(Cr::Mat4f{1.0f}, mesh_position2);

		uniforms.project = glm::perspective(glm::radians(90.0f), aspect_ratio, 0.1f, 100.0f);

		std::cout << mesh << ' ' << shader << '\n';

		// Init time
		float time = window.get_time();
		float time_delta = 0;

		while (!window.should_close())
		{
			window.poll_events();


			if (glfwGetKey(window.get_handle(), GLFW_KEY_ESCAPE) == GLFW_PRESS)
				window.set_to_close();

			f32 rotation_speed = 50.0f * time_delta;
			// Rotate mesh
			camera_position  = glm::rotateY(camera_position, glm::radians(-rotation_speed)); 
			uniforms.view    = glm::lookAt(camera_position, Cr::Vec3f{0.0f}, Cr::VEC3F_UP);

			vk.begin_render();

			model1 = glm::rotate(model1, glm::radians(rotation_speed), Cr::VEC3F_UP);
			uniforms.model = model1;
			vk.draw(mesh, shader, uniforms);

			model2 = glm::rotate(model2, glm::radians(rotation_speed), Cr::VEC3F_UP);
			uniforms.model = model2;
			vk.draw(mesh, shader, uniforms);

			vk.end_render();

			time_delta = window.get_time() - time;
			time += time_delta;
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
