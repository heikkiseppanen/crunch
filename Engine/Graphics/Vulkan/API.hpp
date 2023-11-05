#pragma once

#include "Crunch.hpp"

#include "Graphics/Vulkan/Allocator.hpp"
#include "Graphics/Mesh.hpp"

// Shouldn't need GLFW in this header preferably...
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>

namespace Vk
{

struct SwapChainSupportDetails
{
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> present_modes;
};

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

		void proto_render_loop();

	private:

		static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
			VkDebugUtilsMessageSeverityFlagBitsEXT severity,
			VkDebugUtilsMessageTypeFlagsEXT type,
			const VkDebugUtilsMessengerCallbackDataEXT* p_callback_data,
			void* p_user_data);

		VkShaderModule create_module(const std::string& path);

		VkInstance m_instance;

		VmaAllocator m_allocator;

		VkSurfaceKHR m_surface;

		VkDebugUtilsMessengerEXT m_debug_messenger;

		VkPhysicalDevice m_physical_device;

		VkPhysicalDeviceProperties m_physical_device_properties;
		VkPhysicalDeviceFeatures   m_physical_device_features;

		VkDevice m_logical_device;

		VkQueue m_graphics_queue;
		VkQueue m_presentation_queue;

		VkSwapchainKHR m_swap_chain;

		std::vector<VkImage> m_swap_images;
		std::vector<VkImageView> m_swap_image_views;

		VkFormat m_swap_format;
		VkExtent2D m_swap_extent;

		VkPipelineLayout m_pipeline_layout;
		VkPipeline m_graphics_pipeline;

		u32 m_frames_in_flight = 2;
		u32 m_current_frame = 0;

		VkCommandPool m_command_pool;

		std::vector<VkCommandBuffer> m_command_buffer;

		std::vector<VkSemaphore> m_image_available_semaphore;
		std::vector<VkSemaphore> m_render_finished_semaphore;

		std::vector<VkFence> m_in_flight_fence;

		VkBuffer m_vertex_buffer;
		VkBuffer m_index_buffer;
		VmaAllocation m_vertex_allocation;
		VmaAllocation m_index_allocation;

}; // class API

} // namespace Vk
