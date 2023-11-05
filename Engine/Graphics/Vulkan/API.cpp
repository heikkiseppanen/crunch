#include "Graphics/Vulkan/API.hpp"

#include "Graphics/Vulkan/Extension.hpp"

#include "Shared/Filesystem.hpp"

#include <cstring>
#include <iostream>
#include <vector>
#include <optional>
#include <algorithm>
#include <limits>
#include <set>

#define VK_ASSERT_RESULT(RESULT, MSG) CR_ASSERT_THROW((RESULT) < 0, MSG)

namespace Vk
{

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

API::API(GLFWwindow* surface_context, bool debug)
{
	u32 glfw_extension_count;
	const char** glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);

	std::vector<const char*> instance_extensions(glfw_extensions, glfw_extensions + glfw_extension_count);
	std::vector<const char*> validation_layers;

	if (debug == true)
	{
		instance_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		
		validation_layers.push_back("VK_LAYER_KHRONOS_validation");

		u32 available_layer_count;
		VK_ASSERT_RESULT(vkEnumerateInstanceLayerProperties(&available_layer_count, nullptr), "Failed to get VkLayerProperties") 

		std::vector<VkLayerProperties> available_layers(available_layer_count);
		VK_ASSERT_RESULT(vkEnumerateInstanceLayerProperties(&available_layer_count, available_layers.data()), "Failed to get VkLayerProperties") 

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

	VK_ASSERT_RESULT(vkCreateInstance(&instance_info, nullptr, &m_instance), "Failed to create Vulkan instance.")

	// Need to do device extension stuff separately?
	VK_ASSERT_RESULT(Vk::bind_instance_extension_functions(m_instance), "Failed to bind extensions function calls")

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

		VK_ASSERT_RESULT(Vk::CreateDebugUtilsMessengerEXT(m_instance, &debug_messenger_info, nullptr, &m_debug_messenger), "Failed to create a debug messenger")
	}

	VK_ASSERT_RESULT(glfwCreateWindowSurface(m_instance, surface_context, nullptr, &m_surface), "Failed to create Vulkan surface") 

	// Physical device


	// TODO should be figured out more dynamically later on
	std::vector<const char*> device_extensions
	{
		VK_KHR_SWAPCHAIN_EXTENSION_NAME,
		VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
		VK_KHR_MAINTENANCE1_EXTENSION_NAME
	};

	// Should maybe pack these and other useful information some GPU info structure
	QueueFamilyIndices indices{};
	SwapChainSupportDetails swap_chain_details{};

	u32 device_count;
	VK_ASSERT_RESULT(vkEnumeratePhysicalDevices(m_instance, &device_count, nullptr), "Failed to get fetch devices");

	std::vector<VkPhysicalDevice> device_list{device_count};
	VK_ASSERT_RESULT(vkEnumeratePhysicalDevices(m_instance, &device_count, device_list.data()), "Failed to get fetch devices");

	m_physical_device = VK_NULL_HANDLE;
	for (auto& device : device_list)
	{
		// Confirm properties and features

		VkPhysicalDeviceProperties properties;
		VkPhysicalDeviceFeatures features;

		vkGetPhysicalDeviceProperties(device, &properties);
		vkGetPhysicalDeviceFeatures(device, &features);

		if (properties.deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU ||
		    properties.apiVersion <  VK_VERSION_1_3)
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
		VK_ASSERT_RESULT(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_surface, &swap_chain_details.capabilities), "Failed to fetch physical device surface capabilities")

