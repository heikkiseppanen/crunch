#include <vector>
#include <iostream>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <optional>
#include <cstring>

using u8  = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

using i8  = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;

using f32 = float;
using f64 = double;

#define CR_TERM_BLACK   "\x1B[31m"
#define CR_TERM_RED     "\x1B[31m"
#define CR_TERM_GREEN   "\x1B[32m"
#define CR_TERM_YELLOW  "\x1B[33m"
#define CR_TERM_BLUE    "\x1B[34m"
#define CR_TERM_MAGENTA "\x1B[35m"
#define CR_TERM_CYAN    "\x1B[36m"
#define CR_TERM_WHITE   "\x1B[37m"
#define CR_TERM_DEFAULT "\x1B[39m"

#define CR_TERM_RESET   "\x1B[0m"

#define CR_ASSERT_THROW(COND, MSG) do {                           \
	if (COND) {                                                    \
		throw std::runtime_error(CR_TERM_RED MSG CR_TERM_RESET); \
	}                                                              \
} while(0);

#define CR_LOG(FD, COLOR, MSG) do {                  \
	std::fprintf(FD, COLOR "%s" CR_TERM_RESET "\n", MSG);  \
} while(0);

#define CR_INFO(MSG)  CR_LOG(stdout, CR_TERM_DEFAULT, MSG)
#define CR_SUCCESS(MSG)  CR_LOG(stdout, CR_TERM_GREEN, MSG)
#define CR_WARN(MSG)  CR_LOG(stderr, CR_TERM_YELLOW, MSG)
#define CR_ERROR(MSG) CR_LOG(stderr, CR_TERM_RED, MSG)

GLFWwindow* window;

VkDebugUtilsMessengerEXT debug_messenger;

VkInstance       instance  {VK_NULL_HANDLE};

VkSurfaceKHR     surf      {VK_NULL_HANDLE};

VkPhysicalDevice pdev      {VK_NULL_HANDLE};

VkPhysicalDeviceProperties pdev_properties {};
VkPhysicalDeviceFeatures   pdev_features {};

VkDevice         ldev      {VK_NULL_HANDLE};

VkQueue          qgfx      {VK_NULL_HANDLE};
VkQueue          qpres     {VK_NULL_HANDLE};

u32 graphics_queue_family;
u32 presentation_queue_family;

static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
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

