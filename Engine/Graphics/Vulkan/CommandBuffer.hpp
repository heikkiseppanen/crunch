#pragma once

#include "Graphics/Vulkan/Vulkan.hpp"
#include "Crunch/ClassUtility.hpp"

#include <span>

namespace Cr::Graphics::Vulkan
{

class Buffer;
class Shader;
class Texture;

class CommandBuffer : public NoCopy
{
    public:
        CommandBuffer() = default;
        CommandBuffer(VkDevice device, VkCommandPool pool);
        ~CommandBuffer();

        CommandBuffer(CommandBuffer&& other) noexcept;
        CommandBuffer& operator = (CommandBuffer&& other) noexcept;

        [[nodiscard]] constexpr VkCommandBuffer& get_native() { return m_handle; }

        void begin(VkCommandBufferUsageFlags usage);
        void reset(VkCommandBufferResetFlags flags);
        void end();

        void begin_rendering(VkRenderingInfo rendering_info);
        void end_rendering();

        void pipeline_barrier(VkPipelineStageFlags                   source,
                              VkPipelineStageFlags                   destination,
                              VkDependencyFlags                      dependencies,
                              std::span<const VkMemoryBarrier>       memory_barriers,
                              std::span<const VkBufferMemoryBarrier> buffer_barriers,
                              std::span<const VkImageMemoryBarrier>  image_barriers );

        void copy_buffer(const Vulkan::Buffer& source, Vulkan::Buffer& destination, std::span<const VkBufferCopy> regions);
        void copy_buffer_to_texture(const Vulkan::Buffer& source, Vulkan::Texture& destination, VkImageLayout layout, std::span<const VkBufferImageCopy> regions);

        void bind_shader(const Vulkan::Shader& shader);
        void bind_vertex_buffer(const Vulkan::Buffer& buffer);
        void bind_index_buffer(const Vulkan::Buffer& buffer, VkIndexType index_type);
        void bind_descriptor_set(const Vulkan::Shader& shader, const VkDescriptorSet& descriptor_set);

        void push_constants(const Vulkan::Shader& shader, const PushConstantObject& push_constants);

        void draw_indexed(U32 index_count, U32 instance_count, U32 first_index, I32 vertex_offset, U32 first_instance);

        void set_viewport(F32 x, F32 y, F32 width, F32 height); 
        void set_scissor(I32 x, I32 y, I32 width, I32 height);

    private:
        VkCommandBuffer m_handle {};
        VkCommandPool   m_source_pool {};

        VkDevice        m_device {};
};

} // namespace Cr::Graphics::Vulkan
