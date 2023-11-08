#pragma once

#include "Crunch.hpp"

#include "Graphics/Vulkan/Allocator.hpp"
#include "Graphics/Mesh.hpp"

// Shouldn't need GLFW in this header preferably...
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>

namespace Cr::Vk
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

struct UniformBufferObject
{
	Mat4f model;
	Mat4f view;
	Mat4f project;
};

struct MeshContext
{
	u32 vertex_buffer;
	u32 index_buffer;
};

struct ShaderContext
{
	VkPipeline pipeline;
	VkPipelineLayout pipeline_layout;
//	VkDescriptorSetLayout descriptor_set_layout;
};

struct Buffer
{
	VkBuffer handle;
	VmaAllocation allocation;
	void* data; 
};

class API
{
	public:
		API() = delete;
		API(GLFWwindow* surface_context, bool enable_logs = true);
		~API();

		// RENDERING API

		// TODO Abstract vertex data layout
		u32  create_mesh(const std::vector<Vertex>& vertices, const std::vector<u32>& indices);
		void destroy_mesh(u32 mesh);

		// TODO Expose shader module creation and pipeline configuration further 
		u32  create_shader(const std::vector<u8>& vertex_spirv, const std::vector<u8>& fragment_spirv);
		void destroy_shader(u32 shader);

		void draw(u32 mesh, u32 shader);

		// DRIVER API

		Buffer create_buffer(VkBufferCreateInfo& buffer_info);

	private:

		VkShaderModule create_shader_module(const std::vector<u8>& spirv);

		VkInstance m_instance;

		// Debug context
		VkDebugUtilsMessengerEXT m_debug_messenger;
		static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
			VkDebugUtilsMessageSeverityFlagBitsEXT severity,
			VkDebugUtilsMessageTypeFlagsEXT type,
			const VkDebugUtilsMessengerCallbackDataEXT* p_callback_data,
			void* p_user_data);

		VkSurfaceKHR m_surface;

		VkPhysicalDevice m_physical_device;
		VkPhysicalDeviceProperties m_physical_device_properties;
		VkPhysicalDeviceFeatures m_physical_device_features;

		VkDevice m_device;

		VmaAllocator m_allocator;

		VkCommandPool m_command_pool;

		VkQueue m_graphics_queue;
		VkQueue m_presentation_queue;

		// Swap chain context

		static constexpr u32 FRAMES_IN_FLIGHT = 4;

		VkSwapchainKHR m_swap_chain;
		VkFormat       m_swap_format;
		VkExtent2D     m_swap_extent;

		u32 m_current_frame = 0;
		std::vector<VkImage>     m_swap_images;
		std::vector<VkImageView> m_swap_image_views;

		std::vector<VkFence>     m_in_flight_fence;
		std::vector<VkSemaphore> m_image_available_semaphore;
		std::vector<VkSemaphore> m_render_finished_semaphore;

		std::vector<VkCommandBuffer> m_command_buffer;

		std::vector<Buffer>        m_buffer_pool;

		std::vector<ShaderContext> m_shader_pool;

		std::vector<MeshContext>   m_mesh_contexts;
}; // class API

} // namespace Vk
