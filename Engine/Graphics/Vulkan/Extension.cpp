#include "Graphics/Vulkan/Extension.hpp"

namespace Cr::Vk
{
    // TODO Error handling

    VkResult bind_instance_extension_functions(VkInstance instance)
    {
        CreateDebugUtilsMessengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
        DestroyDebugUtilsMessengerEXT = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");

        return VK_SUCCESS;
    }

    VkResult bind_device_extension_functions(VkInstance instance)
    {
        CmdBeginRenderingKHR = (PFN_vkCmdBeginRenderingKHR)vkGetInstanceProcAddr(instance, "vkCmdBeginRenderingKHR");
        CmdEndRenderingKHR = (PFN_vkCmdEndRenderingKHR)vkGetInstanceProcAddr(instance, "vkCmdEndRenderingKHR");

        return VK_SUCCESS;
    }
}
