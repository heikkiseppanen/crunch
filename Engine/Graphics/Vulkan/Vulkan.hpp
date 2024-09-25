#pragma once

#include "Graphics/Graphics.hpp"

#include <vulkan/vulkan.h>

#define VK_ASSERT_THROW(RESULT, ...) CR_ASSERT_THROW(((RESULT) >= 0), __VA_ARGS__)

namespace Cr::Graphics::Vulkan
{

// TODO Move this stuff under instance/device wrapppers
std::vector<VkLayerProperties>       get_instance_layer_properties();
std::vector<VkPhysicalDevice>        get_physical_devices(VkInstance instance);
std::vector<VkExtensionProperties>   get_physical_device_extension_properties(VkPhysicalDevice physical_device);
std::vector<VkQueueFamilyProperties> get_physical_device_queue_properties(VkPhysicalDevice physical_device);

const char* to_string(VkFormat format);
const char* to_string(VkResult result);

Graphics::Format to_engine_type(VkFormat format);

VkFormat to_native_type(Graphics::Format format);

} // namespace Cr::Vulkan
