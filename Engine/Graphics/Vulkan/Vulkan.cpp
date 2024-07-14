#include "Graphics/Vulkan/Vulkan.hpp"

namespace Cr::Graphics::Vulkan
{

std::vector<VkLayerProperties> get_instance_layer_properties()
{
    u32 property_count;
    VK_ASSERT_THROW(vkEnumerateInstanceLayerProperties(&property_count, nullptr), "Failed to get VkLayerProperties count");

    std::vector<VkLayerProperties> properties {property_count};
    VK_ASSERT_THROW(vkEnumerateInstanceLayerProperties(&property_count, properties.data()), "Failed to enumerate VkLayerProperties");

    return properties;
}

std::vector<VkPhysicalDevice> get_physical_devices(VkInstance instance)
{
    u32 device_count;
    VK_ASSERT_THROW(vkEnumeratePhysicalDevices(instance, &device_count, nullptr), "Failed to get VkPhysicalDevice count");

    std::vector<VkPhysicalDevice> devices {device_count};
    VK_ASSERT_THROW(vkEnumeratePhysicalDevices(instance, &device_count, devices.data()), "Failed to enumerate VkPhysicalDevices");

    return devices;
}

std::vector<VkExtensionProperties> get_physical_device_extension_properties(VkPhysicalDevice physical_device)
{
    u32 extension_count;
    VK_ASSERT_THROW(vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &extension_count, nullptr), "Failed to get VkExtensionProperties count");

    std::vector<VkExtensionProperties> extension_properties{extension_count};
    VK_ASSERT_THROW(vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &extension_count, extension_properties.data()), "Failed to enumerate VkExtensionProperties");

    return extension_properties;
}

std::vector<VkQueueFamilyProperties> get_physical_device_queue_properties(VkPhysicalDevice physical_device)
{
    u32 property_count;
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &property_count, nullptr);

    std::vector<VkQueueFamilyProperties> properties(property_count);
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &property_count, properties.data());

    return properties;
}

} // namespace Cr::Graphics::Vulkan
