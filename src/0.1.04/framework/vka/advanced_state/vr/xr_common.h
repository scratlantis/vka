#pragma once
#include <vulkan/vulkan.h>
#define XR_USE_GRAPHICS_API_VULKAN
#include <openxr/openxr.h>
#include <openxr/openxr_platform.h>
#include <vka/core/core_common.h>
#include <string>
#include <vector>


#define VKXR_CHECK(func)                    \
    {                                       \
        VkResult result = VK_SUCCESS;       \
		VKA_ASSERT(XR_SUCCEEDED(func));     \
        VK_CHECK(result);                   \
    }

namespace vka
{

namespace xr
{

constexpr XrViewConfigurationType viewType             = XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;
constexpr XrEnvironmentBlendMode  environmentBlendMode = XR_ENVIRONMENT_BLEND_MODE_OPAQUE;
constexpr VkFormat                colorFormat          = VK_FORMAT_R8G8B8A8_SRGB;

XrInstance         createInstance();
PFN_xrVoidFunction getXRFunction(XrInstance instance, const char *name);

XrBool32 handleXRError(
    XrDebugUtilsMessageSeverityFlagsEXT         severity,
    XrDebugUtilsMessageTypeFlagsEXT             type,
    const XrDebugUtilsMessengerCallbackDataEXT *callbackData,
    void                                       *userData);

XrDebugUtilsMessengerEXT createDebugMessenger(XrInstance instance);

bool checkBlendModeSupport(XrViewConfigurationType viewType, XrEnvironmentBlendMode environmentBlendMode, XrInstance xrInstance, XrSystemId systemId);


void destroyDebugMessenger(XrInstance instance, XrDebugUtilsMessengerEXT debugMessenger);

XrSystemId getSystem(XrInstance instance);

XrSession createSession(XrInstance instance, XrSystemId systemID, VkInstance vulkanInstance, VkPhysicalDevice physDevice, VkDevice device, uint32_t queueFamilyIndex);

void destroySession(XrSession session);

std::tuple<XrGraphicsRequirementsVulkan2KHR, std::vector<std::string>> getVulkanInstanceRequirements(XrInstance instance, XrSystemId system);
std::tuple<VkPhysicalDevice, std::vector<std::string>> getVulkanDeviceRequirements(XrInstance instance, XrSystemId system, VkInstance vulkanInstance);

XrPath stringToPath(XrInstance instance, const std::string &string);

bool createAction(XrActionSet actionSet, const std::vector<XrPath> &paths, const std::string &actionName, const std::string &localizedActionName, XrActionType type, XrAction &action);

XrPosef makeIdentity();

glm::mat4 poseToMatrix(const XrPosef &pose);

glm::mat4 createProjectionMatrix(XrFovf fov, float nearClip, float farClip);

bool updateActionStatePose(XrSession session, XrAction action, XrPath path, XrActionStatePose &state);

bool updateActionStateFloat(XrSession session, XrAction action, XrPath path, XrActionStateFloat &state);

bool updateActionStateBoolean(XrSession session, XrAction action, XrPath path, XrActionStateBoolean &state);

}        // namespace xr
}