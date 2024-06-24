#include "Window/Window.hpp"

namespace Cr
{

Window::Window(i32 width, i32 height, const std::string& title)
{
    CR_ASSERT_THROW(glfwInit(), "GLFW failed to initialize");

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    m_handle = glfwCreateWindow(width, height, title.c_str(), NULL, NULL);

    CR_ASSERT_THROW(m_handle != nullptr, "Failed to create window.");

    glfwMakeContextCurrent(m_handle);
}

Window::~Window()
{
    glfwDestroyWindow(m_handle);
    glfwTerminate();
}

} // namespace Cr
