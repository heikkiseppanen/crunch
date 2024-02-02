#pragma once

#include <vulkan_core.h>
#include "Graphics/Vulkan/Allocator.hpp"

struct Image
{
    VkImage handle;
    VmaAllocation allocation;
    void* data; 
};

struct Buffer
{
    VkBuffer handle;
    VmaAllocation allocation;
    void* data; 
};

struct ShaderPipeline
{
    VkPipeline pipeline;
    VkPipelineLayout pipeline_layout;

    //VkDescriptorSetLayout descriptor_set_layout;
    //std::array<VkDescriptorSet, FRAMES_IN_FLIGHT> descriptor_set_list;
    //std::array<Buffer, FRAMES_IN_FLIGHT> uniform_buffer_list;
};
