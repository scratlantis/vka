#pragma once
#include <vulkan/vulkan.h>
#include <vector>
namespace vka
{
struct QueueFamilyIndices
{
	int graphicsFamily     = -1;
	int presentationFamily = -1;
	int computeFamily      = -1;

	int graphicsFamilyCount     = 0;
	int presentationFamilyCount = 0;
	int computeFamilyCount      = 0;

	bool isValid()
	{
		return graphicsFamily >= 0 && presentationFamily >= 0 && computeFamily >= 0;
	}
};

struct SwapChainDetails
{
	VkSurfaceCapabilitiesKHR        surfaceCapabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR>   presentationMode;
};

SwapChainDetails   getSwapchainDetails(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
QueueFamilyIndices getQueueFamilies(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
void               selectQueues(int universalQueueCount, int computeQueueCount, int &universalQueueFamily, int &computeQueueFamily, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
bool               checkInstanceExtensionSupport(std::vector<const char *> *checkExtensions);
bool               checkDeviceExtensionSupport(VkPhysicalDevice physicalDevice, const std::vector<const char *> deviceExtensions);
int                checkDeviceSuitable(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, const std::vector<const char *> deviceExtensions);
VkFormat           chooseSupportedFormat(const std::vector<VkFormat> &formats, VkImageTiling tiling, VkFormatFeatureFlags featureFlags, VkPhysicalDevice physicalDevice);
}        // namespace vka