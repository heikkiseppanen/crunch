#pragma once

#include <vulkan/vulkan_core.h>

namespace Cr::Vk
{
	// Instance extensions

	VkResult bind_instance_extension_functions(VkInstance instance);

	inline PFN_vkCreateDebugUtilsMessengerEXT CreateDebugUtilsMessengerEXT;
	inline PFN_vkDestroyDebugUtilsMessengerEXT DestroyDebugUtilsMessengerEXT;

	VkResult bind_device_extension_functions(VkInstance instance);

	inline PFN_vkCmdBeginRendering CmdBeginRenderingKHR;
	inline PFN_vkCmdEndRendering CmdEndRenderingKHR;
}
