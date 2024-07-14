#pragma once

#include "Graphics/Vulkan/Vulkan.hpp"
#include "Graphics/Vulkan/Allocator.hpp"

#include "Shared/ClassUtility.hpp"

namespace Cr::Graphics::Vulkan
{

class Buffer : public NoCopy
{
    public:
        constexpr Buffer() : handle(), allocation(), allocator() {}

        Buffer(VmaAllocator allocator, const VkBufferCreateInfo& buffer_info, const VmaAllocationCreateInfo& allocation_info);
        ~Buffer();

        Buffer(Buffer&& other);
        Buffer& operator = (Buffer&& other);

        void map(const void* begin, u64 size, u64 offset);
        void flush(u64 begin = 0, u64 size = VK_WHOLE_SIZE);

    private:
        void destroy();

        VkBuffer handle;

        VmaAllocation allocation;
        VmaAllocator  allocator;
};

} // namespace Cr::Graphics::Vulkan
