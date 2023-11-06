#include "Graphics/Vulkan/API.hpp"
#include "Window/Window.hpp"

#include <iostream>

int main(int argc, char *argv[])
{
	(void)argc;
	(void)argv;

	try
	{
		Cr::Window window{1280, 720, "Crunch"};
		
		Cr::Vk::API vk{window.get_handle()};

		while (!window.should_close())
		{
			window.poll_events();

			if (glfwGetKey(window.get_handle(), GLFW_KEY_ESCAPE) == GLFW_PRESS)
				window.set_to_close();

			vk.proto_render_loop();
		}
	}
	catch (std::exception& e)
	{
		std::cout << e.what();
	}

	return (0);
}
