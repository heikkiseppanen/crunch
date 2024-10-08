#include "Graphics/Vulkan/Vulkan.hpp"

namespace Cr::Graphics::Vulkan
{

std::vector<VkLayerProperties> get_instance_layer_properties()
{
    U32 property_count;
    VK_ASSERT_THROW(vkEnumerateInstanceLayerProperties(&property_count, nullptr), "Failed to get VkLayerProperties count");

    std::vector<VkLayerProperties> properties {property_count};
    VK_ASSERT_THROW(vkEnumerateInstanceLayerProperties(&property_count, properties.data()), "Failed to enumerate VkLayerProperties");

    return properties;
}

std::vector<VkPhysicalDevice> get_physical_devices(VkInstance instance)
{
    U32 device_count;
    VK_ASSERT_THROW(vkEnumeratePhysicalDevices(instance, &device_count, nullptr), "Failed to get VkPhysicalDevice count");

    std::vector<VkPhysicalDevice> devices {device_count};
    VK_ASSERT_THROW(vkEnumeratePhysicalDevices(instance, &device_count, devices.data()), "Failed to enumerate VkPhysicalDevices");

    return devices;
}

std::vector<VkExtensionProperties> get_physical_device_extension_properties(VkPhysicalDevice physical_device)
{
    U32 extension_count;
    VK_ASSERT_THROW(vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &extension_count, nullptr), "Failed to get VkExtensionProperties count");

    std::vector<VkExtensionProperties> extension_properties{extension_count};
    VK_ASSERT_THROW(vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &extension_count, extension_properties.data()), "Failed to enumerate VkExtensionProperties");

    return extension_properties;
}

std::vector<VkQueueFamilyProperties> get_physical_device_queue_properties(VkPhysicalDevice physical_device)
{
    U32 property_count;
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &property_count, nullptr);

    std::vector<VkQueueFamilyProperties> properties(property_count);
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &property_count, properties.data());

    return properties;
}

