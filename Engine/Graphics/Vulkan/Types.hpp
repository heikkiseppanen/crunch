#pragma once

#include <vulkan_core.h>

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
    VkPipeline pipeline;
    VkPipelineLayout pipeline_layout;

    //VkDescriptorSetLayout descriptor_set_layout;
    //std::array<VkDescriptorSet, FRAMES_IN_FLIGHT> descriptor_set_list;
    //std::array<Buffer, FRAMES_IN_FLIGHT> uniform_buffer_list;
};

} // namespace Cr::Vk
