#include "Core/Window.hpp"

namespace Cr::Core
{

Window::Window(I32 width, I32 height, const std::string& title)
{
    CR_ASSERT_THROW(glfwInit(), "GLFW failed to initialize");

    const auto primary_monitor = glfwGetPrimaryMonitor();
    const auto video_mode      = glfwGetVideoMode(primary_monitor);

    glfwWindowHint(GLFW_FLOATING,     GLFW_TRUE);
    glfwWindowHint(GLFW_RESIZABLE,    GLFW_FALSE);
    glfwWindowHint(GLFW_MAXIMIZED,    GLFW_FALSE);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_REFRESH_RATE, video_mode->refreshRate);

    m_handle = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);

    CR_ASSERT_THROW(m_handle != nullptr, "Failed to create window.");
}

Window::~Window()
{
    glfwDestroyWindow(m_handle);
    glfwTerminate();
}

} // namespace Cr::Core
