#include "Graphics/Vulkan/API.hpp"

#include "Graphics/Vulkan/Extensions.hpp"

#include "Core/Window.hpp"

#include <cstring>
#include <iostream>
#include <vector>
#include <optional>
#include <algorithm>
#include <limits>

namespace Cr::Graphics::Vulkan
{

// TODO Move this somewhere..
struct SwapChainSupportDetails
{
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> present_modes;
};

API::API(const Core::Window& surface_context, bool debug)
{
    U32 version;
    VK_ASSERT_THROW(vkEnumerateInstanceVersion(&version), "Failed to get Vulkan version");

    CR_ASSERT_THROW(version >= VK_VERSION_1_3, "Not a supported vulkan version");

    U32 glfw_extension_count;
    const char** glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);

    std::vector<const char*> instance_extensions(glfw_extensions, glfw_extensions + glfw_extension_count);
    std::vector<const char*> validation_layers;

    if (debug == true)
    {
        instance_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        
        validation_layers.push_back("VK_LAYER_KHRONOS_validation");

        auto layer_properties = get_instance_layer_properties();

        for (auto it = validation_layers.begin(); it != validation_layers.end(); ++it)
        {
            bool is_invalid_layer = true;
            for (const auto& property : layer_properties)
            {
                if (strcmp(*it, property.layerName) == 0)
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
    instance_info.enabledLayerCount       = static_cast<U32>(validation_layers.size());
    instance_info.ppEnabledLayerNames     = validation_layers.data();
    instance_info.enabledExtensionCount   = static_cast<U32>(instance_extensions.size());
    instance_info.ppEnabledExtensionNames = instance_extensions.data();

    VK_ASSERT_THROW(vkCreateInstance(&instance_info, nullptr, &m_instance), "Failed to create a Vulkan instance");

    VK_ASSERT_THROW(Extensions::bind_instance_extension_functions(m_instance), "Failed to bind extensions function calls");

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

        VK_ASSERT_THROW(Extensions::create_debug_utils_messenger(m_instance, &debug_messenger_info, nullptr, &m_debug_messenger), "Failed to create a debug messenger");
    }
    else
    {
        m_debug_messenger = nullptr;
    }

    VK_ASSERT_THROW(glfwCreateWindowSurface(m_instance, surface_context.get_native(), nullptr, &m_surface), "Failed to create Vulkan surface");

    // Physical device


    // Should maybe pack these and other useful information some GPU info structure
    U32 queue_family_index;
    SwapChainSupportDetails swap_chain_details{};

    m_physical_device = nullptr;
    for (auto device : get_physical_devices(m_instance))
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
        // TODO Query for dedicated families
        
        U32 family_index = 0;
        std::optional<U32> suitable_family;
        for (const auto& property : get_physical_device_queue_properties(device))
        {
            VkBool32 supports_presentation = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, family_index, m_surface, &supports_presentation);

            if ((property.queueFlags & VK_QUEUE_GRAPHICS_BIT) &&
                (property.queueFlags & VK_QUEUE_TRANSFER_BIT) &&
                supports_presentation)
            {
                suitable_family = family_index;
                break;
            }

            ++family_index;
        }

        if (!suitable_family.has_value()) { continue; }

        // Confirm extension support

        auto extension_properties = get_physical_device_extension_properties(device);

        U32 extensions_found = 0;
        for (const char* extension : Extensions::DEVICE_EXTENSION_LIST)
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

        if (extensions_found != Extensions::DEVICE_EXTENSION_LIST.size()) { continue; }

        // Confirm Swap chain support
        VK_ASSERT_THROW(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_surface, &swap_chain_details.capabilities), "Failed to fetch physical device surface capabilities");