const char* to_string(VkResult result)
{
    switch (result)
    {
        case VK_SUCCESS: return "VK_SUCCESS";
        case VK_NOT_READY: return "VK_NOT_READY";
        case VK_TIMEOUT: return "VK_TIMEOUT";
        case VK_EVENT_SET: return "VK_EVENT_SET";
        case VK_EVENT_RESET: return "VK_EVENT_RESET";
        case VK_INCOMPLETE: return "VK_INCOMPLETE";
        case VK_ERROR_OUT_OF_HOST_MEMORY: return "VK_ERROR_OUT_OF_HOST_MEMORY";
        case VK_ERROR_OUT_OF_DEVICE_MEMORY: return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
        case VK_ERROR_INITIALIZATION_FAILED: return "VK_ERROR_INITIALIZATION_FAILED";
        case VK_ERROR_DEVICE_LOST: return "VK_ERROR_DEVICE_LOST";
        case VK_ERROR_MEMORY_MAP_FAILED: return "VK_ERROR_MEMORY_MAP_FAILED";
        case VK_ERROR_LAYER_NOT_PRESENT: return "VK_ERROR_LAYER_NOT_PRESENT";
        case VK_ERROR_EXTENSION_NOT_PRESENT: return "VK_ERROR_EXTENSION_NOT_PRESENT";
        case VK_ERROR_FEATURE_NOT_PRESENT: return "VK_ERROR_FEATURE_NOT_PRESENT";
        case VK_ERROR_INCOMPATIBLE_DRIVER: return "VK_ERROR_INCOMPATIBLE_DRIVER";
        case VK_ERROR_TOO_MANY_OBJECTS: return "VK_ERROR_TOO_MANY_OBJECTS";
        case VK_ERROR_FORMAT_NOT_SUPPORTED: return "VK_ERROR_FORMAT_NOT_SUPPORTED";
        case VK_ERROR_FRAGMENTED_POOL: return "VK_ERROR_FRAGMENTED_POOL";
        case VK_ERROR_UNKNOWN: return "VK_ERROR_UNKNOWN";
        case VK_ERROR_OUT_OF_POOL_MEMORY: return "VK_ERROR_OUT_OF_POOL_MEMORY";
        case VK_ERROR_INVALID_EXTERNAL_HANDLE: return "VK_ERROR_INVALID_EXTERNAL_HANDLE";
        case VK_ERROR_FRAGMENTATION: return "VK_ERROR_FRAGMENTATION";
        case VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS: return "VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS";
        case VK_PIPELINE_COMPILE_REQUIRED: return "VK_PIPELINE_COMPILE_REQUIRED";
        case VK_ERROR_SURFACE_LOST_KHR: return "VK_ERROR_SURFACE_LOST_KHR";
        case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR: return "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR";
        case VK_SUBOPTIMAL_KHR: return "VK_SUBOPTIMAL_KHR";
        case VK_ERROR_OUT_OF_DATE_KHR: return "VK_ERROR_OUT_OF_DATE_KHR";
        case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR: return "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR";
        case VK_ERROR_VALIDATION_FAILED_EXT: return "VK_ERROR_VALIDATION_FAILED_EXT";
        case VK_ERROR_INVALID_SHADER_NV: return "VK_ERROR_INVALID_SHADER_NV";
        case VK_ERROR_IMAGE_USAGE_NOT_SUPPORTED_KHR: return "VK_ERROR_IMAGE_USAGE_NOT_SUPPORTED_KHR";
        case VK_ERROR_VIDEO_PICTURE_LAYOUT_NOT_SUPPORTED_KHR: return "VK_ERROR_VIDEO_PICTURE_LAYOUT_NOT_SUPPORTED_KHR";
        case VK_ERROR_VIDEO_PROFILE_OPERATION_NOT_SUPPORTED_KHR: return "VK_ERROR_VIDEO_PROFILE_OPERATION_NOT_SUPPORTED_KHR";
        case VK_ERROR_VIDEO_PROFILE_FORMAT_NOT_SUPPORTED_KHR: return "VK_ERROR_VIDEO_PROFILE_FORMAT_NOT_SUPPORTED_KHR";
        case VK_ERROR_VIDEO_PROFILE_CODEC_NOT_SUPPORTED_KHR: return "VK_ERROR_VIDEO_PROFILE_CODEC_NOT_SUPPORTED_KHR";
        case VK_ERROR_VIDEO_STD_VERSION_NOT_SUPPORTED_KHR: return "VK_ERROR_VIDEO_STD_VERSION_NOT_SUPPORTED_KHR";
        case VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT: return "VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT";
        case VK_ERROR_NOT_PERMITTED_KHR: return "VK_ERROR_NOT_PERMITTED_KHR";
        case VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT: return "VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT";
        case VK_THREAD_IDLE_KHR: return "VK_THREAD_IDLE_KHR";
        case VK_THREAD_DONE_KHR: return "VK_THREAD_DONE_KHR";
        case VK_OPERATION_DEFERRED_KHR: return "VK_OPERATION_DEFERRED_KHR";
        case VK_OPERATION_NOT_DEFERRED_KHR: return "VK_OPERATION_NOT_DEFERRED_KHR";
        case VK_ERROR_INVALID_VIDEO_STD_PARAMETERS_KHR: return "VK_ERROR_INVALID_VIDEO_STD_PARAMETERS_KHR";
        case VK_ERROR_COMPRESSION_EXHAUSTED_EXT: return "VK_ERROR_COMPRESSION_EXHAUSTED_EXT";
        case VK_INCOMPATIBLE_SHADER_BINARY_EXT: return "VK_INCOMPATIBLE_SHADER_BINARY_EXT";

        default:
        {
            CR_ASSERT(false, "Unknown VkResult value: {}", static_cast<I32>(result));
            return "UNKNOWN";
        }
    }
}

