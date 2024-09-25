#pragma once

#include "Graphics/Vulkan/Vulkan.hpp"
#include "Graphics/Vulkan/Allocator.hpp"

#include "Crunch/ClassUtility.hpp"

namespace Cr::Graphics::Vulkan
{

class Texture : public NoCopy
{
    public:
        Texture() = default;

        Texture(VmaAllocator allocator, VkFormat format, VkExtent3D extent);
        ~Texture();

        Texture(Texture&& other) noexcept;
        Texture& operator = (Texture&& other) noexcept;

        [[nodiscard]] constexpr const VkImage&     get_native()  const { return m_handle;  }
        [[nodiscard]] constexpr const VkImageView& get_view()    const { return m_view;    }
        [[nodiscard]] constexpr const VkSampler&   get_sampler() const { return m_sampler; }

    private:
        VkImage     m_handle  = nullptr;
        VkImageView m_view    = nullptr;
        VkSampler   m_sampler = nullptr;

        VmaAllocation m_allocation = nullptr;
        VmaAllocator  m_allocator  = nullptr;
};

}
