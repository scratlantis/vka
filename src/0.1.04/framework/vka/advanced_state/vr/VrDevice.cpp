#include "VrDevice.h"
#include <vka/globals.h>
#include "xr_common.h"

namespace vka
{

void VrDevice::configure(DeviceCI &deviceCI)
{
	this->deviceCI = deviceCI;
}

void VrDevice::destroy()
{
	vkDestroyDevice(logical, nullptr);
	LOAD_CMD_VK_INSTANCE(vkDestroyDebugUtilsMessengerEXT, instance)
	pvkDestroyDebugUtilsMessengerEXT(instance, printfDebugMessenger, nullptr);
	pvkDestroyDebugUtilsMessengerEXT(instance, validationDebugMessenger, nullptr);
	vkDestroyInstance(instance, nullptr);
}

VKAPI_ATTR VkBool32 VKAPI_CALL printf_debug_utils_message_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT             messageType,
    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
    void                                       *pUserData)
{
	std::string message = std::string(pCallbackData->pMessage);
	message             = message.substr(message.find_last_of("|") + 2);
	std::cout << message.c_str();
	return VK_FALSE;
}

VKAPI_ATTR VkBool32 VKAPI_CALL validation_debug_utils_message_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT             messageType,
    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
    void                                       *pUserData)
{
	std::string message = std::string(pCallbackData->pMessage);
	message             = message.substr(message.find_last_of("|") + 1);
	uint32_t endPos     = message.find_last_of("(");
	// message             = message.substr(0, endPos);
	std::cout << "[ Validation Error ]\n"
	          << message.c_str() << "\n";
	return VK_FALSE;
}

