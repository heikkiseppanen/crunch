#include "Graphics/Vulkan/Driver.hpp"

#include "Graphics/Vulkan/Extensions.hpp"

#include "Core/Window.hpp"

#include <vulkan.hpp>

#include <cstring>
#include <iostream>
#include <vector>
#include <optional>
#include <algorithm>
#include <limits>
#include <set>

namespace Cr::Graphics::Vulkan
{

Driver::Driver(const Core::Window& surface_context, bool debug)
{
    u32 version;
    VK_ASSERT_THROW(vkEnumerateInstanceVersion(&version), "Failed to get Vulkan version");

    CR_ASSERT_THROW(version >= VK_VERSION_1_3, "Not a supported vulkan version");

    u32 glfw_extension_count;
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
    instance_info.enabledLayerCount       = static_cast<u32>(validation_layers.size());
    instance_info.ppEnabledLayerNames     = validation_layers.data();
    instance_info.enabledExtensionCount   = static_cast<u32>(instance_extensions.size());
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
        debug_messenger_info.pfnUserCallback = Driver::debug_callback;
        debug_messenger_info.pUserData       = nullptr;

        VK_ASSERT_THROW(Extensions::create_debug_utils_messenger(m_instance, &debug_messenger_info, nullptr, &m_debug_messenger), "Failed to create a debug messenger");
    }
    else
    {
        m_debug_messenger = nullptr;
    }

    VK_ASSERT_THROW(glfwCreateWindowSurface(m_instance, surface_context.context, nullptr, &m_surface), "Failed to create Vulkan surface");

    // Physical device


    // Should maybe pack these and other useful information some GPU info structure
    QueueFamilyIndices indices{};
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

        std::optional<u32> graphics;
        std::optional<u32> presentation;
        
        u32 family_index = 0;
        for (const auto& property : get_physical_device_queue_properties(device))
        {
            VkBool32 supports_presentation = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, family_index, m_surface, &supports_presentation);

            if (!presentation.has_value() && supports_presentation)
            {
                presentation = family_index;
            }
            if (!graphics.has_value() && (property.queueFlags & VK_QUEUE_GRAPHICS_BIT))
            {
                graphics = family_index;
            }
            if (graphics.has_value() && presentation.has_value())
            {
                break;
            }

            ++family_index;
        }

        if (!graphics.has_value() || !presentation.has_value()) { continue; }

        indices.graphics = graphics.value();
        indices.presentation = presentation.value();

        // Confirm extension support

        auto extension_properties = get_physical_device_extension_properties(device);

        u32 extensions_found = 0;
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

        u32 format_count;
        VK_ASSERT_THROW(vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &format_count, nullptr), "Failed to get physical device surface format count");
        
        if (format_count > 0)
        {
            swap_chain_details.formats.resize(format_count);
            VK_ASSERT_THROW(vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &format_count, swap_chain_details.formats.data()), "Failed to get physical device surface formats");
        }

        u32 present_mode_count;
        VK_ASSERT_THROW(vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &present_mode_count, nullptr), "Failed to get physical device surface present_mode count");
        
        if (present_mode_count > 0)
        {
            swap_chain_details.present_modes.resize(present_mode_count);
            VK_ASSERT_THROW(vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &present_mode_count, swap_chain_details.present_modes.data()), "Failed to get physical device surface present_modes");
        }

        if (swap_chain_details.formats.empty() || swap_chain_details.present_modes.empty()) { continue; }

        m_physical_device = device;
    }
    
    CR_ASSERT_THROW(m_physical_device != nullptr, "No suitable Vulkan device found.");

    vkGetPhysicalDeviceFeatures(m_physical_device, &m_physical_device_features);
    vkGetPhysicalDeviceProperties(m_physical_device, &m_physical_device_properties);

    CR_INFO("Suitable device: %s", m_physical_device_properties.deviceName);

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
    logical_device_info.ppEnabledExtensionNames = Extensions::DEVICE_EXTENSION_LIST.data();
    logical_device_info.enabledExtensionCount   = static_cast<u32>(Extensions::DEVICE_EXTENSION_LIST.size());
    logical_device_info.pEnabledFeatures        = &m_physical_device_features;
    logical_device_info.pNext                   = &dynamic_rendering_feature;

    if (debug)
    {
        logical_device_info.ppEnabledLayerNames = validation_layers.data();
        logical_device_info.enabledLayerCount   = static_cast<u32>(validation_layers.size());
    }

    VK_ASSERT_THROW(vkCreateDevice(m_physical_device, &logical_device_info, nullptr, &m_device), "Failed to create logical device");

    (void)Extensions::bind_device_extension_functions(m_device);

    vkGetDeviceQueue(m_device, indices.graphics, 0, &m_graphics_queue);
    vkGetDeviceQueue(m_device, indices.presentation, 0, &m_presentation_queue);

    // Create Vulkan Memory Allocator


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
        glfwGetFramebufferSize(surface_context.context, &width, &height);
        swap_extent.width  = std::clamp(static_cast<u32>(width), capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
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

    VK_ASSERT_THROW(vkCreateSwapchainKHR(m_device, &swap_chain_info, nullptr, &m_swap_chain), "Failed to create a swap chain.");
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
    descriptor_pool_info.flags = 0;
    descriptor_pool_info.maxSets = 128;
    descriptor_pool_info.poolSizeCount = descriptor_pool_size.size();
    descriptor_pool_info.pPoolSizes    = descriptor_pool_size.data();

    VK_ASSERT_THROW(vkCreateDescriptorPool(m_device, &descriptor_pool_info, nullptr, &m_descriptor_pool), "Failed to create descriptor pool");

    // Create command pool

    VkCommandPoolCreateInfo command_pool_info {};
    command_pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    command_pool_info.pNext = nullptr;
    command_pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    command_pool_info.queueFamilyIndex = indices.graphics;

    VK_ASSERT_THROW(vkCreateCommandPool(m_device, &command_pool_info, nullptr, &m_command_pool), "Failed to create command pool");

    // Create command buffers and sync objects

    VkCommandBufferAllocateInfo command_buffer_info {};
    command_buffer_info.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    command_buffer_info.pNext              = nullptr;
    command_buffer_info.commandPool        = m_command_pool;
    command_buffer_info.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY; // Submit to queue, cannot be called from other buffers
    command_buffer_info.commandBufferCount = FRAMES_IN_FLIGHT;

    VK_ASSERT_THROW(vkAllocateCommandBuffers(m_device, &command_buffer_info, m_command_buffer.data()), "Failed to allocate command buffer");

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
    }

    CR_INFO("Vulkan initialized");
}

