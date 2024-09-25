#include "Graphics/Vulkan/Queue.hpp"

namespace Cr::Graphics::Vulkan
{

Queue::Queue(VkDevice device, U32 family_index, U32 queue_index)
    : m_family_index(family_index)
    , m_device(device)
{
    vkGetDeviceQueue(device, family_index, queue_index, &m_handle);

    VkCommandPoolCreateInfo pool_info {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = family_index,
    };

    VK_ASSERT_THROW(vkCreateCommandPool(m_device, &pool_info, nullptr, &m_command_pool), "Failed to create a command pool for Vulkan queue");
}

Queue::~Queue()
{
    if (m_command_pool)
    {
        vkDestroyCommandPool(m_device, m_command_pool, nullptr);
    }
}

Queue::Queue(Queue&& other) noexcept
    : m_handle(std::exchange(other.m_handle, nullptr))
    , m_command_pool(std::exchange(other.m_command_pool, nullptr))
    , m_device(std::exchange(other.m_device, nullptr))
{}

Queue& Queue::operator = (Queue&& other) noexcept
{
    if (this != &other)
    {
        std::swap(m_handle, other.m_handle);
        std::swap(m_command_pool, other.m_command_pool);
        std::swap(m_device, other.m_device);
    }
    return *this;
}

[[nodiscard]] Unique<Vulkan::CommandBuffer> Queue::create_command_buffer()
{
    return create_unique<Vulkan::CommandBuffer>(m_device, m_command_pool);
}

void Queue::submit(std::span<const VkSubmitInfo> submissions, VkFence to_signal)
{
    VK_ASSERT_THROW(vkQueueSubmit(m_handle, submissions.size(), submissions.data(), to_signal), "Failed to submit command buffer"); 
}

void Queue::wait_for_idle()
{
    VK_ASSERT_THROW(vkQueueWaitIdle(m_handle), "Failure while waiting for queue idle.");
}

} // namespace Cr::Graphics::Vulkan