const char* to_string(VkFormat format)
{
    switch(format)
    {
        case VK_FORMAT_R4G4_UNORM_PACK8: return "R4G4_UNORM_PACK8";
        case VK_FORMAT_R4G4B4A4_UNORM_PACK16: return "R4G4B4A4_UNORM_PACK16";
        case VK_FORMAT_B4G4R4A4_UNORM_PACK16: return "B4G4R4A4_UNORM_PACK16";
        case VK_FORMAT_R5G6B5_UNORM_PACK16: return "R5G6B5_UNORM_PACK16";
        case VK_FORMAT_B5G6R5_UNORM_PACK16: return "B5G6R5_UNORM_PACK16";
        case VK_FORMAT_R5G5B5A1_UNORM_PACK16: return "R5G5B5A1_UNORM_PACK16";
        case VK_FORMAT_B5G5R5A1_UNORM_PACK16: return "B5G5R5A1_UNORM_PACK16";
        case VK_FORMAT_A1R5G5B5_UNORM_PACK16: return "A1R5G5B5_UNORM_PACK16";
        case VK_FORMAT_R8_UNORM: return "R8_UNORM";
        case VK_FORMAT_R8_SNORM: return "R8_SNORM";
        case VK_FORMAT_R8_USCALED: return "R8_USCALED";
        case VK_FORMAT_R8_SSCALED: return "R8_SSCALED";
        case VK_FORMAT_R8_UINT: return "R8_UINT";
        case VK_FORMAT_R8_SINT: return "R8_SINT";
        case VK_FORMAT_R8_SRGB: return "R8_SRGB";
        case VK_FORMAT_R8G8_UNORM: return "R8G8_UNORM";
        case VK_FORMAT_R8G8_SNORM: return "R8G8_SNORM";
        case VK_FORMAT_R8G8_USCALED: return "R8G8_USCALED";
        case VK_FORMAT_R8G8_SSCALED: return "R8G8_SSCALED";
        case VK_FORMAT_R8G8_UINT: return "R8G8_UINT";
        case VK_FORMAT_R8G8_SINT: return "R8G8_SINT";
        case VK_FORMAT_R8G8_SRGB: return "R8G8_SRGB";
        case VK_FORMAT_R8G8B8_UNORM: return "R8G8B8_UNORM";
        case VK_FORMAT_R8G8B8_SNORM: return "R8G8B8_SNORM";
        case VK_FORMAT_R8G8B8_USCALED: return "R8G8B8_USCALED";
        case VK_FORMAT_R8G8B8_SSCALED: return "R8G8B8_SSCALED";
        case VK_FORMAT_R8G8B8_UINT: return "R8G8B8_UINT";
        case VK_FORMAT_R8G8B8_SINT: return "R8G8B8_SINT";
        case VK_FORMAT_R8G8B8_SRGB: return "R8G8B8_SRGB";
        case VK_FORMAT_B8G8R8_UNORM: return "B8G8R8_UNORM";
        case VK_FORMAT_B8G8R8_SNORM: return "B8G8R8_SNORM";
        case VK_FORMAT_B8G8R8_USCALED: return "B8G8R8_USCALED";
        case VK_FORMAT_B8G8R8_SSCALED: return "B8G8R8_SSCALED";
        case VK_FORMAT_B8G8R8_UINT: return "B8G8R8_UINT";
        case VK_FORMAT_B8G8R8_SINT: return "B8G8R8_SINT";
        case VK_FORMAT_B8G8R8_SRGB: return "B8G8R8_SRGB";
        case VK_FORMAT_R8G8B8A8_UNORM: return "R8G8B8A8_UNORM";
        case VK_FORMAT_R8G8B8A8_SNORM: return "R8G8B8A8_SNORM";
        case VK_FORMAT_R8G8B8A8_USCALED: return "R8G8B8A8_USCALED";
        case VK_FORMAT_R8G8B8A8_SSCALED: return "R8G8B8A8_SSCALED";
        case VK_FORMAT_R8G8B8A8_UINT: return "R8G8B8A8_UINT";
        case VK_FORMAT_R8G8B8A8_SINT: return "R8G8B8A8_SINT";
        case VK_FORMAT_R8G8B8A8_SRGB: return "R8G8B8A8_SRGB";
        case VK_FORMAT_B8G8R8A8_UNORM: return "B8G8R8A8_UNORM";
        case VK_FORMAT_B8G8R8A8_SNORM: return "B8G8R8A8_SNORM";
        case VK_FORMAT_B8G8R8A8_USCALED: return "B8G8R8A8_USCALED";
        case VK_FORMAT_B8G8R8A8_SSCALED: return "B8G8R8A8_SSCALED";
        case VK_FORMAT_B8G8R8A8_UINT: return "B8G8R8A8_UINT";
        case VK_FORMAT_B8G8R8A8_SINT: return "B8G8R8A8_SINT";
        case VK_FORMAT_B8G8R8A8_SRGB: return "B8G8R8A8_SRGB";
        case VK_FORMAT_A8B8G8R8_UNORM_PACK32: return "A8B8G8R8_UNORM_PACK32";
        case VK_FORMAT_A8B8G8R8_SNORM_PACK32: return "A8B8G8R8_SNORM_PACK32";
        case VK_FORMAT_A8B8G8R8_USCALED_PACK32: return "A8B8G8R8_USCALED_PACK32";
        case VK_FORMAT_A8B8G8R8_SSCALED_PACK32: return "A8B8G8R8_SSCALED_PACK32";
        case VK_FORMAT_A8B8G8R8_UINT_PACK32: return "A8B8G8R8_UINT_PACK32";
        case VK_FORMAT_A8B8G8R8_SINT_PACK32: return "A8B8G8R8_SINT_PACK32";
        case VK_FORMAT_A8B8G8R8_SRGB_PACK32: return "A8B8G8R8_SRGB_PACK32";
        case VK_FORMAT_A2R10G10B10_UNORM_PACK32: return "A2R10G10B10_UNORM_PACK32";
        case VK_FORMAT_A2R10G10B10_SNORM_PACK32: return "A2R10G10B10_SNORM_PACK32";
        case VK_FORMAT_A2R10G10B10_USCALED_PACK32: return "A2R10G10B10_USCALED_PACK32";
        case VK_FORMAT_A2R10G10B10_SSCALED_PACK32: return "A2R10G10B10_SSCALED_PACK32";
        case VK_FORMAT_A2R10G10B10_UINT_PACK32: return "A2R10G10B10_UINT_PACK32";
        case VK_FORMAT_A2R10G10B10_SINT_PACK32: return "A2R10G10B10_SINT_PACK32";
        case VK_FORMAT_A2B10G10R10_UNORM_PACK32: return "A2B10G10R10_UNORM_PACK32";
        case VK_FORMAT_A2B10G10R10_SNORM_PACK32: return "A2B10G10R10_SNORM_PACK32";
        case VK_FORMAT_A2B10G10R10_USCALED_PACK32: return "A2B10G10R10_USCALED_PACK32";
        case VK_FORMAT_A2B10G10R10_SSCALED_PACK32: return "A2B10G10R10_SSCALED_PACK32";
        case VK_FORMAT_A2B10G10R10_UINT_PACK32: return "A2B10G10R10_UINT_PACK32";
        case VK_FORMAT_A2B10G10R10_SINT_PACK32: return "A2B10G10R10_SINT_PACK32";
        case VK_FORMAT_R16_UNORM: return "R16_UNORM";
        case VK_FORMAT_R16_SNORM: return "R16_SNORM";
        case VK_FORMAT_R16_USCALED: return "R16_USCALED";
        case VK_FORMAT_R16_SSCALED: return "R16_SSCALED";
        case VK_FORMAT_R16_UINT: return "R16_UINT";
        case VK_FORMAT_R16_SINT: return "R16_SINT";
        case VK_FORMAT_R16_SFLOAT: return "R16_SFLOAT";
        case VK_FORMAT_R16G16_UNORM: return "R16G16_UNORM";
        case VK_FORMAT_R16G16_SNORM: return "R16G16_SNORM";
        case VK_FORMAT_R16G16_USCALED: return "R16G16_USCALED";
        case VK_FORMAT_R16G16_SSCALED: return "R16G16_SSCALED";
        case VK_FORMAT_R16G16_UINT: return "R16G16_UINT";
        case VK_FORMAT_R16G16_SINT: return "R16G16_SINT";
        case VK_FORMAT_R16G16_SFLOAT: return "R16G16_SFLOAT";
        case VK_FORMAT_R16G16B16_UNORM: return "R16G16B16_UNORM";
        case VK_FORMAT_R16G16B16_SNORM: return "R16G16B16_SNORM";
        case VK_FORMAT_R16G16B16_USCALED: return "R16G16B16_USCALED";
        case VK_FORMAT_R16G16B16_SSCALED: return "R16G16B16_SSCALED";
        case VK_FORMAT_R16G16B16_UINT: return "R16G16B16_UINT";
        case VK_FORMAT_R16G16B16_SINT: return "R16G16B16_SINT";
        case VK_FORMAT_R16G16B16_SFLOAT: return "R16G16B16_SFLOAT";
        case VK_FORMAT_R16G16B16A16_UNORM: return "R16G16B16A16_UNORM";
        case VK_FORMAT_R16G16B16A16_SNORM: return "R16G16B16A16_SNORM";
        case VK_FORMAT_R16G16B16A16_USCALED: return "R16G16B16A16_USCALED";
        case VK_FORMAT_R16G16B16A16_SSCALED: return "R16G16B16A16_SSCALED";
        case VK_FORMAT_R16G16B16A16_UINT: return "R16G16B16A16_UINT";
        case VK_FORMAT_R16G16B16A16_SINT: return "R16G16B16A16_SINT";
        case VK_FORMAT_R16G16B16A16_SFLOAT: return "R16G16B16A16_SFLOAT";
        case VK_FORMAT_R32_UINT: return "R32_UINT";
        case VK_FORMAT_R32_SINT: return "R32_SINT";
        case VK_FORMAT_R32_SFLOAT: return "R32_SFLOAT";
        case VK_FORMAT_R32G32_UINT: return "R32G32_UINT";
        case VK_FORMAT_R32G32_SINT: return "R32G32_SINT";
        case VK_FORMAT_R32G32_SFLOAT: return "R32G32_SFLOAT";
        case VK_FORMAT_R32G32B32_UINT: return "R32G32B32_UINT";
        case VK_FORMAT_R32G32B32_SINT: return "R32G32B32_SINT";
        case VK_FORMAT_R32G32B32_SFLOAT: return "R32G32B32_SFLOAT";
        case VK_FORMAT_R32G32B32A32_UINT: return "R32G32B32A32_UINT";
        case VK_FORMAT_R32G32B32A32_SINT: return "R32G32B32A32_SINT";
        case VK_FORMAT_R32G32B32A32_SFLOAT: return "R32G32B32A32_SFLOAT";
        case VK_FORMAT_R64_UINT: return "R64_UINT";
        case VK_FORMAT_R64_SINT: return "R64_SINT";
        case VK_FORMAT_R64_SFLOAT: return "R64_SFLOAT";
        case VK_FORMAT_R64G64_UINT: return "R64G64_UINT";
        case VK_FORMAT_R64G64_SINT: return "R64G64_SINT";
        case VK_FORMAT_R64G64_SFLOAT: return "R64G64_SFLOAT";
        case VK_FORMAT_R64G64B64_UINT: return "R64G64B64_UINT";
        case VK_FORMAT_R64G64B64_SINT: return "R64G64B64_SINT";
        case VK_FORMAT_R64G64B64_SFLOAT: return "R64G64B64_SFLOAT";
        case VK_FORMAT_R64G64B64A64_UINT: return "R64G64B64A64_UINT";
        case VK_FORMAT_R64G64B64A64_SINT: return "R64G64B64A64_SINT";
        case VK_FORMAT_R64G64B64A64_SFLOAT: return "R64G64B64A64_SFLOAT";
        case VK_FORMAT_B10G11R11_UFLOAT_PACK32: return "B10G11R11_UFLOAT_PACK32";
        case VK_FORMAT_E5B9G9R9_UFLOAT_PACK32: return "E5B9G9R9_UFLOAT_PACK32";
        case VK_FORMAT_D16_UNORM: return "D16_UNORM";
        case VK_FORMAT_X8_D24_UNORM_PACK32: return "X8_D24_UNORM_PACK32";
        case VK_FORMAT_D32_SFLOAT: return "D32_SFLOAT";
        case VK_FORMAT_S8_UINT: return "S8_UINT";
        case VK_FORMAT_D16_UNORM_S8_UINT: return "D16_UNORM_S8_UINT";
        case VK_FORMAT_D24_UNORM_S8_UINT: return "D24_UNORM_S8_UINT";
        case VK_FORMAT_D32_SFLOAT_S8_UINT: return "D32_SFLOAT_S8_UINT";
        case VK_FORMAT_BC1_RGB_UNORM_BLOCK: return "BC1_RGB_UNORM_BLOCK";
        case VK_FORMAT_BC1_RGB_SRGB_BLOCK: return "BC1_RGB_SRGB_BLOCK";
        case VK_FORMAT_BC1_RGBA_UNORM_BLOCK: return "BC1_RGBA_UNORM_BLOCK";
        case VK_FORMAT_BC1_RGBA_SRGB_BLOCK: return "BC1_RGBA_SRGB_BLOCK";
        case VK_FORMAT_BC2_UNORM_BLOCK: return "BC2_UNORM_BLOCK";
        case VK_FORMAT_BC2_SRGB_BLOCK: return "BC2_SRGB_BLOCK";
        case VK_FORMAT_BC3_UNORM_BLOCK: return "BC3_UNORM_BLOCK";
        case VK_FORMAT_BC3_SRGB_BLOCK: return "BC3_SRGB_BLOCK";
        case VK_FORMAT_BC4_UNORM_BLOCK: return "BC4_UNORM_BLOCK";
        case VK_FORMAT_BC4_SNORM_BLOCK: return "BC4_SNORM_BLOCK";
        case VK_FORMAT_BC5_UNORM_BLOCK: return "BC5_UNORM_BLOCK";
        case VK_FORMAT_BC5_SNORM_BLOCK: return "BC5_SNORM_BLOCK";
        case VK_FORMAT_BC6H_UFLOAT_BLOCK: return "BC6H_UFLOAT_BLOCK";
        case VK_FORMAT_BC6H_SFLOAT_BLOCK: return "BC6H_SFLOAT_BLOCK";
        case VK_FORMAT_BC7_UNORM_BLOCK: return "BC7_UNORM_BLOCK";
        case VK_FORMAT_BC7_SRGB_BLOCK: return "BC7_SRGB_BLOCK";
        case VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK: return "ETC2_R8G8B8_UNORM_BLOCK";
        case VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK: return "ETC2_R8G8B8_SRGB_BLOCK";
        case VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK: return "ETC2_R8G8B8A1_UNORM_BLOCK";
        case VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK: return "ETC2_R8G8B8A1_SRGB_BLOCK";
        case VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK: return "ETC2_R8G8B8A8_UNORM_BLOCK";
        case VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK: return "ETC2_R8G8B8A8_SRGB_BLOCK";
        case VK_FORMAT_EAC_R11_UNORM_BLOCK: return "EAC_R11_UNORM_BLOCK";
        case VK_FORMAT_EAC_R11_SNORM_BLOCK: return "EAC_R11_SNORM_BLOCK";
        case VK_FORMAT_EAC_R11G11_UNORM_BLOCK: return "EAC_R11G11_UNORM_BLOCK";
        case VK_FORMAT_EAC_R11G11_SNORM_BLOCK: return "EAC_R11G11_SNORM_BLOCK";
        case VK_FORMAT_ASTC_4x4_UNORM_BLOCK: return "ASTC_4x4_UNORM_BLOCK";
        case VK_FORMAT_ASTC_4x4_SRGB_BLOCK: return "ASTC_4x4_SRGB_BLOCK";
        case VK_FORMAT_ASTC_5x4_UNORM_BLOCK: return "ASTC_5x4_UNORM_BLOCK";
        case VK_FORMAT_ASTC_5x4_SRGB_BLOCK: return "ASTC_5x4_SRGB_BLOCK";
        case VK_FORMAT_ASTC_5x5_UNORM_BLOCK: return "ASTC_5x5_UNORM_BLOCK";
        case VK_FORMAT_ASTC_5x5_SRGB_BLOCK: return "ASTC_5x5_SRGB_BLOCK";
        case VK_FORMAT_ASTC_6x5_UNORM_BLOCK: return "ASTC_6x5_UNORM_BLOCK";
        case VK_FORMAT_ASTC_6x5_SRGB_BLOCK: return "ASTC_6x5_SRGB_BLOCK";
        case VK_FORMAT_ASTC_6x6_UNORM_BLOCK: return "ASTC_6x6_UNORM_BLOCK";
        case VK_FORMAT_ASTC_6x6_SRGB_BLOCK: return "ASTC_6x6_SRGB_BLOCK";
        case VK_FORMAT_ASTC_8x5_UNORM_BLOCK: return "ASTC_8x5_UNORM_BLOCK";
        case VK_FORMAT_ASTC_8x5_SRGB_BLOCK: return "ASTC_8x5_SRGB_BLOCK";
        case VK_FORMAT_ASTC_8x6_UNORM_BLOCK: return "ASTC_8x6_UNORM_BLOCK";
        case VK_FORMAT_ASTC_8x6_SRGB_BLOCK: return "ASTC_8x6_SRGB_BLOCK";
        case VK_FORMAT_ASTC_8x8_UNORM_BLOCK: return "ASTC_8x8_UNORM_BLOCK";
        case VK_FORMAT_ASTC_8x8_SRGB_BLOCK: return "ASTC_8x8_SRGB_BLOCK";
        case VK_FORMAT_ASTC_10x5_UNORM_BLOCK: return "ASTC_10x5_UNORM_BLOCK";
        case VK_FORMAT_ASTC_10x5_SRGB_BLOCK: return "ASTC_10x5_SRGB_BLOCK";
        case VK_FORMAT_ASTC_10x6_UNORM_BLOCK: return "ASTC_10x6_UNORM_BLOCK";
        case VK_FORMAT_ASTC_10x6_SRGB_BLOCK: return "ASTC_10x6_SRGB_BLOCK";
        case VK_FORMAT_ASTC_10x8_UNORM_BLOCK: return "ASTC_10x8_UNORM_BLOCK";
        case VK_FORMAT_ASTC_10x8_SRGB_BLOCK: return "ASTC_10x8_SRGB_BLOCK";
        case VK_FORMAT_ASTC_10x10_UNORM_BLOCK: return "ASTC_10x10_UNORM_BLOCK";
        case VK_FORMAT_ASTC_10x10_SRGB_BLOCK: return "ASTC_10x10_SRGB_BLOCK";
        case VK_FORMAT_ASTC_12x10_UNORM_BLOCK: return "ASTC_12x10_UNORM_BLOCK";
        case VK_FORMAT_ASTC_12x10_SRGB_BLOCK: return "ASTC_12x10_SRGB_BLOCK";
        case VK_FORMAT_ASTC_12x12_UNORM_BLOCK: return "ASTC_12x12_UNORM_BLOCK";
        case VK_FORMAT_ASTC_12x12_SRGB_BLOCK: return "ASTC_12x12_SRGB_BLOCK";
        case VK_FORMAT_G8B8G8R8_422_UNORM: return "G8B8G8R8_422_UNORM";
        case VK_FORMAT_B8G8R8G8_422_UNORM: return "B8G8R8G8_422_UNORM";
        case VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM: return "G8_B8_R8_3PLANE_420_UNORM";
        case VK_FORMAT_G8_B8R8_2PLANE_420_UNORM: return "G8_B8R8_2PLANE_420_UNORM";
        case VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM: return "G8_B8_R8_3PLANE_422_UNORM";
        case VK_FORMAT_G8_B8R8_2PLANE_422_UNORM: return "G8_B8R8_2PLANE_422_UNORM";
        case VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM: return "G8_B8_R8_3PLANE_444_UNORM";
        case VK_FORMAT_R10X6_UNORM_PACK16: return "R10X6_UNORM_PACK16";
        case VK_FORMAT_R10X6G10X6_UNORM_2PACK16: return "R10X6G10X6_UNORM_2PACK16";
        case VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16: return "R10X6G10X6B10X6A10X6_UNORM_4PACK16";
        case VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16: return "G10X6B10X6G10X6R10X6_422_UNORM_4PACK16";
        case VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16: return "B10X6G10X6R10X6G10X6_422_UNORM_4PACK16";
        case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16: return "G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16";
        case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16: return "G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16";
        case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16: return "G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16";
        case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16: return "G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16";
        case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16: return "G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16";
        case VK_FORMAT_R12X4_UNORM_PACK16: return "R12X4_UNORM_PACK16";
        case VK_FORMAT_R12X4G12X4_UNORM_2PACK16: return "R12X4G12X4_UNORM_2PACK16";
        case VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16: return "R12X4G12X4B12X4A12X4_UNORM_4PACK16";
        case VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16: return "G12X4B12X4G12X4R12X4_422_UNORM_4PACK16";
        case VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16: return "B12X4G12X4R12X4G12X4_422_UNORM_4PACK16";
        case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16: return "G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16";
        case VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16: return "G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16";
        case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16: return "G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16";
        case VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16: return "G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16";
        case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16: return "G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16";
        case VK_FORMAT_G16B16G16R16_422_UNORM: return "G16B16G16R16_422_UNORM";
        case VK_FORMAT_B16G16R16G16_422_UNORM: return "B16G16R16G16_422_UNORM";
        case VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM: return "G16_B16_R16_3PLANE_420_UNORM";
        case VK_FORMAT_G16_B16R16_2PLANE_420_UNORM: return "G16_B16R16_2PLANE_420_UNORM";
        case VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM: return "G16_B16_R16_3PLANE_422_UNORM";
        case VK_FORMAT_G16_B16R16_2PLANE_422_UNORM: return "G16_B16R16_2PLANE_422_UNORM";
        case VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM: return "G16_B16_R16_3PLANE_444_UNORM";
        case VK_FORMAT_G8_B8R8_2PLANE_444_UNORM: return "G8_B8R8_2PLANE_444_UNORM";
        case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_444_UNORM_3PACK16: return "G10X6_B10X6R10X6_2PLANE_444_UNORM_3PACK16";
        case VK_FORMAT_G12X4_B12X4R12X4_2PLANE_444_UNORM_3PACK16: return "G12X4_B12X4R12X4_2PLANE_444_UNORM_3PACK16";
        case VK_FORMAT_G16_B16R16_2PLANE_444_UNORM: return "G16_B16R16_2PLANE_444_UNORM";
        case VK_FORMAT_A4R4G4B4_UNORM_PACK16: return "A4R4G4B4_UNORM_PACK16";
        case VK_FORMAT_A4B4G4R4_UNORM_PACK16: return "A4B4G4R4_UNORM_PACK16";
        case VK_FORMAT_ASTC_4x4_SFLOAT_BLOCK: return "ASTC_4x4_SFLOAT_BLOCK";
        case VK_FORMAT_ASTC_5x4_SFLOAT_BLOCK: return "ASTC_5x4_SFLOAT_BLOCK";
        case VK_FORMAT_ASTC_5x5_SFLOAT_BLOCK: return "ASTC_5x5_SFLOAT_BLOCK";
        case VK_FORMAT_ASTC_6x5_SFLOAT_BLOCK: return "ASTC_6x5_SFLOAT_BLOCK";
        case VK_FORMAT_ASTC_6x6_SFLOAT_BLOCK: return "ASTC_6x6_SFLOAT_BLOCK";
        case VK_FORMAT_ASTC_8x5_SFLOAT_BLOCK: return "ASTC_8x5_SFLOAT_BLOCK";
        case VK_FORMAT_ASTC_8x6_SFLOAT_BLOCK: return "ASTC_8x6_SFLOAT_BLOCK";
        case VK_FORMAT_ASTC_8x8_SFLOAT_BLOCK: return "ASTC_8x8_SFLOAT_BLOCK";
        case VK_FORMAT_ASTC_10x5_SFLOAT_BLOCK: return "ASTC_10x5_SFLOAT_BLOCK";
        case VK_FORMAT_ASTC_10x6_SFLOAT_BLOCK: return "ASTC_10x6_SFLOAT_BLOCK";
        case VK_FORMAT_ASTC_10x8_SFLOAT_BLOCK: return "ASTC_10x8_SFLOAT_BLOCK";
        case VK_FORMAT_ASTC_10x10_SFLOAT_BLOCK: return "ASTC_10x10_SFLOAT_BLOCK";
        case VK_FORMAT_ASTC_12x10_SFLOAT_BLOCK: return "ASTC_12x10_SFLOAT_BLOCK";
        case VK_FORMAT_ASTC_12x12_SFLOAT_BLOCK: return "ASTC_12x12_SFLOAT_BLOCK";
        case VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG: return "PVRTC1_2BPP_UNORM_BLOCK_IMG";
        case VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG: return "PVRTC1_4BPP_UNORM_BLOCK_IMG";
        case VK_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG: return "PVRTC2_2BPP_UNORM_BLOCK_IMG";
        case VK_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG: return "PVRTC2_4BPP_UNORM_BLOCK_IMG";
        case VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG: return "PVRTC1_2BPP_SRGB_BLOCK_IMG";
        case VK_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG: return "PVRTC1_4BPP_SRGB_BLOCK_IMG";
        case VK_FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG: return "PVRTC2_2BPP_SRGB_BLOCK_IMG";
        case VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG: return "PVRTC2_4BPP_SRGB_BLOCK_IMG";
        case VK_FORMAT_R16G16_SFIXED5_NV: return "R16G16_SFIXED5_NV";
        case VK_FORMAT_A1B5G5R5_UNORM_PACK16_KHR: return "A1B5G5R5_UNORM_PACK16_KHR";
        case VK_FORMAT_A8_UNORM_KHR: return "A8_UNORM_KHR";
        default: return "UNKNOWN";
    }
}