int main(int argc, char **argv)
{
	(void)argc;
	(void)argv;

	// GLFW INIT
	
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	window = glfwCreateWindow(512, 512, "Vulkan", nullptr, nullptr);

	u32 glfwExtensionCount;
	const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
	
	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
	extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

	// Vulkan

	VkApplicationInfo app_info{
		.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO,
		.pApplicationName   = "Vulkan",
		.applicationVersion = VK_MAKE_VERSION(1, 0, 0),
		.pEngineName        = "No Engine",
		.engineVersion      = VK_MAKE_VERSION(1, 0, 0),
		.apiVersion         = VK_API_VERSION_1_0,
	};


	// Validation layers

	const std::vector<const char*> validation_layers { "VK_LAYER_KHRONOS_validation" };

	u32 validation_layer_count;
	vkEnumerateInstanceLayerProperties(&validation_layer_count, nullptr);

	std::vector<VkLayerProperties> available_validation_layers(validation_layer_count);
	vkEnumerateInstanceLayerProperties(&validation_layer_count, available_validation_layers.data());

	for (auto layer : validation_layers) {
		for (auto available : available_validation_layers) {
			if (strcmp(layer, available.layerName) == 0) {
				std::cout << "Layer " << layer << " found\n";
			}
		}
	}

	// Instance

	VkInstanceCreateInfo instance_info
	{
		.sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		.pApplicationInfo        = &app_info,
		.enabledLayerCount       = static_cast<u32>(validation_layers.size()),
		.ppEnabledLayerNames     = validation_layers.data(),
		.enabledExtensionCount   = static_cast<u32>(extensions.size()),
		.ppEnabledExtensionNames = extensions.data(),
	};

	if (vkCreateInstance(&instance_info, nullptr, &instance) != VK_SUCCESS) {
		CR_ERROR("Failed to create Vulkan instance");
	} else {
		CR_SUCCESS("Created Vulkan instance");
	}

	// Create surface (THANKS GLFW)

	if (glfwCreateWindowSurface(instance, glfwGetCurrentContext(), nullptr, &surf) != VK_SUCCESS) {
		CR_ERROR("Failed to create Vulkan surface\n");
	} else {
		CR_SUCCESS("Created Vulkan surface\n");
	}

	// Debug Callback

	VkDebugUtilsMessengerCreateInfoEXT db_create_info {
		.sType           =  VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
		.pNext           =  nullptr,
		.flags           =  0,
		.messageSeverity =  VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
						    VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
						    VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT, 
		.messageType     =  VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT    |
						    VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
						    VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
		.pfnUserCallback = debug_callback,
		.pUserData       = nullptr,
	};

	auto create_debug_messenger = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

	if (create_debug_messenger == nullptr) {
		std::cerr << "Extension not present\n";
	} else if (create_debug_messenger(instance, &db_create_info, nullptr, &debug_messenger) != VK_SUCCESS) {
		std::cerr << "Failed to set up debug messenger!\n";
	}

	// Select physical device

	std::cout << "Finding devices...\n";

	u32 dev_count {0};
	vkEnumeratePhysicalDevices(instance, &dev_count, nullptr);

	std::vector<VkPhysicalDevice> dev_list(dev_count);
	vkEnumeratePhysicalDevices(instance, &dev_count, dev_list.data());

	std::cout << "Choosing device...\n";

	for (const auto& d : dev_list) {
		// Device properties

		vkGetPhysicalDeviceProperties(d, &pdev_properties); vkGetPhysicalDeviceFeatures(d, &pdev_features);

		// Queue families

		std::optional<u32> gfx {};
		std::optional<u32> present {};

		u32 queue_family_count {};
		vkGetPhysicalDeviceQueueFamilyProperties(d, &queue_family_count, nullptr);

		std::vector<VkQueueFamilyProperties> queue_family_list(queue_family_count);
		vkGetPhysicalDeviceQueueFamilyProperties(d, &queue_family_count, queue_family_list.data());


		i32 i = 0;
		for (const auto& qf : queue_family_list) {
			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(d, i, surf, &presentSupport);

			if (presentSupport) { present = i; }

			if (qf.queueFlags & VK_QUEUE_GRAPHICS_BIT) { gfx = i; }

			if (gfx.has_value() && present.has_value()) break;

			++i;
		}

		if (gfx.has_value()
			&& present.has_value()
			&& pdev_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU
			&& pdev_features.geometryShader
		){
			// THESE CAN END UP BEING THE SAME
			presentation_queue_family = present.value();
			graphics_queue_family = gfx.value();
			pdev = d;
			break;
		}
	}

	if (pdev == VK_NULL_HANDLE) {
		std::cerr << "No vulkan compliant devices!\n";
		return 1;
	} else {
		std::cout << "Vulkan device found\n";
	}

	// Logical Device

	std::array<u32, 2> queue_family_indices { graphics_queue_family, presentation_queue_family };
	std::vector<VkDeviceQueueCreateInfo> queue_info_list;

	f32 queue_priority = 1.0f;

	for (const auto qf : queue_family_indices)
	{
		VkDeviceQueueCreateInfo queue_info {
			.sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
			.queueFamilyIndex = qf,
			.queueCount       = 1,
			.pQueuePriorities = &queue_priority,
		};
		queue_info_list.push_back(queue_info);
	}

	VkDeviceCreateInfo ldev_info {
		.sType                = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		.queueCreateInfoCount = static_cast<u32>(queue_info_list.size()),
		.pQueueCreateInfos    = queue_info_list.data(),
		.pEnabledFeatures     = &pdev_features,
	};

	if (vkCreateDevice(pdev, &ldev_info, nullptr, &ldev) != VK_SUCCESS) {
		CR_ERROR("Failed to create logical device\n");
		return 1;
	} else {
		CR_SUCCESS("Created logical device.\n");
	}

	vkGetDeviceQueue(ldev, graphics_queue_family, 0, &qgfx);
	vkGetDeviceQueue(ldev, presentation_queue_family, 0, &qpres);



	// Main loop

    while(!glfwWindowShouldClose(window))
    {
    	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
			glfwSetWindowShouldClose(window, GLFW_TRUE);
		}

        glfwPollEvents();
    }

	// Vulkan terminate
	vkDestroyDevice(ldev, nullptr);

	auto vkDestroyDebugMessenger = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	vkDestroyDebugMessenger(instance, debug_messenger, nullptr);

	vkDestroySurfaceKHR(instance, surf, nullptr); 
	vkDestroyInstance(instance, nullptr);

	// GLFW terminate
    glfwDestroyWindow(window);
    glfwTerminate();

	return 0;
}
