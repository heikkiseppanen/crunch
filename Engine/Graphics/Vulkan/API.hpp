#pragma once

#include "Crunch.hpp"

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

#include <vector>

namespace Vk
{

struct QueueFamilyIndices
{
	u32 graphics;
	u32 presentation;
};

class API
{
	public:
		API() = delete;
		API(GLFWwindow* surface_context, bool enable_logs = true);
		~API();

	private:

		static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
			VkDebugUtilsMessageSeverityFlagBitsEXT severity,
			VkDebugUtilsMessageTypeFlagsEXT type,
			const VkDebugUtilsMessengerCallbackDataEXT* p_callback_data,
			void* p_user_data);

		VkInstance       m_instance;
		VkSurfaceKHR     m_surface;

		VkDebugUtilsMessengerEXT m_debug_messenger;

		VkPhysicalDevice m_physical_device;
		VkPhysicalDeviceProperties m_physical_device_properties;
		VkPhysicalDeviceFeatures   m_physical_device_features;

		VkDevice         m_logical_device;

		VkQueue          m_graphics_queue;
		VkQueue          m_presentation_queue;

}; // class API

} // namespace Vk