		u32 format_count;
		VK_ASSERT_RESULT(vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &format_count, nullptr), "Failed to get physical device surface format count")
		
		if (format_count > 0)
		{
			swap_chain_details.formats.resize(format_count);
			VK_ASSERT_RESULT(vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &format_count, swap_chain_details.formats.data()), "Failed to get physical device surface formats")
		}

		u32 present_mode_count;
		VK_ASSERT_RESULT(vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &present_mode_count, nullptr), "Failed to get physical device surface present_mode count")
		
		if (present_mode_count > 0)
		{
			swap_chain_details.present_modes.resize(present_mode_count);
			VK_ASSERT_RESULT(vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &present_mode_count, swap_chain_details.present_modes.data()), "Failed to get physical device surface present_modes")
		}

		if (swap_chain_details.formats.empty() || swap_chain_details.present_modes.empty())
			continue;

		m_physical_device = device;
	}
	
	CR_ASSERT_THROW(m_physical_device == VK_NULL_HANDLE, "No suitable Vulkan device found.")

	bind_device_extension_functions(m_instance);

	vkGetPhysicalDeviceFeatures(m_physical_device, &m_physical_device_features);
	vkGetPhysicalDeviceProperties(m_physical_device, &m_physical_device_properties);

	std::cout << VK_VERSION_MAJOR(m_physical_device_properties.apiVersion) << '.';
	std::cout << VK_VERSION_MINOR(m_physical_device_properties.apiVersion) << '.';
	std::cout << VK_VERSION_PATCH(m_physical_device_properties.apiVersion) << '\n';

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

	constexpr VkPhysicalDeviceDynamicRenderingFeaturesKHR dynamic_rendering_feature
	{
		.sType            = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES_KHR,
		.pNext            = VK_NULL_HANDLE,
		.dynamicRendering = VK_TRUE
	};

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

	VK_ASSERT_RESULT(vkCreateDevice(m_physical_device, &logical_device_info, nullptr, &m_logical_device), "Failed to create logical device")

	vkGetDeviceQueue(m_logical_device, indices.graphics, 0, &m_graphics_queue);
	vkGetDeviceQueue(m_logical_device, indices.presentation, 0, &m_presentation_queue);

	// Initialize Vulkan Memory Allocator

	VmaAllocatorCreateInfo allocator_info
	{
		.flags = 0,
		.physicalDevice = m_physical_device,
		.device = m_logical_device,
		.preferredLargeHeapBlockSize = 0,
		.pAllocationCallbacks = nullptr,
		.pDeviceMemoryCallbacks = nullptr,
		.pHeapSizeLimit = nullptr,
		.pVulkanFunctions = nullptr,
		.instance = m_instance,
		.vulkanApiVersion = 0,
		.pTypeExternalMemoryHandleTypes = nullptr,
	};

	VK_ASSERT_RESULT(vmaCreateAllocator(&allocator_info, &m_allocator), "Failed to initialize VulkanMemoryAllocator")

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
		image_count = std::min(swap_chain_details.capabilities.minImageCount + 1, swap_chain_details.capabilities.maxImageCount);
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
	swap_chain_info.oldSwapchain     = VK_NULL_HANDLE; // Swapchain MUST be recreated and old referenced if resize etc happens!

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

	VK_ASSERT_RESULT(vkCreateSwapchainKHR(m_logical_device, &swap_chain_info, nullptr, &m_swap_chain), "Failed to create a swap chain.")
	m_swap_format = surface_format.format;
	m_swap_extent = swap_extent;

	// Get the imaegs of the swap chain 

	u32 swap_image_count;
	VK_ASSERT_RESULT(vkGetSwapchainImagesKHR(m_logical_device, m_swap_chain, &swap_image_count, nullptr), "Failed to fetch swap chain image count");
	m_swap_images.resize(swap_image_count);
	VK_ASSERT_RESULT(vkGetSwapchainImagesKHR(m_logical_device, m_swap_chain, &swap_image_count, m_swap_images.data()), "Failed to fetch swap chain images");

	m_swap_image_views.resize(m_swap_images.size());

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
		
		VK_ASSERT_RESULT(vkCreateImageView(m_logical_device, &image_info, nullptr, &m_swap_image_views[i]), "Failed to create swap chain image view")
	}

	{
		VkShaderModule frag_module = create_module("Assets/Shaders/triangle.frag.spv");
		VkShaderModule vert_module = create_module("Assets/Shaders/triangle.vert.spv");
	
		VkPipelineShaderStageCreateInfo shader_stage_info[2]{};

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

		// TEMP MESH

		VkVertexInputBindingDescription bind_descriptor
		{
			.binding = 0,
			.stride = sizeof(Cr::Vertex),
			.inputRate =VK_VERTEX_INPUT_RATE_VERTEX,
		};


		VkVertexInputAttributeDescription attribute_descriptor[]
		{
			{
				.location = 0,
				.binding = 0,
				.format = VK_FORMAT_R32G32B32_SFLOAT,
				.offset = offsetof(Cr::Vertex, position)
			},
			{
				.location = 1,
				.binding = 0,
				.format = VK_FORMAT_R32G32_SFLOAT,
				.offset = offsetof(Cr::Vertex, uv)
			},
		};

		VkPipelineVertexInputStateCreateInfo vertex_input_info
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
			.pNext = VK_NULL_HANDLE,
			.flags = 0,
			.vertexBindingDescriptionCount   = 1,
			.pVertexBindingDescriptions      = &bind_descriptor,
			.vertexAttributeDescriptionCount = 2,
			.pVertexAttributeDescriptions    = attribute_descriptor,
		};

		VkPipelineInputAssemblyStateCreateInfo input_assembly_info{};
		input_assembly_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		input_assembly_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		input_assembly_info.primitiveRestartEnable = VK_FALSE;

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
		rasterizer_info.cullMode = VK_CULL_MODE_NONE;
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
		//color_blend_info.logicOp = VK_LOGIC_OP_COPY;
		color_blend_info.attachmentCount = 1;
		color_blend_info.pAttachments = &color_blend_attachment;
		//color_blend_info.blendConstants[0] = 0.0f;

		VkPipelineLayoutCreateInfo pipeline_layout_info{};
		pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		//pipeline_layout_info.setLayoutCount = 0;
		//pipeline_layout_info.pSetLayouts = nullptr;
		//pipeline_layout_info.pushConstantRangeCount = 0;
		//pipeline_layout_info.pPushConstantRanges = nullptr;

		VK_ASSERT_RESULT(vkCreatePipelineLayout(m_logical_device, &pipeline_layout_info, nullptr, &m_pipeline_layout), "Failed to create pipeline layout")

		VkPipelineRenderingCreateInfoKHR pipeline_rendering_info{};
		pipeline_rendering_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR;
		pipeline_rendering_info.colorAttachmentCount = 1;
		pipeline_rendering_info.pColorAttachmentFormats = &m_swap_format;


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
		pipeline_info.pDepthStencilState  = VK_NULL_HANDLE;
		pipeline_info.pColorBlendState    = &color_blend_info;
		pipeline_info.pDynamicState       = &dynamic_state_info;
		pipeline_info.layout              = m_pipeline_layout;
		pipeline_info.renderPass          = VK_NULL_HANDLE;
		pipeline_info.subpass             = 0;
		pipeline_info.basePipelineHandle  = VK_NULL_HANDLE;
		pipeline_info.basePipelineIndex   = -1;

		VK_ASSERT_RESULT(vkCreateGraphicsPipelines(m_logical_device, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &m_graphics_pipeline), "Failed to create graphics pipeline")

		vkDestroyShaderModule(m_logical_device, vert_module, nullptr);
		vkDestroyShaderModule(m_logical_device, frag_module, nullptr);
	}

	VkCommandPoolCreateInfo command_pool_info
	{
		.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		.pNext = VK_NULL_HANDLE,
		.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
		.queueFamilyIndex = indices.graphics,
	};

	VK_ASSERT_RESULT(vkCreateCommandPool(m_logical_device, &command_pool_info, nullptr, &m_command_pool), "Failed to create command pool")

	// Create cube mesh

	{
		std::vector<Cr::Vertex> cube_vertices = Cr::get_cube_vertices(1.0f);
		std::vector<u32> cube_indices = Cr::get_cube_indices();

		VkBufferCreateInfo vertex_buffer_info
		{
			.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
			.pNext = VK_NULL_HANDLE,
			.flags = 0,
			.size  = sizeof(cube_vertices[0]) * cube_vertices.size(),
			.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
			.queueFamilyIndexCount = 0,
			.pQueueFamilyIndices = VK_NULL_HANDLE,
		};

		VkBufferCreateInfo index_buffer_info
		{
			.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
			.pNext = VK_NULL_HANDLE,
			.flags = 0,
			.size  = sizeof(cube_indices[0]) * cube_indices.size(),
			.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
			.queueFamilyIndexCount = 0,
			.pQueueFamilyIndices = VK_NULL_HANDLE,
		};

		VmaAllocationCreateInfo alloc_info
		{
			.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
			.usage = VMA_MEMORY_USAGE_AUTO,
			.requiredFlags = 0,
			.preferredFlags = 0,
			.memoryTypeBits = 0,
			.pool = nullptr,
			.pUserData = nullptr,
			.priority = 0
		};

		VK_ASSERT_RESULT(vmaCreateBuffer(m_allocator, &vertex_buffer_info, &alloc_info, &m_vertex_buffer, &m_vertex_allocation, nullptr), "Failed to create vertex buffer")
		VK_ASSERT_RESULT(vmaCreateBuffer(m_allocator, &index_buffer_info, &alloc_info, &m_index_buffer, &m_index_allocation, nullptr), "Failed to create index buffer")

		void *data;

		VK_ASSERT_RESULT(vmaMapMemory(m_allocator, m_vertex_allocation, &data), "Failed to map vertex_buffer");
		std::copy(cube_vertices.cbegin(), cube_vertices.cend(), static_cast<Cr::Vertex*>(data));
		vmaUnmapMemory(m_allocator, m_vertex_allocation);

		VK_ASSERT_RESULT(vmaMapMemory(m_allocator, m_index_allocation, &data), "Failed to map index_buffer");
		std::copy(cube_indices.cbegin(), cube_indices.cend(), static_cast<u32*>(data));
		vmaUnmapMemory(m_allocator, m_index_allocation);

		VK_ASSERT_RESULT(vmaFlushAllocation(m_allocator, m_vertex_allocation, 0, VK_WHOLE_SIZE), "Failed to flush vertex allocation")
		VK_ASSERT_RESULT(vmaFlushAllocation(m_allocator, m_index_allocation, 0, VK_WHOLE_SIZE), "Failed to flush index allocation")
	}

	// Create command buffers and sync objects

	m_command_buffer.resize(m_frames_in_flight);
	m_image_available_semaphore.resize(m_frames_in_flight);
	m_render_finished_semaphore.resize(m_frames_in_flight);
	m_in_flight_fence.resize(m_frames_in_flight);

	VkCommandBufferAllocateInfo command_buffer_info
	{
		.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.pNext              = VK_NULL_HANDLE,
		.commandPool        = m_command_pool,
		.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY, // Submit to queue, cannot be called from other buffers
		.commandBufferCount = static_cast<u32>(m_command_buffer.size()),
	};

	VK_ASSERT_RESULT(vkAllocateCommandBuffers(m_logical_device, &command_buffer_info, m_command_buffer.data()), "Failed to allocate command buffer") 

	VkSemaphoreCreateInfo semaphore_info
	{
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
		.pNext = VK_NULL_HANDLE,
		.flags = 0,
	};

	VkFenceCreateInfo fence_info
	{
		.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
		.pNext = VK_NULL_HANDLE,
		.flags = VK_FENCE_CREATE_SIGNALED_BIT,
	};

	for (std::size_t frame = 0; frame < m_frames_in_flight; ++frame)
	{
		VK_ASSERT_RESULT((vkCreateSemaphore(m_logical_device, &semaphore_info, nullptr, &m_image_available_semaphore[frame]) |
						  vkCreateSemaphore(m_logical_device, &semaphore_info, nullptr, &m_render_finished_semaphore[frame]) |
						  vkCreateFence(m_logical_device, &fence_info, nullptr, &m_in_flight_fence[frame])), "Failed to create semaphores")
	}

	CR_INFO("Vulkan initialized")
}

