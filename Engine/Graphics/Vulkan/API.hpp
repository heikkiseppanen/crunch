#pragma once

#include "Crunch.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "Graphics/Mesh.hpp"

#include "Graphics/Types.hpp"
#include "Graphics/Vulkan/Types.hpp"

#include <vector>
#include <array>

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

struct PushConstantObject
{
    alignas(16) Mat4f model;
    alignas(16) Mat4f view;
    alignas(16) Mat4f project;
};

struct MeshContext
{
    u32 vertex_buffer_id;
    u32 index_buffer_id;
};

struct TextureContext
{
    ImageID image_id;
    u16 width;
    u16 height;
};


class API
{
    public:
        API() = delete;
        API(GLFWwindow* surface_context, bool enable_logs = true);
        ~API();

        // RENDERING API

        // TODO Expose shader module creation and pipeline configuration further 
        
        [[nodiscard]]
        ShaderID shader_create(const std::vector<u8>& vertex_spirv, const std::vector<u8>& fragment_spirv);
        void     shader_destroy(ShaderID shader_id);

        // TODO Abstract vertex data layout
        [[nodiscard]]
        MeshID mesh_create(const std::vector<Vertex>& vertices, const std::vector<u32>& indices);
        void   mesh_destroy(MeshID mesh_id);

        [[nodiscard]]
        TextureID texture_create(const std::string& path);
        void      texture_destroy(TextureID texture_id);

        // Command buffer recording?
        void begin_render();
        void draw(MeshID mesh_id, ShaderID shader_id, PushConstantObject& uniforms);
        void end_render();

        // DRIVER API

        [[nodiscard]]
        BufferID buffer_create(BufferType type, u64 size);
        void     buffer_destroy(BufferID buffer_id);

        void image_destroy(ImageID image_id);

        VkShaderModule create_shader_module(const std::vector<u8>& spirv);

    private:


        // RENDERING COMPONENTS
        std::vector<MeshContext> m_mesh_list;
        std::vector<TextureContext> m_texture_list;

        std::vector<Buffer>         m_buffer_pool;
        std::vector<Image>          m_image_pool;
        std::vector<ShaderPipeline> m_shader_pool;

        // API COMPONENTS
        VkInstance m_instance;

        VkDebugUtilsMessengerEXT m_debug_messenger;
        static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
            VkDebugUtilsMessageSeverityFlagBitsEXT severity,
            VkDebugUtilsMessageTypeFlagsEXT type,
            const VkDebugUtilsMessengerCallbackDataEXT* p_callback_data,
            void* p_user_data);

        VkSurfaceKHR m_surface;

        VkPhysicalDevice           m_physical_device;
        VkPhysicalDeviceProperties m_physical_device_properties;
        VkPhysicalDeviceFeatures   m_physical_device_features;

        VkDevice     m_device;
        VmaAllocator m_allocator;

        VkCommandPool m_command_pool;
//        VkDescriptorPool m_descriptor_pool;

        VkQueue m_graphics_queue;
        VkQueue m_presentation_queue;

        // Swap chain context

        VkSwapchainKHR m_swap_chain;
        VkFormat       m_swap_format;
        VkExtent2D     m_swap_extent;

        u32 m_current_frame = 0;
        u32 m_image_index = 0;
        std::vector<VkImage>     m_swap_images;
        std::vector<VkImageView> m_swap_image_views;

        std::array<VkFence,     FRAMES_IN_FLIGHT> m_in_flight_fence;
        std::array<VkSemaphore, FRAMES_IN_FLIGHT> m_image_available_semaphore;
        std::array<VkSemaphore, FRAMES_IN_FLIGHT> m_render_finished_semaphore;

        std::array<VkCommandBuffer, FRAMES_IN_FLIGHT> m_command_buffer;

}; // class API

} // namespace Vk
