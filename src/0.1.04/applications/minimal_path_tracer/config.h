#pragma once
#include <glm/gtc/matrix_transform.hpp>
#include <vka/vka.h>
static const std::string shaderPath = std::string(APP_SRC_DIR) + "/shaders/";

using namespace vka;
using namespace vka::pbr;


//#define USE_SHADER_PRINT_F
#define RAY_TRACING_SUPPORT
//#define USE_ATOMICS
#define USE_VULKAN_1_3

struct DefaultDeviceCI : DeviceCI
{
	DefaultDeviceCI(std::string appName)
	{
		applicationName     = appName;
		universalQueueCount = 1;
		computeQueueCount   = 0;

		// Mandatory Instance Extensions
		enabledInstanceExtensions = {VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME};
		// Mandatory Device Extensions
		enabledDeviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
		enabledDeviceExtensions.push_back(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
		enabledDeviceExtensions.push_back(VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME);

#ifdef USE_VULKAN_1_3
		enabledDeviceExtensions.push_back(VK_KHR_MAINTENANCE3_EXTENSION_NAME);
		VkPhysicalDeviceVulkan13Features features13{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES};
		features13.maintenance4 = VK_TRUE;
		enabledDeviceFeatures.addNode(features13);
#endif
#ifdef USE_SHADER_PRINT_F
		enabledInstanceExtensions.push_back(VK_EXT_LAYER_SETTINGS_EXTENSION_NAME);
		enabledValidationFeatures.push_back(VK_VALIDATION_FEATURE_ENABLE_DEBUG_PRINTF_EXT);
#endif        // USE_SHADER_PRINT_F

		VkPhysicalDeviceVulkan12Features features12{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES};
		features12.bufferDeviceAddress             = VK_TRUE;
		features12.descriptorBindingPartiallyBound = VK_TRUE;
		enabledDeviceFeatures.addNode(features12);


#ifdef USE_ATOMICS
		enabledDeviceExtensions.push_back(VK_EXT_SHADER_ATOMIC_FLOAT_EXTENSION_NAME);
		enabledDeviceExtensions.push_back(VK_EXT_SHADER_IMAGE_ATOMIC_INT64_EXTENSION_NAME);

		VkPhysicalDeviceShaderAtomicFloatFeaturesEXT shaderAtomicFeatures{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ATOMIC_FLOAT_FEATURES_EXT};
		shaderAtomicFeatures.shaderBufferFloat32Atomics   = VK_TRUE;
		shaderAtomicFeatures.shaderBufferFloat32AtomicAdd = VK_TRUE;
		enabledDeviceFeatures.addNode(shaderAtomicFeatures);

		VkPhysicalDeviceShaderImageAtomicInt64FeaturesEXT shader64Features{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_IMAGE_ATOMIC_INT64_FEATURES_EXT};
		shader64Features.shaderImageInt64Atomics = VK_TRUE;
		enabledDeviceFeatures.addNode(shader64Features);

#endif        // USE_ATOMICS

#ifdef RAY_TRACING_SUPPORT
		enabledDeviceExtensions.push_back(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME);
		enabledDeviceExtensions.push_back(VK_KHR_PIPELINE_LIBRARY_EXTENSION_NAME);
		enabledDeviceExtensions.push_back(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
		enabledDeviceExtensions.push_back(VK_KHR_RAY_QUERY_EXTENSION_NAME);
		enabledDeviceExtensions.push_back(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);

		VkPhysicalDeviceRayQueryFeaturesKHR rayQueryFeatures{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_QUERY_FEATURES_KHR};
		rayQueryFeatures.rayQuery = VK_TRUE;
		enabledDeviceFeatures.addNode(rayQueryFeatures);

		VkPhysicalDeviceRayTracingPipelineFeaturesKHR ray_tracing_pipeline_features{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR};
		ray_tracing_pipeline_features.rayTracingPipeline = VK_TRUE;
		enabledDeviceFeatures.addNode(ray_tracing_pipeline_features);

		VkPhysicalDeviceAccelerationStructureFeaturesKHR acceleration_structure_features{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR};
		acceleration_structure_features.accelerationStructure                                 = VK_TRUE;
		acceleration_structure_features.accelerationStructureIndirectBuild                    = VK_FALSE;
		acceleration_structure_features.accelerationStructureHostCommands                     = VK_FALSE;
		acceleration_structure_features.descriptorBindingAccelerationStructureUpdateAfterBind = VK_FALSE;
		enabledDeviceFeatures.addNode(acceleration_structure_features);
#endif        // RAY_TRACING_SUPPORT
	}
};

struct DefaultIOControlerCI : IOControlerCI
{
	DefaultIOControlerCI(std::string title, uint32_t width, uint32_t height)
	{
		windowTitel                 = title;
		resizable                   = true;
		size                        = {width, height};
		cursorMode                  = vka::WINDOW_CURSOR_MODE_VISIBLE;
		preferedFormats             = {{VK_FORMAT_R8G8B8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR}, {VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR}};
		preferedPresentModes        = {VK_PRESENT_MODE_MAILBOX_KHR};
		preferedSwapchainImageCount = 3;
	}
};

struct DefaultAdvancedStateConfig : AdvancedStateConfig
{
	DefaultAdvancedStateConfig()
	{
		modelPath   = std::string(RESOURCE_BASE_DIR) + "/models/";
		texturePath = std::string(RESOURCE_BASE_DIR) + "/textures/";
		modelUsage  = 0;
	}
};

struct DefaultFixedCameraState : public vka::FixedCameraState
{
	DefaultFixedCameraState()
	{
		fixpoint    = glm::vec3(0.0f, 0.0f, 0.0f);
		distance    = 1.0;
		up          = glm::vec3(0.0f, 1.0f, 0.0f);
		yaw         = 90.f;
		pitch       = 0.0f;
		moveSpeed   = 0.2f;
		turnSpeed   = 0.25f;
		scrollSpeed = 0.1f;
	}
};

