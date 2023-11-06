#include "Graphics/Vulkan/Buffer.hpp"
#include "Graphics/Vulkan/Debug.hpp"

#include <utility>
#include <iostream>

namespace Cr::Vk
{

Buffer::Buffer(VmaAllocator allocator, const VmaAllocationCreateInfo& allocation_info, const VkBufferCreateInfo& buffer_info)
	: m_allocator(allocator)
{
	VK_ASSERT_THROW(vmaCreateBuffer(allocator, &buffer_info, &allocation_info, &handle, &m_allocation, &m_properties), "Failed to allocate buffer")
}

Buffer::Buffer(Buffer&& other) : handle() { *this = std::move(other); }

Buffer& Buffer::operator = (Buffer&& other)
{
	if (this == &other)
		return *this;

	if (handle != VK_NULL_HANDLE)
	{
		vmaDestroyBuffer(m_allocator, handle, m_allocation);
	}

	handle       = std::exchange(other.handle, VK_NULL_HANDLE);
	m_allocator  = std::exchange(other.m_allocator, nullptr);
	m_allocation = std::exchange(other.m_allocation, nullptr);
	m_properties = std::exchange(other.m_properties, {});

	return *this;
}

void Buffer::flush()
{
	VK_ASSERT_THROW(vmaFlushAllocation(m_allocator, m_allocation, 0, VK_WHOLE_SIZE), "Failed to flush vertex allocation")
}

Buffer::~Buffer()
{
	if (handle != VK_NULL_HANDLE)
	{
		vmaDestroyBuffer(m_allocator, handle, m_allocation); 
	}
}

} // namespace Cr::Vk