        U32 format_count;
        VK_ASSERT_THROW(vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &format_count, nullptr), "Failed to get physical device surface format count");
        
        if (format_count > 0)
        {
            swap_chain_details.formats.resize(format_count);
            VK_ASSERT_THROW(vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &format_count, swap_chain_details.formats.data()), "Failed to get physical device surface formats");
        }

        U32 present_mode_count;
        VK_ASSERT_THROW(vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &present_mode_count, nullptr), "Failed to get physical device surface present_mode count");
        
        if (present_mode_count > 0)
        {
            swap_chain_details.present_modes.resize(present_mode_count);
            VK_ASSERT_THROW(vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &present_mode_count, swap_chain_details.present_modes.data()), "Failed to get physical device surface present_modes");
        }

        if (swap_chain_details.formats.empty() || swap_chain_details.present_modes.empty()) { continue; }

        m_physical_device = device;
        queue_family_index = suitable_family.value();
    }
    
    CR_ASSERT_THROW(m_physical_device != nullptr, "No suitable Vulkan device found.");

    vkGetPhysicalDeviceFeatures(m_physical_device, &m_physical_device_features);
    vkGetPhysicalDeviceProperties(m_physical_device, &m_physical_device_properties);

    CR_INFO("Suitable device: %s", m_physical_device_properties.deviceName);

    F32 queue_priority = 1.0f;
    std::array queue_info_list {
        VkDeviceQueueCreateInfo {
            .sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .queueFamilyIndex = queue_family_index,
            .queueCount       = 1,
            .pQueuePriorities = &queue_priority,
        },
    };

    // Logical device

    VkPhysicalDeviceDynamicRenderingFeaturesKHR dynamic_rendering_feature {};
    dynamic_rendering_feature.sType            = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES_KHR;
    dynamic_rendering_feature.pNext            = nullptr;
    dynamic_rendering_feature.dynamicRendering = VK_TRUE;

    VkDeviceCreateInfo logical_device_info{};
    logical_device_info.sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    logical_device_info.pQueueCreateInfos       = queue_info_list.data();
    logical_device_info.queueCreateInfoCount    = static_cast<U32>(queue_info_list.size());
    logical_device_info.ppEnabledExtensionNames = Extensions::DEVICE_EXTENSION_LIST.data();
    logical_device_info.enabledExtensionCount   = static_cast<U32>(Extensions::DEVICE_EXTENSION_LIST.size());
    logical_device_info.pEnabledFeatures        = &m_physical_device_features;
    logical_device_info.pNext                   = &dynamic_rendering_feature;

    if (debug)
    {
        logical_device_info.ppEnabledLayerNames = validation_layers.data();
        logical_device_info.enabledLayerCount   = static_cast<U32>(validation_layers.size());
    }

    VK_ASSERT_THROW(vkCreateDevice(m_physical_device, &logical_device_info, nullptr, &m_device), "Failed to create logical device");

    (void)Extensions::bind_device_extension_functions(m_device);

    m_queue = Queue(m_device, queue_family_index, 0);

    // Create Vulkan Memory Allocator

    VmaAllocatorCreateInfo allocator_info {
        .physicalDevice = m_physical_device,
        .device = m_device,
        .instance = m_instance,
    };

    VK_ASSERT_THROW(vmaCreateAllocator(&allocator_info, &m_allocator), "Failed to initialize VmaAllocator");

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

    if (capabilities.currentExtent.width == std::numeric_limits<U32>::max())
    {
        swap_extent = capabilities.currentExtent;
    }
    else
    {
        I32 width, height;
        glfwGetFramebufferSize(surface_context.get_native(), &width, &height);
        swap_extent.width  = std::clamp(static_cast<U32>(width), capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        swap_extent.height = std::clamp(static_cast<U32>(height), capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
    }

    // Get number for images for swap chain

    U32 image_count;
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
    swap_chain_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE; // Family owns the image, most performant case
    // TODO Once multiple queue families are implemented
    //swap_chain_info.imageSharingMode      = VK_SHARING_MODE_CONCURRENT; // Doesn't need explicit ownership transfer 
    //swap_chain_info.queueFamilyIndexCount = 2;
    //swap_chain_info.pQueueFamilyIndices   = temp_indices; 

    VK_ASSERT_THROW(vkCreateSwapchainKHR(m_device, &swap_chain_info, nullptr, &m_swap_chain), "Failed to create a swap chain.");
    m_swap_format = surface_format.format;
    m_swap_extent = swap_extent;

    // Get the images of the swap chain 

    U32 swap_image_count;
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
        
        VK_ASSERT_THROW(vkCreateImageView(m_device, &image_info, nullptr, &m_swap_image_views[i]), "Failed to create swap chain image view");
    }

//    // Create descriptor pool (Shader uniform buffers)

    std::array<VkDescriptorPoolSize, 2> descriptor_pool_size {
        VkDescriptorPoolSize {
            .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = 128,
        },
        VkDescriptorPoolSize {
            .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .descriptorCount = 128,
        }
    };

    VkDescriptorPoolCreateInfo descriptor_pool_info{};
    descriptor_pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptor_pool_info.pNext = nullptr;
    descriptor_pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    descriptor_pool_info.maxSets = 128;
    descriptor_pool_info.poolSizeCount = descriptor_pool_size.size();
    descriptor_pool_info.pPoolSizes    = descriptor_pool_size.data();

    VK_ASSERT_THROW(vkCreateDescriptorPool(m_device, &descriptor_pool_info, nullptr, &m_descriptor_pool), "Failed to create descriptor pool");

    // Create command buffers and sync objects

    // TODO SWAP CHAIN COMMAND BUFFERS

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
                         vkCreateFence(    m_device, &fence_info,     nullptr, &m_in_flight_fence[frame])), "Failed to create synchronization objects");
        m_command_buffer[frame] = m_queue.create_command_buffer();
    }

    CR_INFO("Vulkan initialized");
}

