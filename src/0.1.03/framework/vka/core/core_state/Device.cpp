#include "Device.h"
#include <vka/globals.h>

namespace vka
{

Device::Device()
{
}
void Device::configure(DeviceCI &deviceCI)
{
	this->deviceCI = deviceCI;
}

void Device::destroy()
{
	vkDestroyDevice(logical, nullptr);
	vkDestroyInstance(instance, nullptr);
}

void Device::createInstance()
{
	VkApplicationInfo appInfo{VK_STRUCTURE_TYPE_APPLICATION_INFO};
	appInfo.pApplicationName = deviceCI.applicationName.c_str();
	appInfo.pEngineName      = deviceCI.applicationName.c_str();
	appInfo.apiVersion       = API_VERSION;

	std::vector<const char *> instanceExtensions;
	if (deviceCI.enabledInstanceExtensions.size() > 0)
	{
		for (const char *enabledExtension : deviceCI.enabledInstanceExtensions)
		{
			if (std::find(deviceCI.enabledInstanceExtensions.begin(), deviceCI.enabledInstanceExtensions.end(), enabledExtension) == deviceCI.enabledInstanceExtensions.end())
			{
				std::cerr << "Enabled instance extension \"" << enabledExtension << "\" is not present at instance level\n";
			}
			instanceExtensions.push_back(enabledExtension);
		}
	}

	VkInstanceCreateInfo instanceCreateInfo{VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
	instanceCreateInfo.pApplicationInfo = &appInfo;

	if (!instanceExtensions.empty())
	{
		//		IF_VALIDATION(
		instanceExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
		instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);        //)
		instanceCreateInfo.enabledExtensionCount   = static_cast<uint32_t>(instanceExtensions.size());
		instanceCreateInfo.ppEnabledExtensionNames = instanceExtensions.data();
	}

	const char *validationLayerName        = "VK_LAYER_KHRONOS_validation";
	instanceCreateInfo.ppEnabledLayerNames = &validationLayerName;
	instanceCreateInfo.enabledLayerCount   = 1;
	VK_CHECK(vkCreateInstance(&instanceCreateInfo, nullptr, &instance));
	// IF_VALIDATION(
	uint32_t instanceLayerCount;
	vkEnumerateInstanceLayerProperties(&instanceLayerCount, nullptr);
	std::vector<VkLayerProperties> instanceLayerProperties(instanceLayerCount);
	vkEnumerateInstanceLayerProperties(&instanceLayerCount, instanceLayerProperties.data());
	bool validationLayerPresent = false;
	for (VkLayerProperties layer : instanceLayerProperties)
	{
		if (strcmp(layer.layerName, validationLayerName) == 0)
		{
			validationLayerPresent = true;
			break;
		}
	}
	if (validationLayerPresent)
	{
		instanceCreateInfo.ppEnabledLayerNames = &validationLayerName;
		instanceCreateInfo.enabledLayerCount   = 1;
	}
	else
	{
		std::cerr << "Validation layer VK_LAYER_KHRONOS_validation not present, validation is disabled";
	}
	gState.initBits |= STATE_INIT_DEVICE_INSTANCE_BIT;
}
void Device::selectPhysicalDevice()
{
	VKA_CHECK(gState.initBits & (STATE_INIT_DEVICE_INSTANCE_BIT | STATE_INIT_IO_WINDOW_BIT));
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
	gState.initBits |= STATE_INIT_DEVICE_PHYSICAL_BIT;
}
void Device::createLogicalDevice()
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
	logicalDeviceCI.pNext                   = deviceCI.enabledFeatures.chainNodes();

	VK_CHECK(vkCreateDevice(physical, &logicalDeviceCI, nullptr, &logical));

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