Driver::~Driver()
{
    if (m_device)
    {
        vkDeviceWaitIdle(m_device);

        // TODO Most of this is really stupid and should be moved to pools or other lifetime management methods
        
        for (ImageID id = 0; id < m_image_pool.size(); ++id)
        {
            image_destroy(id);
        }

        for (BufferID id = 0; id < m_buffer_pool.size(); ++id)
        {
            buffer_destroy(id);
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

        vkDestroyCommandPool(m_device, m_command_pool, nullptr);

        for (ShaderID i = 0; i < m_shader_pool.size(); ++i)
        {
            shader_destroy(i);
        }

        vkDestroyDescriptorPool(m_device, m_descriptor_pool, nullptr);

        for (auto image_view : m_swap_image_views)
        {
            vkDestroyImageView(m_device, image_view, nullptr);
        }

        vkDestroySwapchainKHR(m_device, m_swap_chain, nullptr);

        vmaDestroyAllocator(m_allocator);

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

ShaderID Driver::shader_create(const std::vector<u8>& vertex_spirv, const std::vector<u8>& fragment_spirv)
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
    rasterizer_info.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer_info.frontFace = VK_FRONT_FACE_CLOCKWISE;
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
    std::array descriptor_set_layout_bindings = {
        VkDescriptorSetLayoutBinding{
            .binding = 0,
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = 1,
            .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
            .pImmutableSamplers = nullptr,
        },
        VkDescriptorSetLayoutBinding{
            .binding = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .descriptorCount = 1,
            .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
            .pImmutableSamplers = nullptr,
        },
    };

    VkDescriptorSetLayoutCreateInfo ubo_layout_info{};
    ubo_layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    ubo_layout_info.pNext = nullptr;
    ubo_layout_info.flags = 0;
    ubo_layout_info.bindingCount = descriptor_set_layout_bindings.size();
    ubo_layout_info.pBindings    = descriptor_set_layout_bindings.data();

    ShaderPipeline pipeline {};

    VK_ASSERT_THROW(vkCreateDescriptorSetLayout(m_device, &ubo_layout_info, nullptr, &pipeline.descriptor_set_layout), "Failed to create DescriptorSetLayout");

    // Push constants

    VkPushConstantRange push_constant_range {};
    push_constant_range.offset = 0;
    push_constant_range.size = sizeof(PushConstantObject);
    push_constant_range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkPipelineLayoutCreateInfo pipeline_layout_info{};
    pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_info.setLayoutCount = 1;
    pipeline_layout_info.pSetLayouts = &pipeline.descriptor_set_layout;
    pipeline_layout_info.pushConstantRangeCount = 1;
    pipeline_layout_info.pPushConstantRanges = &push_constant_range;

    VK_ASSERT_THROW(vkCreatePipelineLayout(m_device, &pipeline_layout_info, nullptr, &pipeline.layout), "Failed to create pipeline layout");

    // DYNAMIC RENDERING EXT
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
    pipeline_info.layout              = pipeline.layout;
    pipeline_info.renderPass          = nullptr; // Not needed with dynamic rendering EXT
    pipeline_info.subpass             = 0;       // Same
    pipeline_info.basePipelineHandle  = nullptr;
    pipeline_info.basePipelineIndex   = -1;

    VK_ASSERT_THROW(vkCreateGraphicsPipelines(m_device, nullptr, 1, &pipeline_info, nullptr, &pipeline.handle), "Failed to create graphics pipeline");

    vkDestroyShaderModule(m_device, vert_module, nullptr);
    vkDestroyShaderModule(m_device, frag_module, nullptr);

//    return m_shader_pool.size() - 1;

    // UNIFORM BUFFERS

    // Create uniform buffers
    for (auto& buffer_id : pipeline.uniform_buffer_list)
    {
        buffer_id = buffer_create(BufferType::UNIFORM, sizeof(UniformBufferObject));
    }

  // Create descriptor sets
    std::array<VkDescriptorSetLayout, pipeline.descriptor_set_list.size()> descriptor_set_layouts;
    descriptor_set_layouts.fill(pipeline.descriptor_set_layout);

    VkDescriptorSetAllocateInfo descriptor_set_info {};
    descriptor_set_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descriptor_set_info.pNext = nullptr;
    descriptor_set_info.descriptorPool = m_descriptor_pool;
    descriptor_set_info.descriptorSetCount = pipeline.descriptor_set_list.size();
    descriptor_set_info.pSetLayouts = descriptor_set_layouts.data(); // NEEDS AS MANY LAYOUTS AS SETS!

    VK_ASSERT_THROW(vkAllocateDescriptorSets(m_device, &descriptor_set_info, pipeline.descriptor_set_list.data()), "Failed to allocate descriptor sets");

    for (std::size_t i = 0; i < pipeline.descriptor_set_list.size(); ++i)
    {
        auto& buffer = m_buffer_pool[pipeline.uniform_buffer_list[i]];

        VkDescriptorBufferInfo descriptor_buffer_info {};
        descriptor_buffer_info.buffer = buffer.handle;
        descriptor_buffer_info.offset = 0;
        descriptor_buffer_info.range = sizeof(UniformBufferObject);

        VkDescriptorImageInfo descriptor_image_info {
            .sampler     = m_image_pool.back().sampler, // TODO GROG HARDCODE
            .imageView   = m_image_pool.back().view,    // TODO GROG HARDCODE
            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        };

        std::array writes {
            VkWriteDescriptorSet {
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstSet = pipeline.descriptor_set_list[i],
                .dstBinding = 0,
                .descriptorCount = 1,
                .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .pBufferInfo = &descriptor_buffer_info,
            },
            VkWriteDescriptorSet {
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstSet = pipeline.descriptor_set_list[i],
                .dstBinding = 1,
                .descriptorCount = 1,
                .descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                .pImageInfo = &descriptor_image_info,
            },
        };

        vkUpdateDescriptorSets(m_device, writes.size(), writes.data(), 0, nullptr);
    }

    m_shader_pool.push_back(pipeline);

    return m_shader_pool.size() - 1;
}

void Driver::shader_set_uniform(ShaderID id, const UniformBufferObject& uniforms)
{
    auto uniform_buffer_id = m_shader_pool[id].uniform_buffer_list[m_current_frame];

    this->buffer_map_range(uniform_buffer_id, &uniforms, 1, 0);
}

void Driver::shader_destroy(ShaderID shader_id)
{
    auto& pipeline = m_shader_pool[shader_id];

    if (!pipeline.handle) return;

    for (auto buffer_id : pipeline.uniform_buffer_list)
    {
        buffer_destroy(buffer_id);
    }
    vkDestroyDescriptorSetLayout(m_device, pipeline.descriptor_set_layout, nullptr);
    vkDestroyPipelineLayout(m_device, pipeline.layout, nullptr);
    vkDestroyPipeline(m_device, pipeline.handle, nullptr);

    pipeline = {};
}

void Driver::begin_render()
{
    VkFence fence = m_in_flight_fence[m_current_frame];

    VkSemaphore image_available = m_image_available_semaphore[m_current_frame];

    VK_ASSERT_THROW(vkWaitForFences(m_device, 1, &fence, VK_TRUE, std::numeric_limits<u64>::max()), "Failed while waiting for fences");
    VK_ASSERT_THROW(vkResetFences(m_device, 1, &fence), "Failed to reset renderloop fence");

    VK_ASSERT_THROW(vkAcquireNextImageKHR(m_device, m_swap_chain, std::numeric_limits<u64>::max(), image_available, nullptr, &m_image_index), "Failed to acquire next image");

    VkCommandBuffer command_buffer = m_command_buffer[m_current_frame];

    VkCommandBufferBeginInfo begin_info {};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.pNext = nullptr;
    begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    begin_info.pInheritanceInfo = nullptr;

    VK_ASSERT_THROW(vkBeginCommandBuffer(command_buffer, &begin_info), "Failed to begin recording command buffer");

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

    Extensions::cmd_begin_rendering(command_buffer, &render_info);
}

void Driver::draw(MeshID mesh_id, ShaderID shader_id, const PushConstantObject& push_constants)
{
    const VkCommandBuffer command_buffer = m_command_buffer[m_current_frame];

    ShaderPipeline& pipeline = m_shader_pool[shader_id];

    vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.handle);

    auto& mesh_context = m_mesh_list[mesh_id];

    VkBuffer vertex_buffer = m_buffer_pool[mesh_context.vertex_buffer_id].handle;
    VkBuffer index_buffer  = m_buffer_pool[mesh_context.index_buffer_id].handle;

    VkDeviceSize offset[] = {0};

    vkCmdBindVertexBuffers(command_buffer, 0, 1, &vertex_buffer, offset); 
    vkCmdBindIndexBuffer(command_buffer, index_buffer, 0, VK_INDEX_TYPE_UINT32);

    vkCmdPushConstants(command_buffer, pipeline.layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PushConstantObject), &push_constants);

    VkDescriptorSet descriptor_set = pipeline.descriptor_set_list[m_current_frame];

    vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.layout, 0, 1, &descriptor_set, 0, nullptr);

    vkCmdDrawIndexed(command_buffer, mesh_context.index_count, 1, 0, 0, 0);
}

