#pragma once

#include <array>

#include <vulkan.h>

#include "Graphics/Types.hpp"
#include "Graphics/Vulkan/Allocator.hpp"

namespace Cr::Graphics::Vulkan
{

struct Image
{
    VkImage handle = nullptr;
    VkImageView view = nullptr;
    VkSampler sampler = nullptr;
    VmaAllocation allocation = nullptr;
};

struct Buffer
{
    VkBuffer handle = nullptr;
    VmaAllocation allocation = nullptr;
    void* data = nullptr; 
};

struct ShaderPipeline
{
    VkPipeline handle = nullptr;
    VkPipelineLayout layout = nullptr;

    VkDescriptorSetLayout descriptor_set_layout = nullptr;
    std::array<VkDescriptorSet, 3> descriptor_set_list {};
    std::array<BufferID, 3> uniform_buffer_list {};
};

} // namespace Cr::Vk
