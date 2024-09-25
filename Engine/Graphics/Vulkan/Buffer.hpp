#pragma once

#include "Graphics/Vulkan/Vulkan.hpp"
#include "Graphics/Vulkan/Allocator.hpp"

#include "Crunch/ClassUtility.hpp"

namespace Cr::Graphics::Vulkan
{

class Buffer : public NoCopy
{
    public:
        Buffer() = default;
        Buffer(VmaAllocator allocator, VkBufferUsageFlags usage, U64 size);
        ~Buffer();

        Buffer(Buffer&& other) noexcept;
        Buffer& operator = (Buffer&& other) noexcept;

        void set_data(const void* data, U64 size, U64 offset);

        [[nodiscard]] constexpr const VkBuffer& get_native() const { return m_handle; }

    private:
        VkBuffer m_handle {};

        VmaAllocation m_allocation {};
        VmaAllocator  m_allocator  {};
};

} // namespace Cr::Graphics::Vulkan