void Driver::end_render()
{
    VkFence fence = m_in_flight_fence[m_current_frame];

    VkSemaphore image_available = m_image_available_semaphore[m_current_frame];
    VkSemaphore render_finished = m_render_finished_semaphore[m_current_frame];

    VkCommandBuffer command_buffer = m_command_buffer[m_current_frame];

    Extensions::cmd_end_rendering(command_buffer);

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

    VK_ASSERT_THROW(vkEndCommandBuffer(command_buffer), "Failed to end recording command buffer");

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

    VK_ASSERT_THROW(vkQueueSubmit(m_graphics_queue, 1, &submit_info, fence), "Failed to submit draw command buffer");

    //  Present image 

    VkPresentInfoKHR present_info{};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = &render_finished;
    present_info.swapchainCount = 1;
    present_info.pSwapchains = &m_swap_chain;
    present_info.pImageIndices = &m_image_index;
    present_info.pResults = nullptr;

    VK_ASSERT_THROW(vkQueuePresentKHR(m_presentation_queue, &present_info), "Failed to present");

    if (++m_current_frame == FRAMES_IN_FLIGHT)
        m_current_frame = 0;
}

VkShaderModule Driver::create_shader_module(const std::vector<u8>& spirv)
{
    VkShaderModuleCreateInfo shader_module_info{};
    shader_module_info.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shader_module_info.codeSize = spirv.size();
    shader_module_info.pCode    = reinterpret_cast<const uint32_t*>(spirv.data());

    VkShaderModule shader_module;
    VK_ASSERT_THROW(vkCreateShaderModule(m_device, &shader_module_info, nullptr, &shader_module), "Failed to create shader module");

    return shader_module;
};