API::~API()
{
	vkDeviceWaitIdle(m_logical_device);

	vmaDestroyBuffer(m_allocator, m_index_buffer, m_index_allocation); 
	vmaDestroyBuffer(m_allocator, m_vertex_buffer, m_vertex_allocation); 

	for (auto fence : m_in_flight_fence)
	{
		vkDestroyFence(m_logical_device, fence, nullptr);
	}

	for (auto semaphore : m_image_available_semaphore)
	{
		vkDestroySemaphore(m_logical_device, semaphore, nullptr);
	}

	for (auto semaphore : m_render_finished_semaphore)
	{
		vkDestroySemaphore(m_logical_device, semaphore, nullptr);
	}

	vkDestroyCommandPool(m_logical_device, m_command_pool, nullptr);

	vkDestroyPipeline(m_logical_device, m_graphics_pipeline, nullptr);
	vkDestroyPipelineLayout(m_logical_device, m_pipeline_layout, nullptr);

	for (auto image_view : m_swap_image_views)
	{
		vkDestroyImageView(m_logical_device, image_view, nullptr);
	}

	vkDestroySwapchainKHR(m_logical_device, m_swap_chain, nullptr);

	vmaDestroyAllocator(m_allocator);

	vkDestroyDevice(m_logical_device, nullptr);

	vkDestroySurfaceKHR(m_instance, m_surface, nullptr);

	Vk::DestroyDebugUtilsMessengerEXT(m_instance, m_debug_messenger, nullptr);

	vkDestroyInstance(m_instance, nullptr);
}

