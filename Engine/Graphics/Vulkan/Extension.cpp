#include "Graphics/Vulkan/Extension.hpp"

namespace Cr::Graphics::Vulkan::Extension
{
    // TODO Error handling
    PFN_vkCreateDebugUtilsMessengerEXT  CreateDebugUtilsMessengerEXT  = nullptr;
    PFN_vkDestroyDebugUtilsMessengerEXT DestroyDebugUtilsMessengerEXT = nullptr;

    PFN_vkCmdBeginRendering CmdBeginRenderingKHR = nullptr;
    PFN_vkCmdEndRendering   CmdEndRenderingKHR = nullptr;

    VkResult bind_instance_extension_functions(VkInstance instance)
    {
        CreateDebugUtilsMessengerEXT  = (PFN_vkCreateDebugUtilsMessengerEXT)  vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
        DestroyDebugUtilsMessengerEXT = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");

        return VK_SUCCESS;
    }

    VkResult bind_device_extension_functions(VkDevice device)
    {
        CmdBeginRenderingKHR = (PFN_vkCmdBeginRenderingKHR) vkGetDeviceProcAddr(device, "vkCmdBeginRenderingKHR");
        CmdEndRenderingKHR   = (PFN_vkCmdEndRenderingKHR)   vkGetDeviceProcAddr(device, "vkCmdEndRenderingKHR");

        return VK_SUCCESS;
    }
}
