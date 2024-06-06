#include "Graphics/Vulkan/API.hpp"
#include "Graphics/Vulkan/Extension.hpp"

// #include "Shared/Filesystem.hpp"

#include "Graphics/Vulkan/Debug.hpp"

#include "ktx.h"

#include <cstring>
#include <iostream>
#include <vector>
#include <optional>
#include <algorithm>
#include <limits>
#include <set>

namespace Cr::Graphics::Vulkan
{

API::API(GLFWwindow* surface_context, bool debug)
{
    u32 version;
    VK_ASSERT_THROW(vkEnumerateInstanceVersion(&version), "Failed to get Vulkan version")

    CR_ASSERT_THROW(version < VK_VERSION_1_3, "Not a supported vulkan version")

    u32 glfw_extension_count;
    const char** glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);

    std::vector<const char*> instance_extensions(glfw_extensions, glfw_extensions + glfw_extension_count);
    std::vector<const char*> validation_layers;

    if (debug == true)
    {
        instance_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        
        validation_layers.push_back("VK_LAYER_KHRONOS_validation");

        u32 available_layer_count;
        VK_ASSERT_THROW(vkEnumerateInstanceLayerProperties(&available_layer_count, nullptr), "Failed to get VkLayerProperties") 

        std::vector<VkLayerProperties> available_layers(available_layer_count);
        VK_ASSERT_THROW(vkEnumerateInstanceLayerProperties(&available_layer_count, available_layers.data()), "Failed to get VkLayerProperties") 

        for (auto it = validation_layers.begin(); it != validation_layers.end(); ++it)
        {
            bool is_invalid_layer = true;
            for (const auto& layer : available_layers)
            {
                if (strcmp(*it, layer.layerName) == 0)
                {
                    is_invalid_layer = false;
                    break;
                }
            }
            if (is_invalid_layer)
            {
                validation_layers.erase(it);
            }
        }
    }

    // TODO Parameterize appinfo, layers etc.
    VkApplicationInfo application_info{};
    application_info.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    application_info.pApplicationName   = "CrunchTest";
    application_info.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
    application_info.pEngineName        = "CrunchEngine";
    application_info.engineVersion      = VK_MAKE_VERSION(0, 0, 1);
    application_info.apiVersion         = VK_API_VERSION_1_3;

    VkInstanceCreateInfo instance_info {};
    instance_info.sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instance_info.pApplicationInfo        = &application_info;
    instance_info.enabledLayerCount       = static_cast<u32>(validation_layers.size());
    instance_info.ppEnabledLayerNames     = validation_layers.data();
    instance_info.enabledExtensionCount   = static_cast<u32>(instance_extensions.size());
    instance_info.ppEnabledExtensionNames = instance_extensions.data();

    VK_ASSERT_THROW(vkCreateInstance(&instance_info, nullptr, &m_instance), "Failed to create a Vulkan instance");

    // Need to do device extension stuff separately?
    VK_ASSERT_THROW(Vulkan::Extension::bind_instance_extension_functions(m_instance), "Failed to bind extensions function calls")

    if (debug == true)
    {
        VkDebugUtilsMessengerCreateInfoEXT debug_messenger_info{};
        debug_messenger_info.sType           = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        debug_messenger_info.pNext           = nullptr;
        debug_messenger_info.flags           = 0;
        debug_messenger_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                                               VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                               VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        debug_messenger_info.messageType     = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT    |
                                               VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                               VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        debug_messenger_info.pfnUserCallback = API::debug_callback;
        debug_messenger_info.pUserData       = nullptr;

        VK_ASSERT_THROW(Vulkan::Extension::CreateDebugUtilsMessengerEXT(m_instance, &debug_messenger_info, nullptr, &m_debug_messenger), "Failed to create a debug messenger")
    }

    VK_ASSERT_THROW(glfwCreateWindowSurface(m_instance, surface_context, nullptr, &m_surface), "Failed to create Vulkan surface") 

    // Physical device

    // TODO should be figured out more dynamically later on
    std::vector<const char*> device_extensions
    {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
    };

    // Should maybe pack these and other useful information some GPU info structure
    QueueFamilyIndices indices{};
    SwapChainSupportDetails swap_chain_details{};

    u32 device_count;
    VK_ASSERT_THROW(vkEnumeratePhysicalDevices(m_instance, &device_count, nullptr), "Failed to get fetch devices");

    std::vector<VkPhysicalDevice> device_list{device_count};
    VK_ASSERT_THROW(vkEnumeratePhysicalDevices(m_instance, &device_count, device_list.data()), "Failed to get fetch devices");

    m_physical_device = nullptr;
    for (auto& device : device_list)
    {
        // Confirm properties and features

        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(device, &properties);

        if (properties.deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU ||
            properties.apiVersion <  VK_VERSION_1_3)
            continue;

        VkPhysicalDeviceFeatures features;
        vkGetPhysicalDeviceFeatures(device, &features);

        if (features.samplerAnisotropy != VK_TRUE)
            continue;

        // Confirm queue families

        std::optional<u32> graphics;
        std::optional<u32> presentation;
        
        u32 family_count;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &family_count, nullptr);