void API::proto_render_loop()
{
	VK_ASSERT_RESULT(vkWaitForFences(m_logical_device, 1, &m_in_flight_fence[m_current_frame], VK_TRUE, std::numeric_limits<u64>::max()), "Failed while waiting for fences")
	VK_ASSERT_RESULT(vkResetFences(m_logical_device, 1, &m_in_flight_fence[m_current_frame]), "Failed to reset renderloop fence")

	u32 image_index;
	VK_ASSERT_RESULT(vkAcquireNextImageKHR(m_logical_device, m_swap_chain, std::numeric_limits<u64>::max(), m_image_available_semaphore[m_current_frame], VK_NULL_HANDLE, &image_index), "Failed to acquire next image")

	VkCommandBufferBeginInfo begin_info
	{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.pNext = VK_NULL_HANDLE,
		.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
		.pInheritanceInfo = VK_NULL_HANDLE
	};

//	VK_ASSERT_RESULT(vkResetCommandBuffer(m_command_buffer, 0), "Failed to reset command buffer")
	VK_ASSERT_RESULT(vkBeginCommandBuffer(m_command_buffer[m_current_frame], &begin_info), "Failed to begin recording command buffer")

	VkRenderingAttachmentInfoKHR render_attachment_info{};
	render_attachment_info.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
	render_attachment_info.imageView = m_swap_image_views[image_index];
	render_attachment_info.imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;
	render_attachment_info.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	render_attachment_info.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	render_attachment_info.clearValue = {{{0.0f, 0.0f, 0.0f, 1.0f}}};

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
			.pNext               = VK_NULL_HANDLE,
			.srcAccessMask       = VK_ACCESS_NONE,
			.dstAccessMask       = VK_ACCESS_NONE,
			.oldLayout           = VK_IMAGE_LAYOUT_UNDEFINED,
			.newLayout           = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			.srcQueueFamilyIndex = 0,
			.dstQueueFamilyIndex = 0,
			.image               = m_swap_images[image_index],
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
			m_command_buffer[m_current_frame],
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			0, 0, nullptr, 0, nullptr, 1, &image_memory_barrier);
	}

	vkCmdBindPipeline(m_command_buffer[m_current_frame], VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphics_pipeline);

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = float(m_swap_extent.height); // Offset flipped viewport
	viewport.width = float(m_swap_extent.width);
	viewport.height = -float(m_swap_extent.height); // Flip viewport
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	vkCmdSetViewport(m_command_buffer[m_current_frame], 0, 1, &viewport);

	VkRect2D scissor{};
	scissor.offset = {};
	scissor.extent = m_swap_extent;

	vkCmdSetScissor(m_command_buffer[m_current_frame], 0, 1, &scissor);

	vkCmdBindPipeline(m_command_buffer[m_current_frame], VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphics_pipeline);

	VkDeviceSize offsets[] {0};
	vkCmdBindVertexBuffers(m_command_buffer[m_current_frame], 0, 1, &m_vertex_buffer, offsets); 

	Vk::CmdBeginRenderingKHR(m_command_buffer[m_current_frame], &render_info);

	vkCmdDraw(m_command_buffer[m_current_frame], 16, 1, 0, 0);

	Vk::CmdEndRenderingKHR(m_command_buffer[m_current_frame]);

	{
		VkImageMemoryBarrier image_memory_barrier
		{
			.sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
			.pNext               = VK_NULL_HANDLE,
			.srcAccessMask       = VK_ACCESS_NONE,
			.dstAccessMask       = VK_ACCESS_NONE,
			.oldLayout           = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			.newLayout           = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
			.srcQueueFamilyIndex = 0,
			.dstQueueFamilyIndex = 0,
			.image               = m_swap_images[image_index],
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
			m_command_buffer[m_current_frame],
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
			0, 0, nullptr, 0, nullptr, 1, &image_memory_barrier);
	}

	VK_ASSERT_RESULT(vkEndCommandBuffer(m_command_buffer[m_current_frame]), "Failed to end recording command buffer")

	VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

	VkSubmitInfo submit_info{};
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_info.waitSemaphoreCount = 1;
	submit_info.pWaitSemaphores = &m_image_available_semaphore[m_current_frame];
	submit_info.pWaitDstStageMask = &wait_stage;
	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = &m_command_buffer[m_current_frame];
	submit_info.signalSemaphoreCount = 1;
	submit_info.pSignalSemaphores = &m_render_finished_semaphore[m_current_frame];

	VK_ASSERT_RESULT(vkQueueSubmit(m_graphics_queue, 1, &submit_info, m_in_flight_fence[m_current_frame]), "Failed to submit draw command buffer")

	VkPresentInfoKHR present_info{};
	present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	present_info.waitSemaphoreCount =1;
	present_info.pWaitSemaphores = &m_render_finished_semaphore[m_current_frame];
	present_info.swapchainCount = 1;
	present_info.pSwapchains = &m_swap_chain;
	present_info.pImageIndices = &image_index;
	present_info.pResults = nullptr;

	VK_ASSERT_RESULT(vkQueuePresentKHR(m_presentation_queue, &present_info), "Failed to present")

	if (++m_current_frame == m_frames_in_flight)
		m_current_frame = 0;
}

VkShaderModule API::create_module(const std::string& path)
{
	std::vector<u8> source = Cr::read_binary_file(path);

	CR_ASSERT_THROW(source.empty(), "Failed to load shader file")

	VkShaderModuleCreateInfo shader_module_info{};
	shader_module_info.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	shader_module_info.codeSize = source.size();
	shader_module_info.pCode    = reinterpret_cast<const uint32_t*>(source.data());

	VkShaderModule shader_module;
	VK_ASSERT_RESULT(vkCreateShaderModule(m_logical_device, &shader_module_info, nullptr, &shader_module), "Failed to create shader module")

	return shader_module;
};

} // namespace Vk
