#include "Graphics/Vulkan/Buffer.hpp"

namespace Cr::Graphics::Vulkan
{

Buffer::Buffer(VmaAllocator allocator, const VkBufferCreateInfo& buffer_info, const VmaAllocationCreateInfo& allocation_info)
    : allocator(allocator)
{
    VkResult result = vmaCreateBuffer(this->allocator, &buffer_info, &allocation_info, &this->handle, &this->allocation, nullptr);
    VK_ASSERT_THROW(result, "Failed to construct Vulkan buffer");
}

Buffer::Buffer(Buffer&& other)
    : handle(std::exchange(other.handle, nullptr))
    , allocation(std::exchange(other.allocation, nullptr))
    , allocator(std::exchange(other.allocator, nullptr))
{}

Buffer& Buffer::operator = (Buffer&& other)
{
    if (this == &other) { return *this; }

    this->destroy();

    this->handle     = std::exchange(other.handle, nullptr);
    this->allocation = std::exchange(other.allocation, nullptr);
    this->allocator  = std::exchange(other.allocator, nullptr);

    return *this;
}

Buffer::~Buffer()
{
    this->destroy();
}

void Buffer::map(const void* begin, u64 size, u64 offset)
{
    VkResult result = vmaCopyMemoryToAllocation(this->allocator, begin, this->allocation, offset, size);
    VK_ASSERT_THROW(result, "Failed to copy into Vulkan buffer CPU memory");
}

inline void Buffer::flush(u64 begin, u64 size)
{
    VkResult result = vmaFlushAllocation(this->allocator, this->allocation, begin, size);
    VK_ASSERT_THROW(result, "Failed to flush Vulkan buffer");
} 

void Buffer::destroy()
{
    if (allocator)
    {
        vmaDestroyBuffer(this->allocator, this->handle, this->allocation);
    }
}

} // namespace Cr::Graphics::Vulkan
