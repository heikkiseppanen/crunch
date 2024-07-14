#pragma once

#include "Crunch.hpp"

#include "Graphics/Vulkan/Allocator.hpp"

#include "Graphics/Types.hpp"

#include <vulkan.h>

#define VK_ASSERT_THROW(RESULT, ...) CR_ASSERT_THROW(((RESULT) >= 0), __VA_ARGS__)

namespace Cr::Graphics::Vulkan
{

struct Image
{
    VkImage handle = nullptr;
    VkImageView view = nullptr;
    VkSampler sampler = nullptr;
    VmaAllocation allocation = nullptr;
};

struct ShaderPipeline
{
    VkPipeline handle = nullptr;
    VkPipelineLayout layout = nullptr;

    VkDescriptorSetLayout descriptor_set_layout = nullptr;
    std::array<VkDescriptorSet, 3> descriptor_set_list {};
    std::array<BufferID, 3> uniform_buffer_list {};
};

std::vector<VkLayerProperties>       get_instance_layer_properties();
std::vector<VkPhysicalDevice>        get_physical_devices(VkInstance instance);
std::vector<VkExtensionProperties>   get_physical_device_extension_properties(VkPhysicalDevice physical_device);
std::vector<VkQueueFamilyProperties> get_physical_device_queue_properties(VkPhysicalDevice physical_device);

} // namespace Cr::Vulkan
