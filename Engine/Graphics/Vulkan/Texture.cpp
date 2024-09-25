#include "Graphics/Vulkan/Texture.hpp"

namespace Cr::Graphics::Vulkan
{

Texture::Texture(VmaAllocator allocator, VkFormat format, VkExtent3D extent) 
    : m_allocator(allocator)
{
    VmaAllocatorInfo allocator_info;
    vmaGetAllocatorInfo(m_allocator, &allocator_info);

    const VkDevice device = allocator_info.device;

    VkImageCreateInfo image_info {
        .sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .pNext         = nullptr,
        .flags         = 0,
        .imageType     = VK_IMAGE_TYPE_2D,
        .format        = format,
        .extent        = extent,
        .mipLevels     = 1,
        .arrayLayers   = 1,
        .samples       = VK_SAMPLE_COUNT_1_BIT,
        .tiling        = VK_IMAGE_TILING_OPTIMAL,
        .usage         = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        .sharingMode   = VK_SHARING_MODE_EXCLUSIVE,
        // .queueFamilyIndexCount,
        // .pQueueFamilyIndices,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    };

    VmaAllocationCreateInfo allocation_info {};
    allocation_info.usage = VMA_MEMORY_USAGE_AUTO;

    VK_ASSERT_THROW(vmaCreateImage(m_allocator, &image_info, &allocation_info, &m_handle, &m_allocation, nullptr), "Failed to create texture image");

    VkImageViewCreateInfo view_info {
        .sType              = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .pNext              = nullptr,
        .flags              = 0,
        .image              = m_handle,
        .viewType           = VK_IMAGE_VIEW_TYPE_2D,
        .format             = format,
        .components         = {}, // Default rgb
        .subresourceRange   = {
            .aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel   = 0,
            .levelCount     = 1,
            .baseArrayLayer = 0,
            .layerCount     = 1,
        },
    };

    VK_ASSERT_THROW(vkCreateImageView(device, &view_info, nullptr, &m_view), "Failed to create image view for texture");

    VkSamplerCreateInfo sampler_info {
        .sType                   = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .pNext                   = nullptr,
        .flags                   = 0,
        .magFilter               = VK_FILTER_LINEAR,
        .minFilter               = VK_FILTER_LINEAR,
        .mipmapMode              = VK_SAMPLER_MIPMAP_MODE_LINEAR,
        .addressModeU            = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeV            = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeW            = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .mipLodBias              = 0.0f,
        .anisotropyEnable        = VK_TRUE, 
        .maxAnisotropy           = 1.0f,
        .compareEnable           = VK_FALSE,
        .compareOp               = VK_COMPARE_OP_ALWAYS,
        .minLod                  = 0.0f,
        .maxLod                  = 0.0f,
        .borderColor             = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
        .unnormalizedCoordinates = VK_FALSE,
    };

    VK_ASSERT_THROW(vkCreateSampler(device, &sampler_info, nullptr, &m_sampler), "Failed to create image sampler for texture");
}

Texture::Texture(Texture&& other) noexcept
    : m_handle    (std::exchange(other.m_handle,     nullptr))
    , m_view      (std::exchange(other.m_view,       nullptr))
    , m_sampler   (std::exchange(other.m_sampler,    nullptr))
    , m_allocation(std::exchange(other.m_allocation, nullptr))
    , m_allocator (std::exchange(other.m_allocator,  nullptr))
{}

Texture& Texture::operator = (Texture&& other) noexcept
{
    if (this != &other)
    {
        std::swap(m_handle,     other.m_handle);
        std::swap(m_view,       other.m_view);
        std::swap(m_sampler,    other.m_sampler);
        std::swap(m_allocation, other.m_allocation);
        std::swap(m_allocator,  other.m_allocator);
    }
    return *this;
}

Texture::~Texture()
{
    if (m_view || m_sampler)
    {
        VmaAllocatorInfo allocator_info;
        vmaGetAllocatorInfo(m_allocator, &allocator_info);

        vkDestroyImageView(allocator_info.device, m_view, nullptr);
        vkDestroySampler(allocator_info.device, m_sampler, nullptr);
        m_view    = nullptr;
        m_sampler = nullptr;
    }

    if (m_handle)
    {
        vmaDestroyImage(m_allocator, m_handle, m_allocation);
        m_handle    = nullptr;
    }
    m_allocator = nullptr;
}

} // namespace Cr::Graphics::Vulkan