Buffer Driver::buffer_create()
{
    buffer.data = allocation_details.pMappedData;

    m_buffer_pool.emplace_back(buffer); // TODO Buffer isn't exception safe

    return static_cast<BufferID>(m_buffer_pool.size() - 1); // Overflowing u32 unlikely
}

void Driver::buffer_flush(BufferID id)
{
    auto& buffer = m_buffer_pool[id];

    VK_ASSERT_THROW(vmaFlushAllocation(m_allocator, buffer.allocation,  0, VK_WHOLE_SIZE), "Failed to flush buffer allocation");
}

void Driver::buffer_destroy(BufferID id)
{
    auto& buffer = m_buffer_pool[id];

    if (!buffer.handle) return;

    vmaInvalidateAllocation(m_allocator, buffer.allocation, 0, VK_WHOLE_SIZE);
    vmaDestroyBuffer(m_allocator, buffer.handle, buffer.allocation);

    buffer = {};
}

void Driver::image_destroy(ImageID image_id)
{
    Image& image = m_image_pool[image_id];

    if (!image.handle) return; //

    vkDestroySampler(m_device, image.sampler, nullptr);
    vkDestroyImageView(m_device, image.view, nullptr);
    vmaDestroyImage(m_allocator, image.handle, image.allocation);

    image = {};
}


VKAPI_ATTR VkBool32 VKAPI_CALL Driver::debug_callback(
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
