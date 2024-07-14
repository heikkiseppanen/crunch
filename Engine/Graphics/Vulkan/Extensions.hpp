#pragma once

#include <vulkan.h>

#include <array>

namespace Cr::Graphics::Vulkan::Extensions
{    
    constexpr std::array DEVICE_EXTENSION_LIST
    {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
    };

    VkResult bind_instance_extension_functions(VkInstance instance);
    VkResult bind_device_extension_functions(VkDevice device);

    // Instance extensions

    extern PFN_vkCreateDebugUtilsMessengerEXT  create_debug_utils_messenger;
    extern PFN_vkDestroyDebugUtilsMessengerEXT destroy_debug_utils_messenger;

    // Device extensions

    extern PFN_vkCmdBeginRendering cmd_begin_rendering;
    extern PFN_vkCmdEndRendering   cmd_end_rendering;

} // namespace Cr::Graphics::Vulkan::Extensions
