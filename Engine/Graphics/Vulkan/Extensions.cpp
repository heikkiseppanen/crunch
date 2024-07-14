#include "Graphics/Vulkan/Extensions.hpp"

namespace Cr::Graphics::Vulkan::Extensions
{
    // TODO Error handling
    PFN_vkCreateDebugUtilsMessengerEXT  create_debug_utils_messenger  = nullptr;
    PFN_vkDestroyDebugUtilsMessengerEXT destroy_debug_utils_messenger = nullptr;

    PFN_vkCmdBeginRendering cmd_begin_rendering = nullptr;
    PFN_vkCmdEndRendering   cmd_end_rendering = nullptr;

    VkResult bind_instance_extension_functions(VkInstance instance)
    {
        create_debug_utils_messenger  = (PFN_vkCreateDebugUtilsMessengerEXT)  vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
        destroy_debug_utils_messenger = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");

        return VK_SUCCESS;
    }

    VkResult bind_device_extension_functions(VkDevice device)
    {
        cmd_begin_rendering = (PFN_vkCmdBeginRenderingKHR) vkGetDeviceProcAddr(device, "vkCmdBeginRenderingKHR");
        cmd_end_rendering   = (PFN_vkCmdEndRenderingKHR)   vkGetDeviceProcAddr(device, "vkCmdEndRenderingKHR");

        return VK_SUCCESS;
    }
}