        std::vector<VkQueueFamilyProperties> family_properties(family_count);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &family_count, family_properties.data());

        for (std::size_t i = 0; i < family_properties.size(); ++i)
        {
            VkBool32 supports_presentation = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_surface, &supports_presentation);

            if (!presentation.has_value() && supports_presentation)
            {
                presentation = i;
            }
            if (!graphics.has_value() && (family_properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT))
            {
                graphics = i;
            }
            if (graphics.has_value() && presentation.has_value())
            {
                break;
            }
        }

        if (!graphics.has_value() || !presentation.has_value())
            continue;

        indices.graphics = graphics.value();
        indices.presentation = presentation.value();

        // Confirm extension support

        u32 extension_count;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, nullptr);

        std::vector<VkExtensionProperties> extension_properties{extension_count};
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, extension_properties.data());

        u32 extensions_found = 0;
        for (const char* extension : device_extensions)
        {
            for (const auto& property : extension_properties)
            {
                if (std::strcmp(extension, property.extensionName) == 0)
                {
                    ++extensions_found;;
                    break;
                }
            }
        }

        if (extensions_found != device_extensions.size())
        {
            continue;
        }

        // Confirm Swap chain support
        VK_ASSERT_THROW(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_surface, &swap_chain_details.capabilities), "Failed to fetch physical device surface capabilities")

        u32 format_count;
        VK_ASSERT_THROW(vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &format_count, nullptr), "Failed to get physical device surface format count")
        
        if (format_count > 0)
        {
            swap_chain_details.formats.resize(format_count);
            VK_ASSERT_THROW(vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &format_count, swap_chain_details.formats.data()), "Failed to get physical device surface formats")
        }

        u32 present_mode_count;
        VK_ASSERT_THROW(vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &present_mode_count, nullptr), "Failed to get physical device surface present_mode count")
        
        if (present_mode_count > 0)
        {
            swap_chain_details.present_modes.resize(present_mode_count);
            VK_ASSERT_THROW(vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &present_mode_count, swap_chain_details.present_modes.data()), "Failed to get physical device surface present_modes")
        }

        if (swap_chain_details.formats.empty() || swap_chain_details.present_modes.empty())
        {
            continue;
        }
        m_physical_device = device;
    }
    
    CR_ASSERT_THROW(m_physical_device == nullptr, "No suitable Vulkan device found.")

    vkGetPhysicalDeviceFeatures(m_physical_device, &m_physical_device_features);
    vkGetPhysicalDeviceProperties(m_physical_device, &m_physical_device_properties);

    CR_INFO("Suitable device:");
    CR_INFO(m_physical_device_properties.deviceName);

    std::set<u32> unique_indices { indices.graphics, indices.presentation };

    f32 queue_priority = 1.0f;
    
    std::vector<VkDeviceQueueCreateInfo> queue_info_list;
    for (const auto index : unique_indices)
    {
        VkDeviceQueueCreateInfo queue_info{};
        queue_info.sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_info.queueFamilyIndex = index;
        queue_info.queueCount       = 1;
        queue_info.pQueuePriorities = &queue_priority;

        queue_info_list.push_back(queue_info);
    }

    // Logical device

    VkPhysicalDeviceDynamicRenderingFeaturesKHR dynamic_rendering_feature {};
    dynamic_rendering_feature.sType            = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES_KHR;
    dynamic_rendering_feature.pNext            = nullptr;
    dynamic_rendering_feature.dynamicRendering = VK_TRUE;

    VkDeviceCreateInfo logical_device_info{};
    logical_device_info.sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    logical_device_info.pQueueCreateInfos       = queue_info_list.data();
    logical_device_info.queueCreateInfoCount    = static_cast<u32>(queue_info_list.size());
    logical_device_info.ppEnabledExtensionNames = device_extensions.data();
    logical_device_info.enabledExtensionCount   = static_cast<u32>(device_extensions.size());
    logical_device_info.pEnabledFeatures        = &m_physical_device_features;
    logical_device_info.pNext                   = &dynamic_rendering_feature;

    if (debug)
    {
        logical_device_info.ppEnabledLayerNames = validation_layers.data();
        logical_device_info.enabledLayerCount   = static_cast<u32>(validation_layers.size());
    }

    VK_ASSERT_THROW(vkCreateDevice(m_physical_device, &logical_device_info, nullptr, &m_device), "Failed to create logical device")

    (void)Extension::bind_device_extension_functions(m_device);

    vkGetDeviceQueue(m_device, indices.graphics, 0, &m_graphics_queue);
    vkGetDeviceQueue(m_device, indices.presentation, 0, &m_presentation_queue);

    // Create Vulkan Memory Allocator

    VmaAllocatorCreateInfo allocator_info
    {
        .flags = 0,
        .physicalDevice = m_physical_device,
        .device = m_device,
        .preferredLargeHeapBlockSize = 0,
        .pAllocationCallbacks = nullptr,
        .pDeviceMemoryCallbacks = nullptr,
        .pHeapSizeLimit = nullptr,
        .pVulkanFunctions = nullptr,
        .instance = m_instance,
        .vulkanApiVersion = 0,
        .pTypeExternalMemoryHandleTypes = nullptr,
    };

    VK_ASSERT_THROW(vmaCreateAllocator(&allocator_info, &m_allocator), "Failed to initialize VulkanMemoryAllocator")

    // Select swap chain format

    VkSurfaceFormatKHR surface_format;

    bool format_found = false;
    for (const auto& f : swap_chain_details.formats)
    {
        if (f.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR && f.format == VK_FORMAT_R8G8B8A8_SRGB)
        {
            surface_format = f;
            format_found = true;
            break;
        }
    }
    if (!format_found)
    {
        surface_format = swap_chain_details.formats[0];
    }

    // Select mode for presentation
    // TODO mode selection, FIFO should be guaranteed
    VkPresentModeKHR swap_present_mode = VK_PRESENT_MODE_FIFO_KHR;
    // VK_PRESENT_MODE_IMMEDIATE_KHR = as name implies
    // VK_PRESENT_MODE_MAILBOX_KHR = triple buffer
    // VK_PRESENT_MODE_FIFO_KHR = vsync
    // VK_PRESENT_MODE_FIFO_RELAXED_KHR =

    // Get extents for frames

    VkExtent2D swap_extent;
    const auto& capabilities = swap_chain_details.capabilities;

    if (capabilities.currentExtent.width == std::numeric_limits<u32>::max())
    {
        swap_extent = capabilities.currentExtent;
    }
    else
    {
        i32 width, height;
        glfwGetFramebufferSize(surface_context, &width, &height);
        swap_extent.width = std::clamp(static_cast<u32>(width), capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        swap_extent.height = std::clamp(static_cast<u32>(height), capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
    }

    // Get number for images for swap chain

    u32 image_count;
    if (swap_chain_details.capabilities.maxImageCount != 0)
    {
        image_count = std::clamp(4u, swap_chain_details.capabilities.minImageCount, swap_chain_details.capabilities.maxImageCount);
    }
    else
    {
        image_count = std::max(swap_chain_details.capabilities.minImageCount + 1u, 4u);
    }

    // Create swap chain

    VkSwapchainCreateInfoKHR swap_chain_info{};
    swap_chain_info.sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swap_chain_info.surface          = m_surface;
    swap_chain_info.presentMode      = swap_present_mode;
    swap_chain_info.imageExtent      = swap_extent;
    swap_chain_info.imageFormat      = surface_format.format;
    swap_chain_info.imageColorSpace  = surface_format.colorSpace;
    swap_chain_info.imageArrayLayers = 1;
    swap_chain_info.minImageCount    = image_count;
    swap_chain_info.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swap_chain_info.preTransform     = swap_chain_details.capabilities.currentTransform;
    swap_chain_info.compositeAlpha   = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swap_chain_info.clipped          = VK_TRUE;
    swap_chain_info.oldSwapchain     = nullptr; // Swapchain MUST be recreated and old referenced if resize etc happens!

    u32 temp_indices[] = {indices.graphics, indices.presentation};
    if (indices.graphics == indices.presentation)
    {
        swap_chain_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE; // Family owns the image, most performant case
    }
    else
    {
        swap_chain_info.imageSharingMode      = VK_SHARING_MODE_CONCURRENT; // Doesn't need explicit ownership transfer 
        swap_chain_info.queueFamilyIndexCount = 2;
        swap_chain_info.pQueueFamilyIndices   = temp_indices; 
    }

    VK_ASSERT_THROW(vkCreateSwapchainKHR(m_device, &swap_chain_info, nullptr, &m_swap_chain), "Failed to create a swap chain.")
    m_swap_format = surface_format.format;
    m_swap_extent = swap_extent;

    // Get the images of the swap chain 

    u32 swap_image_count;
    VK_ASSERT_THROW(vkGetSwapchainImagesKHR(m_device, m_swap_chain, &swap_image_count, nullptr), "Failed to fetch swap chain image count");

    m_swap_images.resize(swap_image_count);
    VK_ASSERT_THROW(vkGetSwapchainImagesKHR(m_device, m_swap_chain, &swap_image_count, m_swap_images.data()), "Failed to fetch swap chain images");

    m_swap_image_views.resize(m_swap_images.size());

    // Create views for the images
    for (std::size_t i = 0; i < m_swap_images.size(); ++i)
    {
        VkImageViewCreateInfo image_info{};
        image_info.sType    = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        image_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        image_info.image    = m_swap_images[i];
        image_info.format   = m_swap_format;
        image_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        image_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        image_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        image_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        image_info.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        image_info.subresourceRange.baseMipLevel   = 0;
        image_info.subresourceRange.levelCount     = 1;
        image_info.subresourceRange.baseArrayLayer = 0;
        image_info.subresourceRange.layerCount     = 1;
        
        VK_ASSERT_THROW(vkCreateImageView(m_device, &image_info, nullptr, &m_swap_image_views[i]), "Failed to create swap chain image view")
    }

//    // Create descriptor pool (Shader uniform buffers)
//
//    VkDescriptorPoolSize descriptor_pool_size{};
//    descriptor_pool_size.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
//    descriptor_pool_size.descriptorCount = FRAMES_IN_FLIGHT;
//
//    VkDescriptorPoolCreateInfo descriptor_pool_info{};
//    descriptor_pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
//    descriptor_pool_info.pNext = nullptr;
//    descriptor_pool_info.flags = 0;
//    descriptor_pool_info.maxSets = FRAMES_IN_FLIGHT;
//    descriptor_pool_info.poolSizeCount = 1;
//    descriptor_pool_info.pPoolSizes = &descriptor_pool_size;
//
//    VK_ASSERT_THROW(vkCreateDescriptorPool(m_device, &descriptor_pool_info, nullptr, &m_descriptor_pool), "Failed to create descriptor pool")

    // Create command pool

    VkCommandPoolCreateInfo command_pool_info {};
    command_pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    command_pool_info.pNext = nullptr;
    command_pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    command_pool_info.queueFamilyIndex = indices.graphics;

    VK_ASSERT_THROW(vkCreateCommandPool(m_device, &command_pool_info, nullptr, &m_command_pool), "Failed to create command pool")

    // Create command buffers and sync objects

    VkCommandBufferAllocateInfo command_buffer_info {};
    command_buffer_info.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    command_buffer_info.pNext              = nullptr;
    command_buffer_info.commandPool        = m_command_pool;
    command_buffer_info.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY; // Submit to queue, cannot be called from other buffers
    command_buffer_info.commandBufferCount = FRAMES_IN_FLIGHT;

    VK_ASSERT_THROW(vkAllocateCommandBuffers(m_device, &command_buffer_info, m_command_buffer.data()), "Failed to allocate command buffer") 

    VkSemaphoreCreateInfo semaphore_info {};
    semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    semaphore_info.pNext = nullptr;
    semaphore_info.flags = 0;

    VkFenceCreateInfo fence_info {};
    fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_info.pNext = nullptr;
    fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (std::size_t frame = 0; frame < FRAMES_IN_FLIGHT; ++frame)
    {
        VK_ASSERT_THROW((vkCreateSemaphore(m_device, &semaphore_info, nullptr, &m_image_available_semaphore[frame]) |
                         vkCreateSemaphore(m_device, &semaphore_info, nullptr, &m_render_finished_semaphore[frame]) |
                         vkCreateFence(    m_device, &fence_info,     nullptr, &m_in_flight_fence[frame])), "Failed to create synchronization objects")
    }

    CR_INFO("Vulkan initialized")
}

API::~API()
{
    vkDeviceWaitIdle(m_device);

    // TODO Most of this is really stupid and should be moved to pools or other lifetime management methods
    
    for (auto& image : m_image_pool)
    {
        if (image.sampler) vkDestroySampler(m_device, image.sampler, nullptr);
        if (image.view)    vkDestroyImageView(m_device, image.view, nullptr);
        if (image.handle)  vmaDestroyImage(m_allocator, image.handle, image.allocation);
    }

    for (auto& buffer : m_buffer_pool)
    {
        if (buffer.handle) {vmaDestroyBuffer(m_allocator, buffer.handle, buffer.allocation);}
    }

    for (auto fence : m_in_flight_fence)
    {
        vkDestroyFence(m_device, fence, nullptr);
    }

    for (auto semaphore : m_image_available_semaphore)
    {
        vkDestroySemaphore(m_device, semaphore, nullptr);
    }

    for (auto semaphore : m_render_finished_semaphore)
    {
        vkDestroySemaphore(m_device, semaphore, nullptr);
    }

//    vkDestroyDescriptorPool(m_device, m_descriptor_pool, nullptr);

    vkDestroyCommandPool(m_device, m_command_pool, nullptr);

    for (auto context : m_shader_pool)
    {
        vkDestroyPipelineLayout(m_device, context.pipeline_layout, nullptr);
        vkDestroyPipeline(m_device, context.pipeline, nullptr);
//        vkDestroyDescriptorSetLayout(m_device, context.descriptor_set_layout, nullptr);
//        for (auto& buffer : context.uniform_buffer_list)
//        {
//            destroy_buffer(buffer);
//        }
    }

    for (auto image_view : m_swap_image_views)
    {
        vkDestroyImageView(m_device, image_view, nullptr);
    }

    vmaDestroyAllocator(m_allocator);

    vkDestroySwapchainKHR(m_device, m_swap_chain, nullptr);

    vkDestroyDevice(m_device, nullptr);

    vkDestroySurfaceKHR(m_instance, m_surface, nullptr);

    Vulkan::Extension::DestroyDebugUtilsMessengerEXT(m_instance, m_debug_messenger, nullptr);

    vkDestroyInstance(m_instance, nullptr);
}

ShaderID API::shader_create(const std::vector<u8>& vertex_spirv, const std::vector<u8>& fragment_spirv)
{
    VkShaderModule vert_module = create_shader_module(vertex_spirv);
    VkShaderModule frag_module = create_shader_module(fragment_spirv);

    VkPipelineShaderStageCreateInfo shader_stage_info[2] {};

    shader_stage_info[0].sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shader_stage_info[0].stage  = VK_SHADER_STAGE_VERTEX_BIT;
    shader_stage_info[0].module = vert_module;
    shader_stage_info[0].pName  = "main";

    shader_stage_info[1].sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shader_stage_info[1].stage  = VK_SHADER_STAGE_FRAGMENT_BIT;
    shader_stage_info[1].module = frag_module;
    shader_stage_info[1].pName  = "main";

    std::vector<VkDynamicState> dynamic_states
    {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR,
    };

    VkPipelineDynamicStateCreateInfo dynamic_state_info{};
    dynamic_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamic_state_info.dynamicStateCount = static_cast<u32>(dynamic_states.size());
    dynamic_state_info.pDynamicStates = dynamic_states.data();

    // Pipeline mesh format?

    VkVertexInputBindingDescription bind_descriptor {};
    bind_descriptor.binding = 0;
    bind_descriptor.stride = sizeof(Cr::Vertex);
    bind_descriptor.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    VkVertexInputAttributeDescription attribute_descriptor[2] {};
    attribute_descriptor[0].location = 0;
    attribute_descriptor[0].binding = 0;
    attribute_descriptor[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attribute_descriptor[0].offset = offsetof(Cr::Vertex, position);

    attribute_descriptor[1].location = 1;
    attribute_descriptor[1].binding = 0;
    attribute_descriptor[1].format = VK_FORMAT_R32G32_SFLOAT;
    attribute_descriptor[1].offset = offsetof(Cr::Vertex, uv);

    VkPipelineVertexInputStateCreateInfo vertex_input_info {};
    vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_input_info.pNext = nullptr;
    vertex_input_info.flags = 0;
    vertex_input_info.vertexBindingDescriptionCount   = 1;
    vertex_input_info.pVertexBindingDescriptions      = &bind_descriptor;
    vertex_input_info.vertexAttributeDescriptionCount = 2;
    vertex_input_info.pVertexAttributeDescriptions    = attribute_descriptor;

    VkPipelineInputAssemblyStateCreateInfo input_assembly_info{};
    input_assembly_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    input_assembly_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    input_assembly_info.primitiveRestartEnable = VK_FALSE;

    // Pipeline viewport context?

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = m_swap_extent.width;
    viewport.width = float(m_swap_extent.width);
    viewport.height = -float(m_swap_extent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};
    scissor.offset = {};
    scissor.extent = m_swap_extent;

    VkPipelineViewportStateCreateInfo viewport_info{};
    viewport_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport_info.viewportCount = 1;
    viewport_info.pViewports = &viewport;
    viewport_info.scissorCount = 1;
    viewport_info.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizer_info{};
    rasterizer_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer_info.depthClampEnable = VK_FALSE;
    rasterizer_info.rasterizerDiscardEnable = VK_FALSE;
    rasterizer_info.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer_info.lineWidth = 1.0f;
    rasterizer_info.cullMode = VK_CULL_MODE_FRONT_BIT;
    rasterizer_info.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer_info.depthBiasEnable = VK_FALSE;
    rasterizer_info.depthBiasConstantFactor = 0.0f;
    rasterizer_info.depthBiasClamp = 0.0f;
    rasterizer_info.depthBiasSlopeFactor = 0.0f;

    VkPipelineMultisampleStateCreateInfo multisampling_info{};
    multisampling_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling_info.sampleShadingEnable = VK_TRUE;
    multisampling_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling_info.minSampleShading = 1.0f;
    multisampling_info.pSampleMask = nullptr;
    multisampling_info.alphaToCoverageEnable = VK_FALSE;
    multisampling_info.alphaToOneEnable = VK_FALSE;

    // NOT USED YET
    //VkPipelineDepthStencilStateCreateInfo depth_stencil_info{};

    VkPipelineColorBlendAttachmentState color_blend_attachment{};
    color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
                                            VK_COLOR_COMPONENT_G_BIT |
                                            VK_COLOR_COMPONENT_B_BIT |
                                            VK_COLOR_COMPONENT_A_BIT;
    color_blend_attachment.blendEnable = VK_FALSE;
    color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
    color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;

    VkPipelineColorBlendStateCreateInfo color_blend_info{};
    color_blend_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    color_blend_info.logicOpEnable = VK_FALSE;
//    color_blend_info.logicOp = VK_LOGIC_OP_COPY;
    color_blend_info.attachmentCount = 1;
    color_blend_info.pAttachments = &color_blend_attachment;
    // color_blend_info.blendConstants[0] = 0.0f;

//    // Uniform buffer binding for shader
//    // TODO Implement SPIRV Reflection library from Khronos to automate this for shader creation pipelines.
//    VkDescriptorSetLayoutBinding ubo_layout_binding{};
//    ubo_layout_binding.binding = 0;
//    ubo_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
//    ubo_layout_binding.descriptorCount = 1;
//    ubo_layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
//    ubo_layout_binding.pImmutableSamplers = nullptr;
//
//    VkDescriptorSetLayoutCreateInfo ubo_layout_info{};
//    ubo_layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
//    ubo_layout_info.pNext = nullptr;
//    ubo_layout_info.flags = 0;
//    ubo_layout_info.bindingCount = 1;
//    ubo_layout_info.pBindings = &ubo_layout_binding;
//
//    VkDescriptorSetLayout ubo_layout;
//    VK_ASSERT_THROW(vkCreateDescriptorSetLayout(m_device, &ubo_layout_info, nullptr, &ubo_layout), "Failed to create DescriptorSetLayout");

    // Push constants

    VkPushConstantRange push_constant_range {};
    push_constant_range.offset = 0;
    push_constant_range.size = sizeof(PushConstantObject);
    push_constant_range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkPipelineLayoutCreateInfo pipeline_layout_info{};
    pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

    pipeline_layout_info.setLayoutCount = 0;
    pipeline_layout_info.pSetLayouts = nullptr;
//    pipeline_layout_info.setLayoutCount = 1;
//    pipeline_layout_info.pSetLayouts = &ubo_layout;

    pipeline_layout_info.pushConstantRangeCount = 1;
    pipeline_layout_info.pPushConstantRanges = &push_constant_range;

    VkPipelineLayout pipeline_layout;
    VK_ASSERT_THROW(vkCreatePipelineLayout(m_device, &pipeline_layout_info, nullptr, &pipeline_layout), "Failed to create pipeline layout")

    // For dynamic rendering extension
    VkPipelineRenderingCreateInfoKHR pipeline_rendering_info{};
    pipeline_rendering_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR;
    pipeline_rendering_info.colorAttachmentCount = 1;
    pipeline_rendering_info.pColorAttachmentFormats = &m_swap_format;

    // Create pipeline
    VkGraphicsPipelineCreateInfo pipeline_info{};
    pipeline_info.sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipeline_info.pNext               = &pipeline_rendering_info;
    pipeline_info.stageCount          = 2;
    pipeline_info.pStages             = shader_stage_info;
    pipeline_info.pVertexInputState   = &vertex_input_info;
    pipeline_info.pInputAssemblyState = &input_assembly_info;
    pipeline_info.pViewportState      = &viewport_info;
    pipeline_info.pRasterizationState = &rasterizer_info;
    pipeline_info.pMultisampleState   = &multisampling_info;
    pipeline_info.pDepthStencilState  = nullptr;
    pipeline_info.pColorBlendState    = &color_blend_info;
    pipeline_info.pDynamicState       = &dynamic_state_info;
    pipeline_info.layout              = pipeline_layout;
    pipeline_info.renderPass          = nullptr; // Not needed with dynamic rendering EXT
    pipeline_info.subpass             = 0;              // Same
    pipeline_info.basePipelineHandle  = nullptr;
    pipeline_info.basePipelineIndex   = -1;

    VkPipeline pipeline;
    VK_ASSERT_THROW(vkCreateGraphicsPipelines(m_device, nullptr, 1, &pipeline_info, nullptr, &pipeline), "Failed to create graphics pipeline")

    vkDestroyShaderModule(m_device, vert_module, nullptr);
    vkDestroyShaderModule(m_device, frag_module, nullptr);

    m_shader_pool.push_back({pipeline, pipeline_layout});

    return m_shader_pool.size() - 1;

// UNIFORM BUFFERS

//    m_shader_pool.push_back({pipeline, pipeline_layout, ubo_layout, {}, {}});

//    auto& uniform_buffer_list = m_shader_pool.back().uniform_buffer_list;
//    auto& descriptor_set_list = m_shader_pool.back().descriptor_set_list;
//
//    // Create uniform buffers
//    VkBufferCreateInfo buffer_info {};
//    buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
//    buffer_info.pNext = nullptr;
//    buffer_info.flags = 0;
//    buffer_info.size  = sizeof(PushConstants);
//    buffer_info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
//    buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
//    buffer_info.queueFamilyIndexCount = 0;
//    buffer_info.pQueueFamilyIndices = nullptr;
//
//    for (auto& buffer : uniform_buffer_list)
//    {
//        buffer = create_buffer(buffer_info);
//    }

    // Create descriptor sets
//    std::array<VkDescriptorSetLayout, FRAMES_IN_FLIGHT> descriptor_set_layouts;
//    descriptor_set_layouts.fill(ubo_layout);
//
//    VkDescriptorSetAllocateInfo descriptor_set_info {};
//    descriptor_set_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
//    descriptor_set_info.pNext = nullptr;
//    descriptor_set_info.descriptorPool = m_descriptor_pool;
//    descriptor_set_info.descriptorSetCount = FRAMES_IN_FLIGHT;
//    descriptor_set_info.pSetLayouts = descriptor_set_layouts.data(); // NEEDS AS MANY LAYOUTS AS SETS!
//
//    VK_ASSERT_THROW(vkAllocateDescriptorSets(m_device, &descriptor_set_info, descriptor_set_list.data()), "Failed to allocate descriptor sets")

//    for (std::size_t i = 0; i < FRAMES_IN_FLIGHT; ++i)
//    {
//        VkDescriptorBufferInfo descriptor_buffer_info {};
//        descriptor_buffer_info.buffer = uniform_buffer_list[i].handle;
//        descriptor_buffer_info.offset = 0;
//        descriptor_buffer_info.range = sizeof(PushConstants);
//
//        VkWriteDescriptorSet descriptor_write {};
//        descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
//        descriptor_write.pNext = nullptr;
//        descriptor_write.dstSet = descriptor_set_list[i];
//        descriptor_write.dstBinding = 0;
//        descriptor_write.dstArrayElement = 0;
//        descriptor_write.descriptorCount = 1;
//        descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
//        //descriptor_write.pImageInfo = nullptr;
//        descriptor_write.pBufferInfo = &descriptor_buffer_info;
//        //descriptor_write.pTexelBufferView = nullptr;
//
//        vkUpdateDescriptorSets(m_device, 1, &descriptor_write, 0, nullptr);
//    }

//    return m_shader_pool.size() - 1;
}

void API::shader_destroy(ShaderID shader_id)
{
    // Should do these on renderer level
    auto& shader = m_shader_pool[shader_id];
    vkDestroyPipeline(m_device, shader.pipeline, nullptr);
    vkDestroyPipelineLayout(m_device, shader.pipeline_layout, nullptr);
//    vkDestroyDescriptorSetLayout(m_device, context.descriptor_set_layout, nullptr);
//    for (auto& buffer : context.uniform_buffer_list)
//    {
//        destroy_buffer(buffer);
//    }

    shader = {};
}

MeshID API::mesh_create(const std::vector<Vertex>& vertices, const std::vector<u32>& indices)
{
    // TODO Shouldn't create buffers per mesh, but lets do for now!

    const u64 vertex_buffer_size = sizeof(vertices[0]) * vertices.size();
    const u64 index_buffer_size = sizeof(indices[0]) * indices.size();

    MeshContext mesh {};
    mesh.vertex_buffer_id = this->buffer_create(Graphics::BufferType::VERTEX, vertex_buffer_size);
    mesh.index_buffer_id  = this->buffer_create(Graphics::BufferType::INDEX, index_buffer_size);

    // TODO BEGIN should be driver level thing
    auto& vertex_buffer = m_buffer_pool[mesh.vertex_buffer_id];
    auto& index_buffer  = m_buffer_pool[mesh.index_buffer_id];

    std::copy(vertices.cbegin(), vertices.cend(), static_cast<Cr::Vertex*>(vertex_buffer.data));
    std::copy(indices.cbegin(), indices.cend(), static_cast<u32*>(index_buffer.data));

    VK_ASSERT_THROW(vmaFlushAllocation(m_allocator, vertex_buffer.allocation, 0, VK_WHOLE_SIZE), "Failed to flush buffer allocation")
    VK_ASSERT_THROW(vmaFlushAllocation(m_allocator, index_buffer.allocation,  0, VK_WHOLE_SIZE), "Failed to flush buffer allocation")
    // END

    m_mesh_list.emplace_back(mesh);

    return m_mesh_list.size() - 1;
}

void API::mesh_destroy(MeshID mesh_id)
{
    // NOTE Should rather just free up the context from within the buffer... am I going to end up writing my own vk buffer allocator at some point?
    auto& mesh = m_mesh_list[mesh_id];

    this->buffer_destroy(mesh.vertex_buffer_id);
    this->buffer_destroy(mesh.index_buffer_id);

    // TODO Meshes don't have any sort of an invalidated state to work with, should be resolved by the ownership pool implementation honestly
}

u32 API::texture_create(const std::string& path)
{
    TextureContext texture;

    // TODO Move ktx import elsewhere
    ktxTexture2* ktx_handle;

    CR_ASSERT_THROW(ktxTexture2_CreateFromNamedFile(path.c_str(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &ktx_handle) != KTX_SUCCESS, "Failed to load ktx image");
    const Defer ktx_cleanup = [ktx_handle]{ ktxTexture_Destroy(ktxTexture(ktx_handle)); };

    const ktx_uint8_t* image_data = ktxTexture_GetData(ktxTexture(ktx_handle));
    const ktx_size_t   image_size = ktxTexture_GetDataSize(ktxTexture(ktx_handle));

    // Should we just have one of these statically?
    const BufferID staging_buffer_id = buffer_create(BufferType::STAGING, image_size);
    const Defer staging_buffer_cleanup = [this, staging_buffer_id] { this->buffer_destroy(staging_buffer_id); };

    const auto& staging_buffer = m_buffer_pool[staging_buffer_id];
    std::copy(image_data, image_data + image_size, static_cast<ktx_uint8_t*>(staging_buffer.data));

    vmaFlushAllocation(m_allocator, staging_buffer.allocation, 0, VK_WHOLE_SIZE);

    // NOTE This could work well as a builder pattern?
    VkImageCreateInfo image_info{};
    image_info.sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_info.pNext         = nullptr;
    image_info.flags         = 0;
    image_info.imageType     = VK_IMAGE_TYPE_2D;
    image_info.format        = static_cast<VkFormat>(ktx_handle->vkFormat);
    image_info.extent.width  = ktx_handle->baseWidth;
    image_info.extent.height = ktx_handle->baseHeight;
    image_info.extent.depth  = 1;
    image_info.mipLevels     = ktx_handle->numLevels;
    image_info.arrayLayers   = ktx_handle->numLayers;
    image_info.samples       = VK_SAMPLE_COUNT_1_BIT;
    image_info.tiling        = VK_IMAGE_TILING_OPTIMAL;
    image_info.usage         = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    image_info.sharingMode   = VK_SHARING_MODE_EXCLUSIVE;
//    image_info.queueFamilyIndexCount;
//    image_info.pQueueFamilyIndices;
    image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    VmaAllocationCreateInfo allocation_info {};
    allocation_info.flags           = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
    allocation_info.usage           = VMA_MEMORY_USAGE_AUTO;
    
    Image image;

    VK_ASSERT_THROW(vmaCreateImage(m_allocator, &image_info, &allocation_info, &image.handle, &image.allocation, nullptr), "Failed to create texture image");

    m_image_pool.push_back(image);
    
    texture.image_id = static_cast<decltype(texture.image_id)>(m_image_pool.size() - 1); // Overflow possibility, but unlikely

    // TODO: Command buffer abstraction
    // NOTE: Shouldn't have to create these for every single texture created
    //       but rather manage uploading multiple at once on renderer level

    VkCommandBuffer command_buffer;
    VkCommandBufferAllocateInfo command_buffer_info {};
    command_buffer_info.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    command_buffer_info.pNext              = nullptr;
    command_buffer_info.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY; // Submit to queue, cannot be called from other buffers
    command_buffer_info.commandPool        = m_command_pool;
    command_buffer_info.commandBufferCount = 1;

    VK_ASSERT_THROW(vkAllocateCommandBuffers(m_device, &command_buffer_info, &command_buffer), "Failed to allocate command buffer") 

    const Defer command_buffer_cleanup = [this, command_buffer]{
        vkFreeCommandBuffers(m_device, m_command_pool, 1, &command_buffer);
    };

    VkCommandBufferBeginInfo begin_info {};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    VK_ASSERT_THROW(vkBeginCommandBuffer(command_buffer, &begin_info), "Failure to begin image upload command buffer");

    { 
        VkImageMemoryBarrier image_memory_barrier
        {
            .sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .pNext               = nullptr,
            .srcAccessMask       = VK_ACCESS_NONE,                       // Wait for these accesses...
            .dstAccessMask       = VK_ACCESS_TRANSFER_WRITE_BIT,         // ...and before these ones... 
            .oldLayout           = VK_IMAGE_LAYOUT_UNDEFINED,            // ...change layout...
            .newLayout           = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, // ...to this!
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image               = image.handle,
            .subresourceRange
            {
                .aspectMask      = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel    = 0,
                .levelCount      = 1,
                .baseArrayLayer  = 0,
                .layerCount      = 1,
            },
        };
        vkCmdPipelineBarrier(
            command_buffer, 
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            0, 0, nullptr, 0, nullptr, 1, &image_memory_barrier
        );
    }

    VkBufferImageCopy region_info {}; 
    region_info.bufferOffset = 0;
    region_info.bufferRowLength = 0;
    region_info.bufferImageHeight = 0;

    region_info.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region_info.imageSubresource.mipLevel = 0;
    region_info.imageSubresource.baseArrayLayer = 0;
    region_info.imageSubresource.layerCount = 1;

    region_info.imageOffset = {0,0,0};
    region_info.imageExtent = image_info.extent;

    vkCmdCopyBufferToImage(command_buffer, staging_buffer.handle, image.handle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region_info);
            
    { 
        VkImageMemoryBarrier image_memory_barrier
        {
            .sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .pNext               = nullptr,
            .srcAccessMask       = VK_ACCESS_TRANSFER_WRITE_BIT, 
            .dstAccessMask       = VK_ACCESS_SHADER_READ_BIT,
            .oldLayout           = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            .newLayout           = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image               = image.handle,
            .subresourceRange
            {
                .aspectMask      = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel    = 0,
                .levelCount      = 1,
                .baseArrayLayer  = 0,
                .layerCount      = 1,
            },
        };
        vkCmdPipelineBarrier(command_buffer,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            0, 0, nullptr, 0, nullptr, 1, &image_memory_barrier
        );
    }

    vkEndCommandBuffer(command_buffer);

    VkSubmitInfo submit_info{};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.pCommandBuffers = &command_buffer;
    submit_info.commandBufferCount = 1;

    VK_ASSERT_THROW(vkQueueSubmit(m_graphics_queue, 1, &submit_info, VK_NULL_HANDLE), "Failed to submit command buffer");
    VK_ASSERT_THROW(vkQueueWaitIdle(m_graphics_queue), "Failure to wait for queue after image upload submission");

    VkImageViewCreateInfo view_info {};
    view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    view_info.pNext = VK_NULL_HANDLE;
    view_info.flags = 0;
    view_info.image = image.handle;
    view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    view_info.format = static_cast<VkFormat>(ktx_handle->vkFormat);
    view_info.components = {}; // Default rgb
    view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    view_info.subresourceRange.baseMipLevel = 0;
    view_info.subresourceRange.levelCount = 1;
    view_info.subresourceRange.baseArrayLayer = 0;
    view_info.subresourceRange.layerCount = 1;

    // Store into image for now... 
    VK_ASSERT_THROW(vkCreateImageView(m_device, &view_info, nullptr, &image.view), "Failed to create image view for texture");

    VkSamplerCreateInfo sampler_info {};
    sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    sampler_info.pNext = nullptr;
    sampler_info.flags = 0;
    sampler_info.magFilter  = VkFilter::VK_FILTER_LINEAR;
    sampler_info.minFilter  = VkFilter::VK_FILTER_LINEAR;
    sampler_info.addressModeU = VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.addressModeV = VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.addressModeW = VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.mipmapMode = VkSamplerMipmapMode::VK_SAMPLER_MIPMAP_MODE_LINEAR;
    sampler_info.mipLodBias = 0.0f;
    sampler_info.anisotropyEnable = VK_TRUE; 
    sampler_info.maxAnisotropy = m_physical_device_properties.limits.maxSamplerAnisotropy;
    sampler_info.compareEnable = VK_FALSE;
    sampler_info.compareOp = VK_COMPARE_OP_ALWAYS;
    sampler_info.minLod = 0.0f;
    sampler_info.maxLod = 0.0f;
    sampler_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    sampler_info.unnormalizedCoordinates = VK_FALSE;

    VK_ASSERT_THROW(vkCreateSampler(m_device, &sampler_info, nullptr, &image.sampler), "Failed to create image sampler for texture");

    m_texture_list.emplace_back(texture);
    return static_cast<TextureID>(m_texture_list.size() - 1); // Overflow danger
}

void API::texture_destroy(TextureID texture_id)
{
    TextureContext& texture = m_texture_list[texture_id];
    this->image_destroy(texture.image_id);

    // No invalidation state, should be done in ownership pool
}

void API::image_destroy(ImageID image_id)
{
    Image& image = m_image_pool[image_id];

    if (image.handle) vkDestroySampler(m_device, image.sampler, nullptr);
    if (image.view)   vkDestroyImageView(m_device, image.view, nullptr);
    if (image.handle) vmaDestroyImage(m_allocator, image.handle, image.allocation);
}

void API::begin_render()
{
    VkFence fence = m_in_flight_fence[m_current_frame];

    VkSemaphore image_available = m_image_available_semaphore[m_current_frame];

    VK_ASSERT_THROW(vkWaitForFences(m_device, 1, &fence, VK_TRUE, std::numeric_limits<u64>::max()), "Failed while waiting for fences")
    VK_ASSERT_THROW(vkResetFences(m_device, 1, &fence), "Failed to reset renderloop fence")

    VK_ASSERT_THROW(vkAcquireNextImageKHR(m_device, m_swap_chain, std::numeric_limits<u64>::max(), image_available, nullptr, &m_image_index), "Failed to acquire next image")

    VkCommandBuffer command_buffer = m_command_buffer[m_current_frame];

    VkCommandBufferBeginInfo begin_info {};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.pNext = nullptr;
    begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    begin_info.pInheritanceInfo = nullptr;

    VK_ASSERT_THROW(vkBeginCommandBuffer(command_buffer, &begin_info), "Failed to begin recording command buffer")

    VkRenderingAttachmentInfoKHR render_attachment_info{};
    render_attachment_info.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
    render_attachment_info.imageView = m_swap_image_views[m_image_index];
    render_attachment_info.imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;
    render_attachment_info.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    render_attachment_info.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    render_attachment_info.clearValue = {{{0.2f, 0.2f, 0.2f, 1.0f}}};

    VkRenderingInfoKHR render_info{};
    render_info.sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR;
    render_info.renderArea = {{0, 0}, m_swap_extent};
    render_info.layerCount = 1;
    render_info.colorAttachmentCount = 1;
    render_info.pColorAttachments = &render_attachment_info;

    {
        VkImageMemoryBarrier image_memory_barrier
        {
            .sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .pNext               = nullptr,
            .srcAccessMask       = VK_ACCESS_NONE,
            .dstAccessMask       = VK_ACCESS_NONE,
            .oldLayout           = VK_IMAGE_LAYOUT_UNDEFINED,
            .newLayout           = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .srcQueueFamilyIndex = 0,
            .dstQueueFamilyIndex = 0,
            .image               = m_swap_images[m_image_index],
            .subresourceRange
            {
                .aspectMask      = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel    = 0,
                .levelCount      = 1,
                .baseArrayLayer  = 0,
                .layerCount      = 1,
            },
        };

        vkCmdPipelineBarrier(
            command_buffer,
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            0, 0, nullptr, 0, nullptr, 1, &image_memory_barrier);
    }

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = float(m_swap_extent.height); // Offset flipped viewport
    viewport.width = float(m_swap_extent.width);
    viewport.height = -float(m_swap_extent.height); // Flip viewport
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    vkCmdSetViewport(command_buffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {};
    scissor.extent = m_swap_extent;

    vkCmdSetScissor(command_buffer, 0, 1, &scissor);

    Vulkan::Extension::CmdBeginRenderingKHR(command_buffer, &render_info);
}

void API::draw(MeshID mesh_id, ShaderID shader_id, PushConstantObject& uniforms)
{
    VkCommandBuffer command_buffer = m_command_buffer[m_current_frame];

    ShaderPipeline& shader = m_shader_pool[shader_id];

    vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, shader.pipeline);

    auto& mesh_context = m_mesh_list[mesh_id];

    VkBuffer vertex_buffer = m_buffer_pool[mesh_context.vertex_buffer_id].handle;
    VkBuffer index_buffer  = m_buffer_pool[mesh_context.index_buffer_id].handle;

    VkDeviceSize offset[] = {0};

    vkCmdBindVertexBuffers(command_buffer, 0, 1, &vertex_buffer, offset); 
    vkCmdBindIndexBuffer(command_buffer, index_buffer, 0, VK_INDEX_TYPE_UINT32);

    vkCmdPushConstants(command_buffer, shader.pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PushConstantObject), &uniforms);

//    VkDescriptorSet descriptor_set = shader.descriptor_set_list[m_current_frame];
//
//    vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, shader.pipeline_layout, 0, 1, &descriptor_set, 0, nullptr);

    vkCmdDrawIndexed(command_buffer, 36, 1, 0, 0, 0);

}

void API::end_render()
{
    VkFence fence = m_in_flight_fence[m_current_frame];

    VkSemaphore image_available = m_image_available_semaphore[m_current_frame];
    VkSemaphore render_finished = m_render_finished_semaphore[m_current_frame];

    VkCommandBuffer command_buffer = m_command_buffer[m_current_frame];

    Vulkan::Extension::CmdEndRenderingKHR(command_buffer);

    VkImageMemoryBarrier image_memory_barrier
    {
        .sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .pNext               = nullptr,
        .srcAccessMask       = VK_ACCESS_NONE,
        .dstAccessMask       = VK_ACCESS_NONE,
        .oldLayout           = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .newLayout           = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        .srcQueueFamilyIndex = 0,
        .dstQueueFamilyIndex = 0,
        .image               = m_swap_images[m_image_index],
        .subresourceRange
        {
            .aspectMask      = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel    = 0,
            .levelCount      = 1,
            .baseArrayLayer  = 0,
            .layerCount      = 1,
        },
    };

    vkCmdPipelineBarrier(
        command_buffer,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
        0, 0, nullptr, 0, nullptr, 1, &image_memory_barrier);

    VK_ASSERT_THROW(vkEndCommandBuffer(command_buffer), "Failed to end recording command buffer")

    // Update shader uniforms
//    UniformBufferObject* uniform_buffer = static_cast<UniformBufferObject*>(shader.uniform_buffer_list[m_current_frame].data);
//    *uniform_buffer = uniforms;
//
//    vmaFlushAllocation(m_allocator, shader.uniform_buffer_list[m_current_frame].all

    // Submit commands

    VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    VkSubmitInfo submit_info{};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = &image_available;
    submit_info.pWaitDstStageMask = &wait_stage;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &command_buffer;
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = &render_finished;

    VK_ASSERT_THROW(vkQueueSubmit(m_graphics_queue, 1, &submit_info, fence), "Failed to submit draw command buffer")

    //  Present image 

    VkPresentInfoKHR present_info{};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = &render_finished;
    present_info.swapchainCount = 1;
    present_info.pSwapchains = &m_swap_chain;
    present_info.pImageIndices = &m_image_index;
    present_info.pResults = nullptr;

    VK_ASSERT_THROW(vkQueuePresentKHR(m_presentation_queue, &present_info), "Failed to present")

    if (++m_current_frame == FRAMES_IN_FLIGHT)
        m_current_frame = 0;
}

VkShaderModule API::create_shader_module(const std::vector<u8>& spirv)
{
    VkShaderModuleCreateInfo shader_module_info{};
    shader_module_info.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shader_module_info.codeSize = spirv.size();
    shader_module_info.pCode    = reinterpret_cast<const uint32_t*>(spirv.data());

    VkShaderModule shader_module;
    VK_ASSERT_THROW(vkCreateShaderModule(m_device, &shader_module_info, nullptr, &shader_module), "Failed to create shader module")

    return shader_module;
};

BufferID API::buffer_create(Graphics::BufferType type, u64 size)
{
    VkBufferCreateInfo buffer_info {};
    buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    buffer_info.size = size;

    switch (type)
    {
        case Graphics::BufferType::VERTEX:
        {
            buffer_info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT; break;
        }
        case Graphics::BufferType::INDEX:
        {
            buffer_info.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT; break;
        }
        case Graphics::BufferType::STAGING:
        {
            buffer_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT; break;
        }
        default: CR_ASSERT_THROW(true, "Unimplemented buffer type!");
    };

    VmaAllocationCreateInfo allocation_info {};
    allocation_info.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
    allocation_info.usage = VMA_MEMORY_USAGE_AUTO;

    Buffer buffer;
    VmaAllocationInfo allocation_details;

    VK_ASSERT_THROW(vmaCreateBuffer(m_allocator, &buffer_info, &allocation_info, &buffer.handle, &buffer.allocation, &allocation_details), "Failed to create buffer");

    buffer.data = allocation_details.pMappedData;

    m_buffer_pool.emplace_back(buffer); // TODO Buffer isn't exception safe

    return static_cast<BufferID>(m_buffer_pool.size() - 1); // Overflowing u32 unlikely
}

void API::buffer_destroy(BufferID id)
{
    auto& buffer = m_buffer_pool[id];

    vmaInvalidateAllocation(m_allocator, buffer.allocation, 0, VK_WHOLE_SIZE);
    vmaDestroyBuffer(m_allocator, buffer.handle, buffer.allocation);

    buffer = {};
}

VKAPI_ATTR VkBool32 VKAPI_CALL API::debug_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT severity,
    VkDebugUtilsMessageTypeFlagsEXT type,
    const VkDebugUtilsMessengerCallbackDataEXT* p_callback_data,
    void* p_user_data)
{
    (void)p_user_data;

    auto& output = (severity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) ? std::cerr : std::cout;

    output << "VK_LOG_";

    switch (severity)
    {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT        : output << "VERBOSE" ; break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT           : output << "INFO"   ; break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT        : output << "WARNING"; break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT          : output << "ERROR"  ; break;
        default: output << "UNKNOWN";
    }

    output << '_';

    switch (type)
    {
        case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT                : output << "GENERAL"     ; break;
        case VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT             : output << "VALIDATION"  ; break;
        case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT            : output << "PERFORMANCE" ; break;
        case VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT : output << "DEVICE_BIND" ; break;
        default: output << "UNKNOWN";
    }

    output << ' ' << p_callback_data->pMessage << '\n';

    return VK_FALSE;
}

} // namespace Vk
