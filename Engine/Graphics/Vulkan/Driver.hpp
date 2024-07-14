#pragma once

#include "Graphics/Vulkan/Vulkan.hpp"
#include "Graphics/Vulkan/Buffer.hpp"

namespace Cr::Core { class Window; }

namespace Cr::Graphics::Vulkan {

static constexpr u32 FRAMES_IN_FLIGHT = 3;

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

class Driver
{
    public:
        Driver() = delete;
        Driver(const Core::Window& surface_context, bool debug = true);
        ~Driver();

        // DRIVER API

        [[nodiscard]]
        Vulkan::Buffer create_buffer(VkBufferUsageFlags usage, u64 size);

        //void image_create(ImageID image_id);
        //void image_destroy(ImageID image_id);

        VkShaderModule create_shader_module(const std::vector<u8>& spirv);

    private:

        // API COMPONENTS
        VkInstance m_instance = VK_NULL_HANDLE;

        VkDebugUtilsMessengerEXT m_debug_messenger;
        static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
            VkDebugUtilsMessageSeverityFlagBitsEXT severity,
            VkDebugUtilsMessageTypeFlagsEXT type,
            const VkDebugUtilsMessengerCallbackDataEXT* p_callback_data,
            void* p_user_data);

        VkSurfaceKHR m_surface = VK_NULL_HANDLE;

        VkPhysicalDevice           m_physical_device = VK_NULL_HANDLE;
        VkPhysicalDeviceProperties m_physical_device_properties;
        VkPhysicalDeviceFeatures   m_physical_device_features;

        VkDevice     m_device    = VK_NULL_HANDLE;
        VmaAllocator m_allocator = VK_NULL_HANDLE;

        VkCommandPool m_command_pool       = VK_NULL_HANDLE;
        VkDescriptorPool m_descriptor_pool = VK_NULL_HANDLE;

        VkQueue m_graphics_queue     = VK_NULL_HANDLE;
        VkQueue m_presentation_queue = VK_NULL_HANDLE;

        // Swap chain context

        VkSwapchainKHR m_swap_chain  = VK_NULL_HANDLE;
        VkFormat       m_swap_format;
        VkExtent2D     m_swap_extent;

        u32 m_current_frame = 0;
        u32 m_image_index = 0;
        std::vector<VkImage>     m_swap_images;
        std::vector<VkImageView> m_swap_image_views;

        std::vector<Buffer>         m_buffer_pool;
        std::vector<Image>          m_image_pool;
        std::vector<ShaderPipeline> m_shader_pool;

        std::array<VkFence,     FRAMES_IN_FLIGHT> m_in_flight_fence {};
        std::array<VkSemaphore, FRAMES_IN_FLIGHT> m_image_available_semaphore {};
        std::array<VkSemaphore, FRAMES_IN_FLIGHT> m_render_finished_semaphore {};

        std::array<VkCommandBuffer, FRAMES_IN_FLIGHT> m_command_buffer {};

}; // class API

} // namespace Vk
