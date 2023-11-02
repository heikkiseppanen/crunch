#pragma once

#include <vulkan/vulkan_core.h>

namespace Vk
{
	VkResult bind_extension_functions(VkInstance instance);

	inline PFN_vkCreateDebugUtilsMessengerEXT CreateDebugUtilsMessengerEXT;
	inline PFN_vkDestroyDebugUtilsMessengerEXT DestroyDebugUtilsMessengerEXT;
}
