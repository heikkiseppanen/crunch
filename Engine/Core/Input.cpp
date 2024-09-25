#include "Core/Window.hpp"
#include "Core/Input.hpp"

#include <GLFW/glfw3.h>

namespace Cr::Core
{

Input::Input(GLFWwindow* context) : m_context(context)
{
    if (glfwRawMouseMotionSupported())
    {
        glfwSetInputMode(m_context, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
    }

    glfwSetInputMode(m_context, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

} // namespace Cr::System
