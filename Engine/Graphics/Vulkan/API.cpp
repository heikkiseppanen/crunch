#include "Graphics/Vulkan/API.hpp"
#include "Graphics/Vulkan/Extension.hpp"

#include <cstring>
#include <iostream>
#include <vector>
#include <optional>
#include <set>

#define VK_RESULT_ASSERT(RESULT, MSG) CR_ASSERT_THROW((RESULT) != VK_SUCCESS, MSG)

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
		VK_RESULT_ASSERT(vkEnumerateInstanceLayerProperties(&available_layer_count, nullptr), "Failed to get VkLayerProperties") 

		std::vector<VkLayerProperties> available_layers(available_layer_count);
		VK_RESULT_ASSERT(vkEnumerateInstanceLayerProperties(&available_layer_count, available_layers.data()), "Failed to get VkLayerProperties") 

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

	VK_RESULT_ASSERT(vkCreateInstance(&instance_info, nullptr, &m_instance), "Failed to create Vulkan instance.")

	// Need to do device extension stuff separately?
	VK_RESULT_ASSERT(Vk::bind_extension_functions(m_instance), "Failed to bind extensions function calls")

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
		debug_messenger_info.pfnUserCallback = debug_callback;
		debug_messenger_info.pUserData       = nullptr;

		VK_RESULT_ASSERT(Vk::CreateDebugUtilsMessengerEXT(m_instance, &debug_messenger_info, nullptr, &m_debug_messenger), "Failed to create a debug messenger")
	}

	VK_RESULT_ASSERT(glfwCreateWindowSurface(m_instance, surface_context, nullptr, &m_surface), "Failed to create Vulkan surface") 

	std::vector<const char*> device_extensions
	{
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	u32 device_count;
	VK_RESULT_ASSERT(vkEnumeratePhysicalDevices(m_instance, &device_count, nullptr), "Failed to get fetch devices");

	std::vector<VkPhysicalDevice> device_list{device_count};
	VK_RESULT_ASSERT(vkEnumeratePhysicalDevices(m_instance, &device_count, device_list.data()), "Failed to get fetch devices");

	m_physical_device = VK_NULL_HANDLE;
	for (auto& device : device_list)
	{
		VkPhysicalDeviceProperties properties;
		VkPhysicalDeviceFeatures features;

		vkGetPhysicalDeviceProperties(device, &properties);
		vkGetPhysicalDeviceFeatures(device, &features);

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

		if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
		{
			m_physical_device = device;
		}
	}
	
	CR_ASSERT_THROW(m_physical_device == VK_NULL_HANDLE, "No suitable Vulkan device found.")

	vkGetPhysicalDeviceFeatures(m_physical_device, &m_physical_device_features);
	vkGetPhysicalDeviceProperties(m_physical_device, &m_physical_device_properties);

	std::optional<u32> graphics;
	std::optional<u32> presentation;

	u32 family_count;
	vkGetPhysicalDeviceQueueFamilyProperties(m_physical_device, &family_count, nullptr);

	std::vector<VkQueueFamilyProperties> family_properties(family_count);
	vkGetPhysicalDeviceQueueFamilyProperties(m_physical_device, &family_count, family_properties.data());

	for (std::size_t i = 0; i < family_properties.size(); ++i)
	{
		VkBool32 supports_presentation = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(m_physical_device, i, m_surface, &supports_presentation);

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

	CR_ASSERT_THROW(!graphics.has_value(), "Could not find a graphics queue family")
	CR_ASSERT_THROW(!presentation.has_value(), "Could not find a presentation queue family")

	QueueFamilyIndices indices{};
	indices.graphics = graphics.value();
	indices.presentation = presentation.value();

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

	VkDeviceCreateInfo logical_device_info{};
	logical_device_info.sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	logical_device_info.pQueueCreateInfos       = queue_info_list.data();
	logical_device_info.queueCreateInfoCount    = static_cast<u32>(queue_info_list.size());
	logical_device_info.ppEnabledExtensionNames = device_extensions.data();
	logical_device_info.enabledExtensionCount   = static_cast<u32>(device_extensions.size());
	logical_device_info.pEnabledFeatures        = &m_physical_device_features;

	if (debug)
	{
		logical_device_info.ppEnabledLayerNames = validation_layers.data();
		logical_device_info.enabledLayerCount   = static_cast<u32>(validation_layers.size());
	}

	VK_RESULT_ASSERT(vkCreateDevice(m_physical_device, &logical_device_info, nullptr, &m_logical_device), "Failed to create logical device")

	vkGetDeviceQueue(m_logical_device, indices.graphics, 0, &m_graphics_queue);
	vkGetDeviceQueue(m_logical_device, indices.presentation, 0, &m_presentation_queue);

	//VK_RESULT_ASSERT(
}

API::~API()
{
	Vk::DestroyDebugUtilsMessengerEXT(m_instance, m_debug_messenger, nullptr);
	vkDestroyDevice(m_logical_device, nullptr);
	vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
	vkDestroyInstance(m_instance, nullptr);
}

} // namespace Vk
