#include "Core/Window.hpp"

namespace Cr::Core
{

Window::Window(i32 width, i32 height, const std::string& title)
{
    CR_ASSERT_THROW(glfwInit(), "GLFW failed to initialize");

    const auto primary_monitor = glfwGetPrimaryMonitor();

    const auto video_mode = glfwGetVideoMode(primary_monitor);

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    glfwWindowHint(GLFW_MAXIMIZED, GLFW_FALSE);
    glfwWindowHint(GLFW_REFRESH_RATE, video_mode->refreshRate);

    this->context = glfwCreateWindow(width, height, title.c_str(), NULL, NULL);

    CR_ASSERT_THROW(this->context != nullptr, "Failed to create window.");

    glfwMakeContextCurrent(this->context);
}

Window::~Window()
{
    glfwDestroyWindow(this->context);
    glfwTerminate();
}

} // namespace Cr
