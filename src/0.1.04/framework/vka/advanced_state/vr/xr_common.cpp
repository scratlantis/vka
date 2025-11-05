#include "xr_common.h"

#include <iostream>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>



using namespace std;
namespace vka
{

namespace xr
{
XrInstance createInstance()
{
	uint32_t instanceExtensionCount;
	XrResult res = xrEnumerateInstanceExtensionProperties(nullptr, 0u, &instanceExtensionCount, nullptr);
	if (XR_FAILED(res))
	{
		return XR_NULL_HANDLE;
	}

	XrInstance instance;

	static const char *const  applicationName  = "OpenXR Example";
	static const unsigned int majorVersion     = 0;
	static const unsigned int minorVersion     = 1;
	static const unsigned int patchVersion     = 0;
	static const char *const  extensionNames[] = {
        "XR_KHR_vulkan_enable",
        "XR_KHR_vulkan_enable2",
        "XR_EXT_debug_utils"};

	XrInstanceCreateInfo instanceCreateInfo{};
	instanceCreateInfo.type        = XR_TYPE_INSTANCE_CREATE_INFO;
	instanceCreateInfo.createFlags = 0;
	strcpy(instanceCreateInfo.applicationInfo.applicationName, applicationName);
	instanceCreateInfo.applicationInfo.applicationVersion = XR_MAKE_VERSION(majorVersion, minorVersion, patchVersion);
	strcpy(instanceCreateInfo.applicationInfo.engineName, applicationName);
	instanceCreateInfo.applicationInfo.engineVersion = XR_MAKE_VERSION(majorVersion, minorVersion, patchVersion);
	instanceCreateInfo.applicationInfo.apiVersion    = XR_MAKE_VERSION(1, 0, 34);
	instanceCreateInfo.enabledExtensionCount         = sizeof(extensionNames) / sizeof(const char *);
	instanceCreateInfo.enabledExtensionNames         = extensionNames;

	// instanceCreateInfo.applicationInfo.apiVersion         = XR_API_VERSION_1_0;
	// instanceCreateInfo.applicationInfo.applicationVersion = static_cast<uint32_t>(XR_MAKE_VERSION(1, 2, 0));

	static const char *const layerNames[] = {"XR_APILAYER_LUNARG_core_validation"};

	// instanceCreateInfo.enabledApiLayerCount = 1;
	// instanceCreateInfo.enabledApiLayerNames = layerNames;

	XrResult result = xrCreateInstance(&instanceCreateInfo, &instance);

	if (result != XR_SUCCESS)
	{
		cerr << "Failed to create OpenXR instance: " << result << endl;
		return XR_NULL_HANDLE;
	}

	return instance;
}

bool checkBlendModeSupport(XrViewConfigurationType viewType, XrEnvironmentBlendMode environmentBlendMode, XrInstance xrInstance, XrSystemId systemId)
{
	uint32_t environmentBlendModeCount;
	XrResult result = xrEnumerateEnvironmentBlendModes(xrInstance, systemId, viewType, 0u, &environmentBlendModeCount, nullptr);
	if (XR_FAILED(result))
	{
		DEBUG_BREAK;
		return false;
	}

	std::vector<XrEnvironmentBlendMode> supportedEnvironmentBlendModes(environmentBlendModeCount);
	result = xrEnumerateEnvironmentBlendModes(xrInstance, systemId, viewType, environmentBlendModeCount,
	                                          &environmentBlendModeCount, supportedEnvironmentBlendModes.data());
	if (XR_FAILED(result))
	{
		DEBUG_BREAK;
		return false;
	}

	bool modeFound = false;
	for (const XrEnvironmentBlendMode &mode : supportedEnvironmentBlendModes)
	{
		if (mode == environmentBlendMode)
		{
			modeFound = true;
			break;
		}
	}

	if (!modeFound)
	{
		DEBUG_BREAK;
		return false;
	}
	return true;
}

PFN_xrVoidFunction getXRFunction(XrInstance instance, const char *name)
{
	PFN_xrVoidFunction func;

	XrResult result = xrGetInstanceProcAddr(instance, name, &func);

	if (result != XR_SUCCESS)
	{
		cerr << "Failed to load OpenXR extension function '" << name << "': " << result << endl;
		DEBUG_BREAK
		return nullptr;
	}

	return func;
}

XrBool32 handleXRError(
    XrDebugUtilsMessageSeverityFlagsEXT         severity,
    XrDebugUtilsMessageTypeFlagsEXT             type,
    const XrDebugUtilsMessengerCallbackDataEXT *callbackData,
    void                                       *userData)
{
	cerr << "OpenXR ";

	switch (type)
	{
		case XR_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT:
			cerr << "general ";
			break;
		case XR_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT:
			cerr << "validation ";
			break;
		case XR_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT:
			cerr << "performance ";
			break;
		case XR_DEBUG_UTILS_MESSAGE_TYPE_CONFORMANCE_BIT_EXT:
			cerr << "conformance ";
			break;
	}

	switch (severity)
	{
		case XR_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
			cerr << "(verbose): ";
			break;
		case XR_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
			cerr << "(info): ";
			break;
		case XR_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
			cerr << "(warning): ";
			break;
		case XR_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
			cerr << "(error): ";
			break;
	}

	cerr << callbackData->message << endl;

	return XR_FALSE;
}

XrDebugUtilsMessengerEXT createDebugMessenger(XrInstance instance)
{
	XrDebugUtilsMessengerEXT debugMessenger;

	XrDebugUtilsMessengerCreateInfoEXT debugMessengerCreateInfo{};
	debugMessengerCreateInfo.type              = XR_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	debugMessengerCreateInfo.messageSeverities = XR_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | XR_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | XR_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	debugMessengerCreateInfo.messageTypes      = XR_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | XR_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | XR_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT | XR_DEBUG_UTILS_MESSAGE_TYPE_CONFORMANCE_BIT_EXT;
	debugMessengerCreateInfo.userCallback      = handleXRError;
	debugMessengerCreateInfo.userData          = nullptr;

	auto xrCreateDebugUtilsMessengerEXT = (PFN_xrCreateDebugUtilsMessengerEXT) getXRFunction(instance, "xrCreateDebugUtilsMessengerEXT");

	XrResult result = xrCreateDebugUtilsMessengerEXT(instance, &debugMessengerCreateInfo, &debugMessenger);

	if (result != XR_SUCCESS)
	{
		cerr << "Failed to create OpenXR debug messenger: " << result << endl;
		return XR_NULL_HANDLE;
	}

	return debugMessenger;
}

void destroyDebugMessenger(XrInstance instance, XrDebugUtilsMessengerEXT debugMessenger)
{
	auto xrDestroyDebugUtilsMessengerEXT = (PFN_xrDestroyDebugUtilsMessengerEXT) getXRFunction(instance, "xrDestroyDebugUtilsMessengerEXT");

	xrDestroyDebugUtilsMessengerEXT(debugMessenger);
}

XrSystemId getSystem(XrInstance instance)
{
	XrSystemId systemID;

	XrSystemGetInfo systemGetInfo{};
	systemGetInfo.type       = XR_TYPE_SYSTEM_GET_INFO;
	systemGetInfo.formFactor = XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY;

	XrResult result = xrGetSystem(instance, &systemGetInfo, &systemID);

	if (result != XR_SUCCESS)
	{
		cerr << "Failed to get system: " << result << endl;
		return XR_NULL_SYSTEM_ID;
	}

	return systemID;
}

XrSession createSession(
    XrInstance       instance,
    XrSystemId       systemID,
    VkInstance       vulkanInstance,
    VkPhysicalDevice physDevice,
    VkDevice         device,
    uint32_t         queueFamilyIndex)
{
	XrSession session;

	XrGraphicsBindingVulkan2KHR graphicsBinding{};
	graphicsBinding.type             = XR_TYPE_GRAPHICS_BINDING_VULKAN2_KHR;
	graphicsBinding.instance         = vulkanInstance;
	graphicsBinding.physicalDevice   = physDevice;
	graphicsBinding.device           = device;
	graphicsBinding.queueFamilyIndex = queueFamilyIndex;
	graphicsBinding.queueIndex       = 0;

	XrSessionCreateInfo sessionCreateInfo{};
	sessionCreateInfo.type        = XR_TYPE_SESSION_CREATE_INFO;
	sessionCreateInfo.next        = &graphicsBinding;
	sessionCreateInfo.createFlags = 0;
	sessionCreateInfo.systemId    = systemID;

	XrResult result = xrCreateSession(instance, &sessionCreateInfo, &session);

	if (result != XR_SUCCESS)
	{
		cerr << "Failed to create OpenXR session: " << result << endl;
		return XR_NULL_HANDLE;
	}

	return session;
}

void destroySession(XrSession session)
{
	xrDestroySession(session);
}

tuple<XrGraphicsRequirementsVulkan2KHR, vector<string>> getVulkanInstanceRequirements(XrInstance instance, XrSystemId system)
{
	auto                             xrGetVulkanGraphicsRequirements2KHR = (PFN_xrGetVulkanGraphicsRequirements2KHR) getXRFunction(instance, "xrGetVulkanGraphicsRequirements2KHR");
	XrGraphicsRequirementsVulkan2KHR graphicsRequirements2{};
	graphicsRequirements2.type = XR_TYPE_GRAPHICS_REQUIREMENTS_VULKAN2_KHR;
	XrResult result            = xrGetVulkanGraphicsRequirements2KHR(instance, system, &graphicsRequirements2);
	if (result != XR_SUCCESS)
	{
		cerr << "Failed to get Vulkan graphics requirements2: " << result << endl;
		return {graphicsRequirements2, {}};
	}

	auto     xrGetVulkanInstanceExtensionsKHR = (PFN_xrGetVulkanInstanceExtensionsKHR) getXRFunction(instance, "xrGetVulkanInstanceExtensionsKHR");
	uint32_t instanceExtensionsSize;

	result = xrGetVulkanInstanceExtensionsKHR(instance, system, 0, &instanceExtensionsSize, nullptr);

	if (result != XR_SUCCESS)
	{
		cerr << "Failed to get Vulkan instance extensions: " << result << endl;
		return {graphicsRequirements2, {}};
	}

	char *instanceExtensionsData = new char[instanceExtensionsSize];

	result = xrGetVulkanInstanceExtensionsKHR(instance, system, instanceExtensionsSize, &instanceExtensionsSize, instanceExtensionsData);

	if (result != XR_SUCCESS)
	{
		cerr << "Failed to get Vulkan instance extensions: " << result << endl;
		return {graphicsRequirements2, {}};
	}

	vector<string> instanceExtensions;

	uint32_t last = 0;
	for (uint32_t i = 0; i <= instanceExtensionsSize; i++)
	{
		if (i == instanceExtensionsSize || instanceExtensionsData[i] == ' ')
		{
			instanceExtensions.push_back(string(instanceExtensionsData + last, i - last));
			last = i + 1;
		}
	}

	delete[] instanceExtensionsData;

	return {graphicsRequirements2, instanceExtensions};
}

tuple<VkPhysicalDevice, vector<string>> getVulkanDeviceRequirements(XrInstance instance, XrSystemId system, VkInstance vulkanInstance)
{
	auto xrGetVulkanGraphicsDeviceKHR   = (PFN_xrGetVulkanGraphicsDeviceKHR) getXRFunction(instance, "xrGetVulkanGraphicsDeviceKHR");
	auto xrGetVulkanDeviceExtensionsKHR = (PFN_xrGetVulkanDeviceExtensionsKHR) getXRFunction(instance, "xrGetVulkanDeviceExtensionsKHR");

	VkPhysicalDevice physicalDevice;

	XrResult result = xrGetVulkanGraphicsDeviceKHR(instance, system, vulkanInstance, &physicalDevice);

	if (result != XR_SUCCESS)
	{
		cerr << "Failed to get Vulkan graphics device: " << result << endl;
		return {VK_NULL_HANDLE, {}};
	}

	uint32_t deviceExtensionsSize;

	result = xrGetVulkanDeviceExtensionsKHR(instance, system, 0, &deviceExtensionsSize, nullptr);

	if (result != XR_SUCCESS)
	{
		cerr << "Failed to get Vulkan device extensions: " << result << endl;
		return {VK_NULL_HANDLE, {}};
	}

	char *deviceExtensionsData = new char[deviceExtensionsSize];

	result = xrGetVulkanDeviceExtensionsKHR(instance, system, deviceExtensionsSize, &deviceExtensionsSize, deviceExtensionsData);

	if (result != XR_SUCCESS)
	{
		cerr << "Failed to get Vulkan device extensions: " << result << endl;
		return {VK_NULL_HANDLE, {}};
	}

	vector<string> deviceExtensions;

	uint32_t last = 0;
	for (uint32_t i = 0; i <= deviceExtensionsSize; i++)
	{
		if (i == deviceExtensionsSize || deviceExtensionsData[i] == ' ')
		{
			deviceExtensions.push_back(string(deviceExtensionsData + last, i - last));
			last = i + 1;
		}
	}

	delete[] deviceExtensionsData;

	return {physicalDevice, deviceExtensions};
}


XrPath stringToPath(XrInstance instance, const std::string &string)
{
	XrPath         path;
	const XrResult result = xrStringToPath(instance, string.c_str(), &path);
	if (XR_FAILED(result))
	{
		return XR_NULL_PATH;
	}

	return path;
}

bool createAction(XrActionSet                actionSet,
                        const std::vector<XrPath> &paths,
                        const std::string         &actionName,
                        const std::string         &localizedActionName,
                        XrActionType               type,
                        XrAction                  &action)
{
	XrActionCreateInfo actionCreateInfo{XR_TYPE_ACTION_CREATE_INFO};
	actionCreateInfo.actionType          = type;
	actionCreateInfo.countSubactionPaths = static_cast<uint32_t>(paths.size());
	actionCreateInfo.subactionPaths      = paths.data();

	memcpy(actionCreateInfo.actionName, actionName.data(), actionName.length() + 1u);
	memcpy(actionCreateInfo.localizedActionName, localizedActionName.data(), localizedActionName.length() + 1u);

	XrResult result = xrCreateAction(actionSet, &actionCreateInfo, &action);
	if (XR_FAILED(result))
	{
		return false;
	}

	return true;
}

XrPosef makeIdentity()
{
	XrPosef identity;
	identity.position    = {0.0f, 0.0f, 0.0f};
	identity.orientation = {0.0f, 0.0f, 0.0f, 1.0f};
	return identity;
}

glm::mat4 poseToMatrix(const XrPosef &pose)
{
	const glm::mat4 translation =
	    glm::translate(glm::mat4(1.0f), glm::vec3(pose.position.x, pose.position.y, pose.position.z));

	const glm::mat4 rotation =
	    glm::toMat4(glm::quat(pose.orientation.w, pose.orientation.x, pose.orientation.y, pose.orientation.z));

	return translation * rotation;
}

glm::mat4 createProjectionMatrix(XrFovf fov, float nearClip, float farClip)
{
	const float l = glm::tan(fov.angleLeft);
	const float r = glm::tan(fov.angleRight);
	const float d = glm::tan(fov.angleDown);
	const float u = glm::tan(fov.angleUp);

	const float w = r - l;
	const float h = d - u;

	glm::mat4 projectionMatrix;
	projectionMatrix[0] = {2.0f / w, 0.0f, 0.0f, 0.0f};
	projectionMatrix[1] = {0.0f, 2.0f / h, 0.0f, 0.0f};
	projectionMatrix[2] = {(r + l) / w, (u + d) / h, -(farClip + nearClip) / (farClip - nearClip), -1.0f};
	projectionMatrix[3] = {0.0f, 0.0f, -(farClip * (nearClip + nearClip)) / (farClip - nearClip), 0.0f};
	return projectionMatrix;
}

bool updateActionStatePose(XrSession session, XrAction action, XrPath path, XrActionStatePose &state)
{
	XrActionStateGetInfo actionStateGetInfo{XR_TYPE_ACTION_STATE_GET_INFO};
	actionStateGetInfo.action        = action;
	actionStateGetInfo.subactionPath = path;

	const XrResult result = xrGetActionStatePose(session, &actionStateGetInfo, &state);
	if (XR_FAILED(result))
	{
		return false;
	}

	return true;
}

bool updateActionStateFloat(XrSession session, XrAction action, XrPath path, XrActionStateFloat &state)
{
	XrActionStateGetInfo actionStateGetInfo{XR_TYPE_ACTION_STATE_GET_INFO};
	actionStateGetInfo.action        = action;
	actionStateGetInfo.subactionPath = path;

	const XrResult result = xrGetActionStateFloat(session, &actionStateGetInfo, &state);
	if (XR_FAILED(result))
	{
		return false;
	}

	return true;
}

bool updateActionStateBoolean(XrSession session, XrAction action, XrPath path, XrActionStateBoolean &state)
{
	XrActionStateGetInfo actionStateGetInfo{XR_TYPE_ACTION_STATE_GET_INFO};
	actionStateGetInfo.action        = action;
	actionStateGetInfo.subactionPath = path;

	const XrResult result = xrGetActionStateBoolean(session, &actionStateGetInfo, &state);
	if (XR_FAILED(result))
	{
		return false;
	}

	return true;
}

}        // namespace xr
}        // namespace vka