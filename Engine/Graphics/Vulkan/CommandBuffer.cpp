#include "Graphics/Vulkan/CommandBuffer.hpp"

#include "Graphics/Vulkan/Debug.hpp"

namespace Cr::Vk
{

CommandBuffer::CommandBuffer(VkDevice device, VkCommandPool pool) 
{
    VkCommandBufferAllocateInfo command_buffer_info {};
    command_buffer_info.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    command_buffer_info.pNext              = nullptr;
    command_buffer_info.commandPool        = pool;
    command_buffer_info.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY; // Submit to queue, cannot be called from other buffers
    command_buffer_info.commandBufferCount = 1;

    VK_ASSERT_THROW(vkAllocateCommandBuffers(device, &command_buffer_info, &m_handle), "Failed to allocate command buffer");
}

void CommandBuffer::begin_recording(VkCommandBufferUsageFlags flags)
{
    VkCommandBufferBeginInfo begin_info {};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.pNext = nullptr;
    begin_info.flags = flags;
    begin_info.pInheritanceInfo = nullptr;

    VK_ASSERT_THROW(vkBeginCommandBuffer(m_handle, &begin_info), "Failed to begin recording command buffer");
}

void CommandBuffer::end_recording()
{
    VK_ASSERT_THROW(vkEndCommandBuffer(m_handle), "Failed to end recording command buffer")
}

void CommandBuffer::set_image_memory_barrier(Image& image, VkImageLayout old_layout, VkImageLayout new_layout, VkImageSubresourceRange sub_resource_layout)
{
    VkImageMemoryBarrier image_memory_barrier
    {
        .sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .pNext               = VK_NULL_HANDLE,
        .srcAccessMask       = VK_ACCESS_NONE, // ??
        .dstAccessMask       = VK_ACCESS_NONE, // ??
        .oldLayout           = old_layout,
        .newLayout           = new_layout,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image               = image.handle,
        .subresourceRange    = sub_resource_layout,
    };

    vkCmdPipelineBarrier(
        m_handle,
        VK_PIPELINE_STAGE_NONE, // Stages to WAIT in previous commands before...
        VK_PIPELINE_STAGE_NONE, // ...these are allowed to execute in following commands.
        0, 0, nullptr, 0, nullptr, 1, &image_memory_barrier);
    
}

}
