#pragma once
#include <vka/vka.h>
struct DefaultDeviceCI : vka::DeviceCI
{
	DefaultDeviceCI(std::string appName)
	{
		applicationName     = appName;
		universalQueueCount = 1;
		computeQueueCount   = 0;

		// Instance Extensions
		enabledInstanceExtensions = {VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME};

		// Device Extensions
		enabledDeviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

	}
};

struct DefaultIOControlerCI : vka::IOControlerCI
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

struct DefaultAdvancedStateConfig : vka::AdvancedStateConfig
{
	DefaultAdvancedStateConfig()
	{
		modelPath = std::string(APP_SRC_DIR) + "/models/";
		texturePath = std::string(APP_SRC_DIR) + "/textures/";
		modelUsage = 0;
	}

};