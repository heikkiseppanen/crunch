#include "Window/Window.hpp"

#include "Graphics/Vulkan/API.hpp"

#include <iostream>

int main(int argc, char *argv[])
{
	(void)argc;
	(void)argv;

	try
	{
		Cr::Window window{1280, 720, "Crunch"};
		
		Vk::API vk{window.get_handle()};

		while (!window.should_close())
		{
		}

	}
	catch (std::exception& e)
	{
		std::cout << e.what();
	}

	return (0);
}
