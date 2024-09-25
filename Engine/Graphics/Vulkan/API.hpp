#pragma once

#include "Graphics/Vulkan/Vulkan.hpp"

#include "Graphics/Vulkan/Buffer.hpp"
#include "Graphics/Vulkan/Texture.hpp"
#include "Graphics/Vulkan/Queue.hpp"
#include "Graphics/Vulkan/CommandBuffer.hpp"
#include "Graphics/Vulkan/ShaderModule.hpp"
#include "Graphics/Vulkan/Shader.hpp"

namespace Cr::Core { class Window; }

namespace Cr::Graphics::Vulkan {

static constexpr U32 FRAMES_IN_FLIGHT = 3;


class API
{
    public:
        API() = delete;
        API(const Core::Window& surface_context, bool debug = true);
        ~API();

        [[nodiscard]] Unique<Vulkan::Buffer>       create_buffer(VkBufferCreateFlags usage, U64 size);
        [[nodiscard]] Unique<Vulkan::Texture>      create_texture(VkFormat format, VkExtent3D extent);

        [[nodiscard]] Unique<Vulkan::ShaderModule> create_shader_module(std::span<const U8> spirv, VkShaderStageFlags stage);
        [[nodiscard]] Unique<Vulkan::Shader>       create_shader(VkPipelineBindPoint usage, std::span<const Vulkan::ShaderModule* const> modules);

        [[nodiscard]] Vulkan::Queue& get_command_queue(VkQueueFlags family);

        [[nodiscard]] Vulkan::CommandBuffer& begin_frame();
                                        void end_frame();

    // TEMP
    //private:
        static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
            VkDebugUtilsMessageSeverityFlagBitsEXT severity,
            VkDebugUtilsMessageTypeFlagsEXT type,
            const VkDebugUtilsMessengerCallbackDataEXT* p_callback_data,
            void* p_user_data);

        // API COMPONENTS
        VkInstance m_instance = VK_NULL_HANDLE;

        VkDebugUtilsMessengerEXT m_debug_messenger = VK_NULL_HANDLE;

        VkSurfaceKHR m_surface = VK_NULL_HANDLE;

        VkPhysicalDevice           m_physical_device = VK_NULL_HANDLE;
        VkPhysicalDeviceProperties m_physical_device_properties;
        VkPhysicalDeviceFeatures   m_physical_device_features;

        VkDevice     m_device    = VK_NULL_HANDLE;
        VmaAllocator m_allocator = VK_NULL_HANDLE;

        VkDescriptorPool m_descriptor_pool = VK_NULL_HANDLE;

        Vulkan::Queue m_queue {};

        // SWAP CHAIN

        VkSwapchainKHR m_swap_chain  = VK_NULL_HANDLE;
        VkFormat       m_swap_format;
        VkExtent2D     m_swap_extent;

        U32 m_frame_index = 0;
        U32 m_image_index = 0;

        std::vector<VkImage>     m_swap_images;
        std::vector<VkImageView> m_swap_image_views;
        std::array<Unique<Vulkan::CommandBuffer>, FRAMES_IN_FLIGHT> m_command_buffer {};

        std::array<VkFence,     FRAMES_IN_FLIGHT> m_in_flight_fence {};
        std::array<VkSemaphore, FRAMES_IN_FLIGHT> m_image_available_semaphore {};
        std::array<VkSemaphore, FRAMES_IN_FLIGHT> m_render_finished_semaphore {};


}; // class API

} // namespace Vk