API::~API()
{
    if (m_device)
    {
        vkDeviceWaitIdle(m_device);

        // TODO Most of this is really stupid and should be moved to pools or other lifetime management methods

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

        vkDestroyDescriptorPool(m_device, m_descriptor_pool, nullptr);

        for (auto image_view : m_swap_image_views)
        {
            vkDestroyImageView(m_device, image_view, nullptr);
        }

        vkDestroySwapchainKHR(m_device, m_swap_chain, nullptr);

        vmaDestroyAllocator(m_allocator);

        m_command_buffer = {};
        m_queue = {}; // TODO temporary fix for lack of RAII destruction order...

        vkDestroyDevice(m_device, nullptr);
    }


    if (m_instance) 
    {

        vkDestroySurfaceKHR(m_instance, m_surface, nullptr);

        if (m_debug_messenger)
        {
            Extensions::destroy_debug_utils_messenger(m_instance, m_debug_messenger, nullptr);
        }

        vkDestroyInstance(m_instance, nullptr);
    }
}

[[nodiscard]] Vulkan::Queue& API::get_command_queue(VkQueueFlags features)
{
    (void)features;
    return m_queue; // TODO Handling of multiple specialized queues, use one for now
}

[[nodiscard]] Unique<Vulkan::Buffer> API::create_buffer(VkBufferCreateFlags usage, U64 size)
{
    return create_unique<Vulkan::Buffer>(m_allocator, usage, size); // TODO implement resource pooling
}

[[nodiscard]] Unique<Vulkan::ShaderModule> API::create_shader_module(std::span<const U8> spirv, VkShaderStageFlags stage)
{
    return create_unique<Vulkan::ShaderModule>(m_device, spirv, stage); // TODO implement resource pooling
}

[[nodiscard]] Unique<Vulkan::Shader> API::create_shader(VkPipelineBindPoint usage, std::span<const Vulkan::ShaderModule* const> modules)
{
    return create_unique<Vulkan::Shader>(m_device, m_swap_format, usage, modules); // TODO implement resource pooling
}

[[nodiscard]] Unique<Vulkan::Texture> API::create_texture(VkFormat format, VkExtent3D extent)
{
    return create_unique<Vulkan::Texture>(m_allocator, format, extent); // TODO implement resource pooling
}

