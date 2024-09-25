#include "Graphics/Vulkan/CommandBuffer.hpp"

#include "Graphics/Vulkan/Buffer.hpp"
#include "Graphics/Vulkan/Shader.hpp"
#include "Graphics/Vulkan/Texture.hpp"

#include "Graphics/Vulkan/Extensions.hpp"

namespace Cr::Graphics::Vulkan
{

CommandBuffer::CommandBuffer(VkDevice device, VkCommandPool pool) : m_handle(), m_source_pool(pool), m_device(device) {
    VkCommandBufferAllocateInfo info {
        .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool        = m_source_pool,
        .level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1,
    };
    VK_ASSERT_THROW(vkAllocateCommandBuffers(m_device, &info, &m_handle), "Failed to allocate command buffer");
}

CommandBuffer::CommandBuffer(CommandBuffer&& other) noexcept
    : m_handle(std::exchange(other.m_handle, nullptr))
    , m_source_pool(std::exchange(other.m_source_pool, nullptr))
    , m_device(std::exchange(other.m_device, nullptr))
{}

CommandBuffer& CommandBuffer::operator = (CommandBuffer&& other) noexcept
{
    if (this != &other)
    {
        std::swap(m_handle, other.m_handle);
        std::swap(m_source_pool, other.m_source_pool);
        std::swap(m_device, other.m_device);
    }
    return *this;
}

CommandBuffer::~CommandBuffer()
{
    if (m_device)
    {
        vkFreeCommandBuffers(m_device, m_source_pool, 1, &m_handle);
    }
}

void CommandBuffer::begin(VkCommandBufferUsageFlags usage)
{
    VkCommandBufferBeginInfo info {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = usage,
    };
    VK_ASSERT_THROW(vkBeginCommandBuffer(m_handle, &info), "Failed to allocate command buffer");
}

void CommandBuffer::end()
{
    VK_ASSERT_THROW(vkEndCommandBuffer(m_handle), "Failed to end command buffer");
}

void CommandBuffer::reset(VkCommandBufferResetFlags flags)
{
    VK_ASSERT_THROW(vkResetCommandBuffer(m_handle, flags), "Failed to reset command buffer");
}

void CommandBuffer::begin_rendering(VkRenderingInfo rendering_info) 
{
    Extensions::cmd_begin_rendering(m_handle, &rendering_info);
}

void CommandBuffer::end_rendering()
{
    Extensions::cmd_end_rendering(m_handle);
}

void CommandBuffer::pipeline_barrier(VkPipelineStageFlags             source,
                                     VkPipelineStageFlags             destination,
                                     VkDependencyFlags                dependencies,
                                     std::span<const VkMemoryBarrier>       memory_barriers,
                                     std::span<const VkBufferMemoryBarrier> buffer_barriers,
                                     std::span<const VkImageMemoryBarrier>  image_barriers)
{
    vkCmdPipelineBarrier(
        m_handle, source, destination, dependencies,
        memory_barriers.size(), memory_barriers.data(),
        buffer_barriers.size(), buffer_barriers.data(),
        image_barriers.size(),  image_barriers.data()
    );
}

void CommandBuffer::copy_buffer(const Vulkan::Buffer& source, Vulkan::Buffer& destination, std::span<const VkBufferCopy> regions)
{
    vkCmdCopyBuffer(m_handle, source.get_native(), destination.get_native(), regions.size(), regions.data());
}

void CommandBuffer::copy_buffer_to_texture(const Vulkan::Buffer& source, Vulkan::Texture& destination, VkImageLayout layout, std::span<const VkBufferImageCopy> regions)
{
    vkCmdCopyBufferToImage(m_handle, source.get_native(), destination.get_native(), layout, regions.size(), regions.data());
}

void CommandBuffer::bind_shader(const Vulkan::Shader& shader)
{
    vkCmdBindPipeline(m_handle, shader.get_bind_point(), shader.get_native());
}

void CommandBuffer::bind_vertex_buffer(const Vulkan::Buffer& buffer)
{
    const VkDeviceSize offset = 0;
    vkCmdBindVertexBuffers(m_handle, 0, 1, &buffer.get_native(), &offset);
}

void CommandBuffer::bind_index_buffer(const Vulkan::Buffer& buffer, VkIndexType index_type)
{
    vkCmdBindIndexBuffer(m_handle, buffer.get_native(), 0, index_type);
}

void CommandBuffer::bind_descriptor_set(const Vulkan::Shader& shader, const VkDescriptorSet& descriptor_set)
{
    vkCmdBindDescriptorSets(m_handle, shader.get_bind_point(), shader.get_pipeline_layout(), 0, 1, &descriptor_set, 0, nullptr);
}

void CommandBuffer::push_constants(const Vulkan::Shader& shader, const PushConstantObject& push_constants)
{
    vkCmdPushConstants(m_handle, shader.get_pipeline_layout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PushConstantObject), &push_constants);
}

void CommandBuffer::draw_indexed(U32 index_count, U32 instance_count, U32 first_index, I32 vertex_offset, U32 first_instance)
{
    vkCmdDrawIndexed(m_handle, index_count, instance_count, first_index, vertex_offset, first_instance);
}

void CommandBuffer::set_viewport(F32 x, F32 y, F32 width, F32 height)
{
    VkViewport viewport {
        .x = x,
        .y = y,
        .width = width,
        .height = height,
        .minDepth = 0.0f,
        .maxDepth = 1.0f,
    };
    vkCmdSetViewport(m_handle, 0, 1, &viewport);
}

void CommandBuffer::set_scissor(I32 x, I32 y, I32 width, I32 height)
{
    VkRect2D scissor {
        .offset { .x = x, .y = y },
        .extent { .width = static_cast<U32>(width), .height = static_cast<U32>(height) }
    };
    vkCmdSetScissor(m_handle, 0, 1, &scissor);
}
}
