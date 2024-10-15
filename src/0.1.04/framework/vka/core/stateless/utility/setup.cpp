#pragma once
#include "setup.h"
#include "macros.h"

namespace vka
{

QueueFamilyIndices getQueueFamilies(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
{
	QueueFamilyIndices indices{};
	uint32_t           queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
	std::vector<VkQueueFamilyProperties> queueFamilyList(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilyList.data());
	int index = 0;
	for (const auto &queueFamily : queueFamilyList)
	{
		if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			indices.graphicsFamily = index;
			indices.graphicsFamilyCount += 1;
		}
		if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT)
		{
			indices.computeFamily = index;
			indices.computeFamilyCount += 1;
		}
		VkBool32 presentation_support = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, index, surface, &presentation_support);
		if (queueFamily.queueCount > 0 && presentation_support)
		{
			indices.presentationFamily = index;
			indices.presentationFamilyCount += 1;
		}
		if (indices.isValid())
		{
			break;
		}
		index++;
	}
	return indices;
}

void selectQueues(int universalQueueCount, int computeQueueCount, int &universalQueueFamily, int &computeQueueFamily, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
{
	universalQueueFamily = -1;
	computeQueueFamily   = -1;
	QueueFamilyIndices indices{};
	uint32_t           queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
	std::vector<VkQueueFamilyProperties> queueFamilyList(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilyList.data());
	int index = 0;
	for (auto &queueFamily : queueFamilyList)
	{
		if (universalQueueFamily = -1 &&
		                           queueFamily.queueCount >= universalQueueCount && queueFamily.queueFlags & (VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT))
		{
			VkBool32 presentation_support = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, index, surface, &presentation_support);
			if (presentation_support)
			{
				universalQueueFamily = index;
				queueFamily.queueCount -= universalQueueCount;
			}
		}
		if (computeQueueFamily = -1 &&
		                         queueFamily.queueCount >= computeQueueCount && queueFamily.queueFlags & (VK_QUEUE_COMPUTE_BIT))
		{
			computeQueueFamily = index;
			queueFamily.queueCount -= computeQueueCount;
		}
		if ((universalQueueFamily != -1 || universalQueueCount == 0) && (computeQueueFamily != -1 || computeQueueCount == 0))
		{
			break;
		}
	}
}

SwapChainDetails getSwapchainDetails(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
{
	SwapChainDetails swapchainDetails{};
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &swapchainDetails.surfaceCapabilities);
	uint32_t formatCount = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);
	swapchainDetails.formats.resize(formatCount);
	vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, swapchainDetails.formats.data());
	uint32_t presentationCount = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentationCount, nullptr);
	if (presentationCount > 0)
	{
		swapchainDetails.presentationMode.resize(presentationCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentationCount, swapchainDetails.presentationMode.data());
	}
	return swapchainDetails;
}

bool checkInstanceExtensionSupport(std::vector<const char *> *checkExtensions)
{
	uint32_t extensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
	std::vector<VkExtensionProperties> extensions(extensionCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());
	for (const auto &checkExtension : *checkExtensions)
	{
		bool hasExtension = false;
		for (const auto &extension : extensions)
		{
			if (strcmp(checkExtension, extension.extensionName))
			{
				hasExtension = true;
				break;
			}
		}
		if (!hasExtension)
		{
			return false;
		}
	}
	return true;
}

bool checkDeviceExtensionSupport(VkPhysicalDevice physicalDevice, const std::vector<const char *> deviceExtensions)
{
	uint32_t extensionCount = 0;
	vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);
	if (extensionCount == 0)
	{
		return false;
	}
	std::vector<VkExtensionProperties> extensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, extensions.data());

	for (const auto &deviceExtension : deviceExtensions)
	{
		bool hasExtension = false;
		for (const auto &extension : extensions)
		{
			if (strcmp(deviceExtension, extension.extensionName) == 0)
			{
				hasExtension = true;
				break;
			}
		}
		if (!hasExtension)
		{
			return false;
		}
	}
	return true;
}

int checkDeviceSuitable(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, const std::vector<const char *> deviceExtensions)
{
	VkPhysicalDeviceProperties deviceProperties;
	vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);
	VkPhysicalDeviceFeatures deviceFeatures;
	vkGetPhysicalDeviceFeatures(physicalDevice, &deviceFeatures);
	QueueFamilyIndices indices             = getQueueFamilies(physicalDevice, surface);
	bool               extensionsSupported = checkDeviceExtensionSupport(physicalDevice, deviceExtensions);
	bool               swapChainValid      = false;
	if (extensionsSupported)
	{
		SwapChainDetails swapChainDetails = getSwapchainDetails(physicalDevice, surface);
		swapChainValid                    = !swapChainDetails.presentationMode.empty() && !swapChainDetails.formats.empty();
	}
	if (!(indices.isValid() && extensionsSupported && swapChainValid && deviceFeatures.samplerAnisotropy))
	{
		return 0;
	}
	else if (deviceProperties.vendorID == INTEL_VENDOR_ID)
	{
		return 1;
	}
	else if (deviceProperties.vendorID == AMD_VENDOR_ID)
	{
		return 2;
	}
	else if (deviceProperties.vendorID == NVIDIA_VENDOR_ID)
	{
		return 3;
	}
	return 1;
}

VkFormat chooseSupportedFormat(const std::vector<VkFormat> &formats, VkImageTiling tiling, VkFormatFeatureFlags featureFlags, VkPhysicalDevice physicalDevice)
{
	for (VkFormat format : formats)
	{
		VkFormatProperties properties;
		vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &properties);
		if (tiling == VK_IMAGE_TILING_LINEAR && (properties.linearTilingFeatures & featureFlags) == featureFlags)
		{
			return format;
		}
		else if (tiling == VK_IMAGE_TILING_OPTIMAL && (properties.optimalTilingFeatures & featureFlags) == featureFlags)
		{
			return format;
		}
	}
	throw std::runtime_error("Failed to find supported format!");
}

}        // namespace vka