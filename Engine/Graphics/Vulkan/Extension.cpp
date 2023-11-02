#include "Graphics/Vulkan/Extension.hpp"

namespace Vk
{
	VkResult bind_extension_functions(VkInstance instance)
	{
		CreateDebugUtilsMessengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
		DestroyDebugUtilsMessengerEXT = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");

		return VK_SUCCESS;
	}
}
