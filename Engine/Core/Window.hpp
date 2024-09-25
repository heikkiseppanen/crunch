#pragma once

#include "Crunch/Crunch.hpp"
#include "Crunch/ClassUtility.hpp"

#
#include <GLFW/glfw3.h>

#include <string>

namespace Cr::Core
{

class Window : public NoCopy
{
    public:
        Window() = delete;
        Window(I32 width, I32 height, const std::string& title);
        ~Window();

        inline void poll_events() const noexcept  { glfwPollEvents(); };

        [[nodiscard]]
        inline bool should_close() const noexcept { return glfwWindowShouldClose(m_handle); };
        inline void set_to_close() const noexcept { glfwSetWindowShouldClose(m_handle, GLFW_TRUE); };

        [[nodiscard]]
        inline float get_time() const noexcept { return glfwGetTime(); }

        inline GLFWwindow* get_native() const { return m_handle; }

    private:
        GLFWwindow* m_handle = {};
};

} // namespace Cr::Core
