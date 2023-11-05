#pragma once

#include "Crunch.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>

namespace Cr
{

class Window
{

private:

	GLFWwindow *m_handle;

public:

	Window() = delete;
	Window(i32 width, i32 height, const std::string& title);
	~Window();

	inline void poll_events() const noexcept  { glfwPollEvents(); };

	[[nodiscard]]
	inline bool should_close() const noexcept { return glfwWindowShouldClose(m_handle); };
	inline void set_to_close() const noexcept { glfwSetWindowShouldClose(m_handle, GLFW_TRUE); };

	constexpr inline GLFWwindow* get_handle() const noexcept { return m_handle; };

}; // class Window

} // namespace Cr
