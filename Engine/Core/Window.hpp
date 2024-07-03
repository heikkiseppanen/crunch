#pragma once

#include "Crunch.hpp"
#include "Shared/ClassUtility.hpp"

#include <GLFW/glfw3.h>

#include <string>

namespace Cr::Core
{

struct Window : public NoValueSemantics
{
    using Context = GLFWwindow*;

    Context context = {};

    Window() = delete;
    Window(i32 width, i32 height, const std::string& title);
    ~Window();

    inline void poll_events() const noexcept  { glfwPollEvents(); };

    [[nodiscard]]
    inline bool should_close() const noexcept { return glfwWindowShouldClose(context); };
    inline void set_to_close() const noexcept { glfwSetWindowShouldClose(context, GLFW_TRUE); };

    [[nodiscard]]
    inline float get_time() const noexcept { return glfwGetTime(); }

}; // class Window

} // namespace Cr
