#include "XrContext.h"
#include "xr_common.h"

namespace vka
{





void XrContext::initInstance(std::vector<const char *> &extensions)
{
	xrInstance = xr::createInstance();
	xrDebugMessenger = xr::createDebugMessenger(xrInstance);
	xrSystemId = xr::getSystem(xrInstance);

	// Load extension function pointers
	xrCreateVulkanInstanceKHR = (PFN_xrCreateVulkanInstanceKHR)xr::getXRFunction(xrInstance, "xrCreateVulkanInstanceKHR");
	xrCreateVulkanDeviceKHR = (PFN_xrCreateVulkanDeviceKHR)xr::getXRFunction(xrInstance, "xrCreateVulkanDeviceKHR");
	xrGetVulkanGraphicsDevice2KHR = (PFN_xrGetVulkanGraphicsDevice2KHR)xr::getXRFunction(xrInstance, "xrGetVulkanGraphicsDevice2KHR");
	xrGetVulkanGraphicsRequirements2KHR = (PFN_xrGetVulkanGraphicsRequirements2KHR)xr::getXRFunction(xrInstance, "xrGetVulkanGraphicsRequirements2KHR");

	// Get Vulkan requirements
	std::tie(xrGraphicsRequirements, xrInstanceExtensions) = xr::getVulkanInstanceRequirements(xrInstance, xrSystemId);

	VKA_ASSERT(xr::checkBlendModeSupport(xr::viewType, xr::environmentBlendMode, xrInstance, xrSystemId));

	for (uint32_t i = 0; i < xrInstanceExtensions.size(); i++)
	{
		extensions.push_back(xrInstanceExtensions[i].c_str());
	}
}

void XrContext::destroy()
{
	if (xrDebugMessenger != nullptr)
	{
		xr::destroyDebugMessenger(xrInstance, xrDebugMessenger);
		xrDebugMessenger = nullptr;
	}
	if (xrInstance != nullptr)
	{
		xrDestroyInstance(xrInstance);
		xrInstance = nullptr;
	}
}

}        // namespace vka