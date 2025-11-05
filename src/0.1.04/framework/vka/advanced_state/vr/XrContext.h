#pragma once
#include <vector>
#include <vka/core/core_state/Device.h>
#define XR_USE_GRAPHICS_API_VULKAN
#include <openxr/openxr.h>
#include <openxr/openxr_platform.h>
#include <string>

namespace vka
{

class XrContext
{
  public:
	// xr
	XrInstance               xrInstance = nullptr;
	XrDebugUtilsMessengerEXT xrDebugMessenger = nullptr;
	XrSystemId               xrSystemId       = 0U;

	// Extension function pointers
	PFN_xrCreateVulkanInstanceKHR           xrCreateVulkanInstanceKHR           = nullptr;
	PFN_xrCreateVulkanDeviceKHR             xrCreateVulkanDeviceKHR             = nullptr;
	PFN_xrGetVulkanGraphicsDevice2KHR       xrGetVulkanGraphicsDevice2KHR       = nullptr;
	PFN_xrGetVulkanGraphicsRequirements2KHR xrGetVulkanGraphicsRequirements2KHR = nullptr;

	XrGraphicsRequirementsVulkanKHR xrGraphicsRequirements;
	std::vector<std::string>           xrInstanceExtensions;

	VkPhysicalDevice         xrPhysicalDevice;
	std::vector<std::string> xrDeviceRequirements;





	XrContext() = default;
	~XrContext() = default;

	void initInstance(std::vector<const char *> &extensions);
	void destroy();

	DELETE_COPY_CONSTRUCTORS(XrContext);

  protected:
};
}        // namespace vka