Vulkan::CommandBuffer& API::begin_frame()
{
    VkFence fence = m_in_flight_fence[m_frame_index];

    VkSemaphore image_available = m_image_available_semaphore[m_frame_index];

    VK_ASSERT_THROW(vkWaitForFences(m_device, 1, &fence, VK_TRUE, std::numeric_limits<U64>::max()), "Failed while waiting for fences");
    VK_ASSERT_THROW(vkResetFences(  m_device, 1, &fence), "Failed to reset renderloop fence");

    VK_ASSERT_THROW(vkAcquireNextImageKHR(m_device, m_swap_chain, std::numeric_limits<U64>::max(), image_available, nullptr, &m_image_index), "Failed to acquire next image");

    auto& cmd = *m_command_buffer[m_frame_index];

    cmd.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

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

        cmd.pipeline_barrier( VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, {}, {}, {&image_memory_barrier, 1});
    }

    cmd.set_viewport(0, F32(m_swap_extent.height), F32(m_swap_extent.width), -F32(m_swap_extent.height)); // For inverted viewport
    cmd.set_scissor(0, 0, m_swap_extent.width, m_swap_extent.height);

    VkRenderingAttachmentInfoKHR render_attachment_info{};
    render_attachment_info.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
    render_attachment_info.imageView = m_swap_image_views[m_image_index];
    render_attachment_info.imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;
    render_attachment_info.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    render_attachment_info.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    render_attachment_info.clearValue = {{{0.2f, 0.2f, 0.2f, 1.0f}}};

    VkRenderingInfoKHR render_info{};
    render_info.sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR;
    render_info.renderArea = {{}, m_swap_extent};
    render_info.layerCount = 1;
    render_info.colorAttachmentCount = 1;
    render_info.pColorAttachments = &render_attachment_info;

    cmd.begin_rendering(render_info);

    return cmd;
}

//void API::draw(MeshID mesh_id, ShaderID shader_id, const PushConstantObject& push_constants)
//{
//    const VkCommandBuffer command_buffer = m_command_buffer[m_current_frame];
//
//    ShaderPipeline& pipeline = m_shader_pool[shader_id];
//
//    vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.handle);
//
//    auto& mesh_context = m_mesh_list[mesh_id];
//
//    VkBuffer vertex_buffer = m_buffer_pool[mesh_context.vertex_buffer_id].handle;
//    VkBuffer index_buffer  = m_buffer_pool[mesh_context.index_buffer_id].handle;
//
//    VkDeviceSize offset[] = {0};
//
//    vkCmdBindVertexBuffers(command_buffer, 0, 1, &vertex_buffer, offset); 
//    vkCmdBindIndexBuffer(command_buffer, index_buffer, 0, VK_INDEX_TYPE_UINT32);
//
//    vkCmdPushConstants(command_buffer, pipeline.layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PushConstantObject), &push_constants);
//
//    VkDescriptorSet descriptor_set = pipeline.descriptor_set_list[m_current_frame];
//
//    vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.layout, 0, 1, &descriptor_set, 0, nullptr);
//
//    vkCmdDrawIndexed(command_buffer, mesh_context.index_count, 1, 0, 0, 0);
//}

void API::end_frame()
{
    VkFence fence = m_in_flight_fence[m_frame_index];

    VkSemaphore image_available = m_image_available_semaphore[m_frame_index];
    VkSemaphore render_finished = m_render_finished_semaphore[m_frame_index];

    auto& cmd = m_command_buffer[m_frame_index];

    Extensions::cmd_end_rendering(cmd->get_native());

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

    cmd->pipeline_barrier( VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, {}, {}, {&image_memory_barrier, 1});

    cmd->end();

    // Submit commands

    VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    VkSubmitInfo submit_info {
        .sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount   = 1,
        .pWaitSemaphores      = &image_available,
        .pWaitDstStageMask    = &wait_stage,
        .commandBufferCount   = 1,
        .pCommandBuffers      = &cmd->get_native(),
        .signalSemaphoreCount = 1,
        .pSignalSemaphores    = &render_finished,
    };

    m_queue.submit({&submit_info, 1}, fence); 

    //  Present image 

    VkPresentInfoKHR present_info {
        .sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores    = &render_finished,
        .swapchainCount     = 1,
        .pSwapchains        = &m_swap_chain,
        .pImageIndices      = &m_image_index,
        .pResults           = nullptr,
    };

    VK_ASSERT_THROW(vkQueuePresentKHR(m_queue.get_native(), &present_info), "Failed to present");

    if (++m_frame_index == FRAMES_IN_FLIGHT)
    {
        m_frame_index = 0;
    }
}

//void API::image_destroy(ImageID image_id)
//{
//    Image& image = m_image_pool[image_id];
//
//    if (!image.handle) return; //
//
//    vkDestroySampler(m_device, image.sampler, nullptr);
//    vkDestroyImageView(m_device, image.view, nullptr);
//    vmaDestroyImage(m_allocator, image.handle, image.allocation);
//
//    image = {};
//}


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

} // namespace Cr::Graphics::Vulkan
