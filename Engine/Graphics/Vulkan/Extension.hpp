#pragma once

#include <array>

#include <vulkan/vulkan_core.h>

namespace Cr::Vk
{    
    // Instance extensions

    VkResult bind_instance_extension_functions(VkInstance instance);

    extern PFN_vkCreateDebugUtilsMessengerEXT  CreateDebugUtilsMessengerEXT;
    extern PFN_vkDestroyDebugUtilsMessengerEXT DestroyDebugUtilsMessengerEXT;

    VkResult bind_device_extension_functions(VkDevice device);

    extern PFN_vkCmdBeginRendering CmdBeginRenderingKHR;
    extern PFN_vkCmdEndRendering   CmdEndRenderingKHR;

}
