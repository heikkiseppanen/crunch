#pragma once

#include "Crunch.hpp"

#include "Graphics/Mesh.hpp"
#include "Graphics/Types.hpp"

#include "Graphics/Vulkan/Types.hpp"

#include <vector>
#include <array>

namespace Cr::Core { struct Window; }

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

struct PushConstantObject // 128 byte minimum support
{
    alignas(16) Mat4f model; // 64
};

struct UniformBufferObject
{
    alignas(16) Mat4f projected_view;
};

struct MeshContext
{
    u32 vertex_buffer_id;
    u32 index_buffer_id;
    u32 index_count;
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
        API(const Core::Window& surface_context, bool debug = true);
        ~API();

        // RENDERING API

        // TODO Expose shader module creation and pipeline configuration further 
        
        [[nodiscard]]
        ShaderID shader_create(const std::vector<u8>& vertex_spirv, const std::vector<u8>& fragment_spirv);
        void     shader_set_uniform(ShaderID id, const UniformBufferObject& uniforms);
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
        void draw(MeshID mesh_id, ShaderID shader_id, const PushConstantObject& push_constants);
        void end_render();

        // DRIVER API

        [[nodiscard]]
        BufferID buffer_create(BufferType type, u64 size);

        template<typename T>
        void     buffer_map_range(BufferID id, const T* begin, u64 count, u64 offset)
        {
            const auto& buffer = m_buffer_pool[id];
            std::copy(begin, begin + count, static_cast<T*>(buffer.data) + offset);
        }

        void buffer_flush(BufferID id);
        void buffer_destroy(BufferID id);

        void image_destroy(ImageID image_id);

        VkShaderModule create_shader_module(const std::vector<u8>& spirv);

    private:

        // RENDERING COMPONENTS
        std::vector<MeshContext> m_mesh_list;
        std::vector<TextureContext> m_texture_list;

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