Graphics::Format to_engine_type(VkFormat format)
{
    switch (format)
    {
        case VK_FORMAT_R32G32B32A32_SFLOAT: return Format::FLOAT_R32G32B32A32;
        case VK_FORMAT_R32G32B32_SFLOAT:    return Format::FLOAT_R32G32B32;
        case VK_FORMAT_R32G32_SFLOAT:       return Format::FLOAT_R32G32;
        case VK_FORMAT_R32_SFLOAT:          return Format::FLOAT_R32;

        default:
        {
            CR_ASSERT(false, "No matching engine format for Vulkan format");
            return Format::UNDEFINED;
        }
    }
}

VkFormat to_native_type(Graphics::Format format)
{
    switch (format)
    {
        case Format::FLOAT_R32G32B32A32: return VK_FORMAT_R32G32B32A32_SFLOAT;
        case Format::FLOAT_R32G32B32:    return VK_FORMAT_R32G32B32_SFLOAT;
        case Format::FLOAT_R32G32:       return VK_FORMAT_R32G32_SFLOAT;
        case Format::FLOAT_R32:          return VK_FORMAT_R32_SFLOAT;

        default:
        {
            CR_ASSERT(false, "No matching Vulkan format for engine format");
            return VK_FORMAT_UNDEFINED;
        }
    }
}

} // namespace Cr::Graphics::Vulkan
