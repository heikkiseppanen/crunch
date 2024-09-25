#include "Graphics/Vulkan/Buffer.hpp"

namespace Cr::Graphics::Vulkan
{

Buffer::Buffer(VmaAllocator allocator, VkBufferUsageFlags usage, U64 size)
    : m_allocator(allocator)
{
    VkBufferCreateInfo buffer_info
    {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = size,
        .usage = usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };

    VmaAllocationCreateInfo allocation_info
    {
        .usage = VMA_MEMORY_USAGE_AUTO,
    };

    if (buffer_info.usage & (VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT))
    {
        allocation_info.flags |= VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |  VMA_ALLOCATION_CREATE_MAPPED_BIT;
    }

    VkResult result = vmaCreateBuffer(m_allocator, &buffer_info, &allocation_info, &m_handle, &m_allocation, nullptr);
    VK_ASSERT_THROW(result, "Failed to construct Vulkan buffer");
}

Buffer::Buffer(Buffer&& other) noexcept
    : m_handle    (std::exchange(other.m_handle,     nullptr))
    , m_allocation(std::exchange(other.m_allocation, nullptr))
    , m_allocator (std::exchange(other.m_allocator,  nullptr))
{}

Buffer& Buffer::operator = (Buffer&& other) noexcept
{
    if (this != &other)
    {
        std::swap(m_handle,     other.m_handle);
        std::swap(m_allocation, other.m_allocation);
        std::swap(m_allocator,  other.m_allocator);
    }
    return *this;
}

Buffer::~Buffer()
{
    if (m_handle)
    {
        vmaDestroyBuffer(m_allocator, m_handle, m_allocation);
        m_allocator  = nullptr;
        m_handle     = nullptr;
        m_allocation = nullptr;
    }
}

void Buffer::set_data(const void* begin, U64 size, U64 offset)
{
    VkResult result = vmaCopyMemoryToAllocation(m_allocator, begin, m_allocation, offset, size);
    VK_ASSERT_THROW(result, "Failure to copy data into Vulkan buffer host memory: {}", to_string(result));
}

} // namespace Cr::Graphics::Vulkan