void VrDevice::createInstance()
{
	std::vector<const char *> instanceExtensions;
	VkApplicationInfo appInfo{VK_STRUCTURE_TYPE_APPLICATION_INFO};
	appInfo.pApplicationName = deviceCI.applicationName.c_str();
	appInfo.pEngineName      = deviceCI.applicationName.c_str();
	appInfo.apiVersion       = API_VERSION;

	if (deviceCI.enabledInstanceExtensions.size() > 0)
	{
		for (const char *enabledExtension : deviceCI.enabledInstanceExtensions)
		{
			instanceExtensions.push_back(enabledExtension);
		}
	}


	std::vector<VkExtensionProperties> supportedVulkanInstanceExtensions;
	{
		uint32_t instanceExtensionCount;
		if (vkEnumerateInstanceExtensionProperties(nullptr, &instanceExtensionCount, nullptr) != VK_SUCCESS)
		{
			DEBUG_BREAK;
		}

		supportedVulkanInstanceExtensions.resize(instanceExtensionCount);
		if (vkEnumerateInstanceExtensionProperties(nullptr, &instanceExtensionCount,
		                                           supportedVulkanInstanceExtensions.data()) != VK_SUCCESS)
		{
			DEBUG_BREAK;
		}
	}

	// Check that all required Vulkan instance extensions are supported
	for (const char *extension : instanceExtensions)
	{
		bool extensionSupported = false;

		for (const VkExtensionProperties &supportedExtension : supportedVulkanInstanceExtensions)
		{
			if (strcmp(extension, supportedExtension.extensionName) == 0)
			{
				extensionSupported = true;
				break;
			}
		}

		if (!extensionSupported)
		{
			std::cerr << "Missing Vulkan instance extension \"" << extension << "\"";
			DEBUG_BREAK;
			return;
		}
	}




	VkInstanceCreateInfo instanceCreateInfo{VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
	instanceCreateInfo.pApplicationInfo = &appInfo;

	if (!instanceExtensions.empty())
	{
		instanceExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
		instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		instanceCreateInfo.enabledExtensionCount   = static_cast<uint32_t>(instanceExtensions.size());
		instanceCreateInfo.ppEnabledExtensionNames = instanceExtensions.data();
	}

	const char *validationLayerName        = "VK_LAYER_KHRONOS_validation";
	instanceCreateInfo.ppEnabledLayerNames = &validationLayerName;
	instanceCreateInfo.enabledLayerCount   = 1;

	VkValidationFeaturesEXT validationFeatures{VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT};
	validationFeatures.enabledValidationFeatureCount = deviceCI.enabledValidationFeatures.size();
	validationFeatures.pEnabledValidationFeatures    = deviceCI.enabledValidationFeatures.data();
	validationFeatures.pNext                         = deviceCI.enabledInstanceFeatures.chainNodes();
	instanceCreateInfo.pNext                         = &validationFeatures;

	if (gState.isVrEnabled())
	{
		XrVulkanInstanceCreateInfoKHR createInfo{XR_TYPE_VULKAN_INSTANCE_CREATE_INFO_KHR};
		createInfo.pfnGetInstanceProcAddr = &vkGetInstanceProcAddr;
		createInfo.systemId               = gState.xrContext.xrSystemId;
		createInfo.vulkanCreateInfo       = &instanceCreateInfo;
		VkResult vkResult;
		if (gState.xrContext.xrCreateVulkanInstanceKHR(gState.xrContext.xrInstance, &createInfo, &instance, &vkResult))
		{
			DEBUG_BREAK;
		}
		VK_CHECK(vkResult);
	}
	else
	{
		VK_CHECK(vkCreateInstance(&instanceCreateInfo, nullptr, &instance));
	}

	LOAD_CMD_VK_INSTANCE(vkCreateDebugUtilsMessengerEXT, instance)

	VkDebugUtilsMessengerCreateInfoEXT printfDebugUtilsMessengerCI{VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT};
	printfDebugUtilsMessengerCI.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;
	printfDebugUtilsMessengerCI.messageType     = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
	printfDebugUtilsMessengerCI.pfnUserCallback = printf_debug_utils_message_callback;
	VK_CHECK(pvkCreateDebugUtilsMessengerEXT(instance, &printfDebugUtilsMessengerCI, nullptr, &printfDebugMessenger));

	VkDebugUtilsMessengerCreateInfoEXT validationDebugUtilsMessengerCI{VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT};
	validationDebugUtilsMessengerCI.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	validationDebugUtilsMessengerCI.messageType     = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
	validationDebugUtilsMessengerCI.pfnUserCallback = validation_debug_utils_message_callback;
	VK_CHECK(pvkCreateDebugUtilsMessengerEXT(instance, &validationDebugUtilsMessengerCI, nullptr, &printfDebugMessenger));

	gState.initBits |= STATE_INIT_DEVICE_INSTANCE_BIT;
}


void VrDevice::selectPhysicalDevice()
{
	VKA_CHECK(gState.initBits & (STATE_INIT_DEVICE_INSTANCE_BIT | STATE_INIT_IO_WINDOW_BIT));

	if (gState.isVrEnabled())
	{
		XrVulkanGraphicsDeviceGetInfoKHR vulkanGraphicsDeviceGetInfo{XR_TYPE_VULKAN_GRAPHICS_DEVICE_GET_INFO_KHR};
		vulkanGraphicsDeviceGetInfo.systemId       = gState.xrContext.xrSystemId;
		vulkanGraphicsDeviceGetInfo.vulkanInstance = instance;
		XrResult result = gState.xrContext.xrGetVulkanGraphicsDevice2KHR(
		    gState.xrContext.xrInstance,
		    &vulkanGraphicsDeviceGetInfo,
		    &physical);
		VKA_ASSERT(XR_SUCCEEDED(result));
	}
	else
	{
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
		if (deviceCount == 0)
		{
			throw std::runtime_error("Can not find GPU's that support Vulkan Instance!");
		}

		std::vector<VkPhysicalDevice> deviceList(deviceCount);
		vkEnumeratePhysicalDevices(instance, &deviceCount, deviceList.data());

		int maxDevicePriority = 0;
		for (const auto &device : deviceList)
		{
			int devicePriority = checkDeviceSuitable(device, gState.io.surface, deviceCI.enabledDeviceExtensions);
			if (devicePriority > maxDevicePriority)
			{
				physical          = device;
				maxDevicePriority = devicePriority;
			}
		}
		if (maxDevicePriority == 0)
		{
			throw std::runtime_error("Can not find suitable GPU!");
		}
	}
	gState.initBits |= STATE_INIT_DEVICE_PHYSICAL_BIT;
}
void VrDevice::createLogicalDevice()
{
	std::vector<VkDeviceQueueCreateInfo> queueCIs;
	VkDeviceQueueCreateInfo              queueCI{VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO};

	std::vector<float> piority(std::max(deviceCI.universalQueueCount, deviceCI.computeQueueCount));
	for (size_t i = 0; i < piority.size(); i++)
	{
		piority[i] = 1.0f;
	}
	queueCI.pQueuePriorities = piority.data();
	int universalFamily, computeFamily;
	selectQueues(deviceCI.universalQueueCount, deviceCI.computeQueueCount, universalFamily, computeFamily, physical, gState.io.surface);

	if (deviceCI.universalQueueCount > 0)
	{
		queueCI.queueFamilyIndex = universalFamily;
		queueCI.queueCount       = deviceCI.universalQueueCount;
		queueCIs.push_back(queueCI);
	}
	if (deviceCI.computeQueueCount > 0)
	{
		queueCI.queueFamilyIndex = computeFamily;
		queueCI.queueCount       = deviceCI.computeQueueCount;
		queueCIs.push_back(queueCI);
	}

	VkDeviceCreateInfo logicalDeviceCI{VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
	logicalDeviceCI.queueCreateInfoCount    = VKA_COUNT(queueCIs);
	logicalDeviceCI.pQueueCreateInfos       = queueCIs.data();
	logicalDeviceCI.enabledExtensionCount   = VKA_COUNT(deviceCI.enabledDeviceExtensions);
	logicalDeviceCI.ppEnabledExtensionNames = deviceCI.enabledDeviceExtensions.data();
	logicalDeviceCI.pNext                   = deviceCI.enabledDeviceFeatures.chainNodes();

	if (gState.isVrEnabled())
	{
		XrVulkanDeviceCreateInfoKHR createInfo{XR_TYPE_VULKAN_DEVICE_CREATE_INFO_KHR};
		createInfo.pfnGetInstanceProcAddr = &vkGetInstanceProcAddr;
		createInfo.systemId               = gState.xrContext.xrSystemId;
		createInfo.vulkanCreateInfo       = &logicalDeviceCI;
		createInfo.vulkanPhysicalDevice   = physical;
		VkResult vkResult;
		VKA_ASSERT(XR_SUCCEEDED(gState.xrContext.xrCreateVulkanDeviceKHR(gState.xrContext.xrInstance, &createInfo, &logical, &vkResult)));
		VK_CHECK(vkResult);
		XrGraphicsRequirementsVulkan2KHR graphicsRequirements{XR_TYPE_GRAPHICS_REQUIREMENTS_VULKAN2_KHR};
		VKA_ASSERT(XR_SUCCEEDED(gState.xrContext.xrGetVulkanGraphicsRequirements2KHR(
		    gState.xrContext.xrInstance,
		    gState.xrContext.xrSystemId,
		    &graphicsRequirements)));
	}
	else
	{
		VK_CHECK(vkCreateDevice(physical, &logicalDeviceCI, nullptr, &logical));
	}

	universalQueues.resize(deviceCI.universalQueueCount);
	for (size_t i = 0; i < deviceCI.universalQueueCount; i++)
	{
		vkGetDeviceQueue(logical, universalFamily, i, &universalQueues[i]);
	}
	computeQueues.resize(deviceCI.computeQueueCount);
	if (computeFamily != universalFamily)
	{
		for (size_t i = 0; i < deviceCI.computeQueueCount; i++)
		{
			vkGetDeviceQueue(logical, computeFamily, i, &computeQueues[i]);
		}
	}
	else
	{
		for (size_t i = deviceCI.universalQueueCount; i < deviceCI.universalQueueCount + deviceCI.computeQueueCount; i++)
		{
			vkGetDeviceQueue(logical, computeFamily, i, &computeQueues[i]);
		}
	}
	gState.initBits |= STATE_INIT_DEVICE_LOGICAL_BIT;
	gState.initBits |= STATE_INIT_DEVICE_BIT;

}

}        // namespace vka