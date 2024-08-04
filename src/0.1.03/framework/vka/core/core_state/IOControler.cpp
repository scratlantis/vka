#include "IOControler.h"
#include <vka/globals.h>
#include <vka/core/stateless/vk_types/default_values.h>
namespace vka
{
WindowCI IOControlerCI::getWindowCI()
{
	WindowCI windowCI{};
	windowCI.cursorMode = cursorMode;
	windowCI.title      = windowTitel;
	windowCI.height     = size.height;
	windowCI.width      = size.width;
	windowCI.resizable  = resizable;
	return windowCI;
}

IOController::IOController()
{
}

void IOController::configure(IOControlerCI &controllerCI, Window *window)
{
	VKA_CHECK(gState.initBits & STATE_INIT_DEVICE_INSTANCE_BIT);
	this->controllerCI = controllerCI;
	this->window       = window;
	window->init(controllerCI.getWindowCI(), gState.device.instance);
	surface = window->getSurface();
	gState.initBits |= STATE_INIT_IO_WINDOW_BIT;
}

void IOController::init()
{
	VKA_CHECK(gState.initBits & STATE_INIT_DEVICE_BIT);
	swapChainDetails = getSwapchainDetails(gState.device.physical, window->getSurface());
	surfaceFormat    = swapChainDetails.formats[0];        // Must exist after vulkan specifications
	selectByPreference(swapChainDetails.formats, controllerCI.preferedFormats, surfaceFormat);
	format      = surfaceFormat.format;
	presentMode = VK_PRESENT_MODE_FIFO_KHR;
	selectByPreference(swapChainDetails.presentationMode, controllerCI.preferedPresentModes, presentMode);

	imageCount = controllerCI.preferedSwapchainImageCount;
	imageCount = std::max(imageCount, swapChainDetails.surfaceCapabilities.minImageCount);
	if (swapChainDetails.surfaceCapabilities.maxImageCount > 0)        // 0 == limitless
	{
		imageCount = std::min(imageCount, swapChainDetails.surfaceCapabilities.maxImageCount);
	}
	shouldRecreateSwapchain = true;

	updateSwapchain();
	gState.initBits |= STATE_INIT_IO_BIT;
}

void IOController::updateSwapchain()
{
	if (!shouldRecreateSwapchain)
	{
		swapchainWasRecreated = false;
		return;
	}
	vkDeviceWaitIdle(gState.device.logical);

	while (window->size().height == 0 || window->size().width == 0)
	{
		window->waitEvents();
	}
	swapChainDetails = getSwapchainDetails(gState.device.physical, window->getSurface());
	VKA_CHECK(gState.initBits & STATE_INIT_DEVICE_BIT);

	extent = window->size();
	clamp(extent, swapChainDetails.surfaceCapabilities.minImageExtent, swapChainDetails.surfaceCapabilities.maxImageExtent);

	VkSwapchainCreateInfoKHR vkSwapchainCI{VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR};
	vkSwapchainCI.surface          = surface;
	vkSwapchainCI.imageFormat      = surfaceFormat.format;
	vkSwapchainCI.imageColorSpace  = surfaceFormat.colorSpace;
	vkSwapchainCI.presentMode      = presentMode;
	vkSwapchainCI.imageExtent      = extent;
	vkSwapchainCI.minImageCount    = imageCount;
	vkSwapchainCI.imageArrayLayers = 1;
	vkSwapchainCI.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	vkSwapchainCI.preTransform     = swapChainDetails.surfaceCapabilities.currentTransform;
	vkSwapchainCI.compositeAlpha   = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	vkSwapchainCI.clipped          = VK_TRUE;

	QueueFamilyIndices indices = getQueueFamilies(gState.device.physical, surface);

	if (indices.graphicsFamily != indices.presentationFamily)
	{
		uint32_t queueFamilyIndices[] = {
		    (uint32_t) indices.graphicsFamily,
		    (uint32_t) indices.presentationFamily};

		vkSwapchainCI.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
		vkSwapchainCI.queueFamilyIndexCount = 2;
		vkSwapchainCI.pQueueFamilyIndices   = queueFamilyIndices;
	}
	else
	{
		vkSwapchainCI.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	}

	VkSwapchainKHR oldSwapchain = VK_NULL_HANDLE;
	if ((gState.initBits & STATE_INIT_IO_SWAPCHAIN_BIT))
	{
		oldSwapchain = swapchain;
	}
	vkSwapchainCI.oldSwapchain = oldSwapchain;

	VK_CHECK(vkCreateSwapchainKHR(gState.device.logical, &vkSwapchainCI, nullptr, &swapchain));

	if ((gState.initBits & (STATE_INIT_IO_SWAPCHAIN_BIT)))
	{
		VKA_CHECK((gState.initBits & (STATE_INIT_FRAME_BIT)))
		for (size_t i = 0; i < imageCount; i++)
		{
			vkDestroyImageView(gState.device.logical, imageViews[i], nullptr);
		}
		vkDestroySwapchainKHR(gState.device.logical, oldSwapchain, nullptr);
	}

	images.resize(imageCount);
	imageViews.resize(imageCount);
	imageLayouts.resize(imageCount);
	for (size_t i = 0; i < imageLayouts.size(); i++)
	{
		imageLayouts[i] = VK_IMAGE_LAYOUT_UNDEFINED;
	}
	VK_CHECK(vkGetSwapchainImagesKHR(gState.device.logical, swapchain, &imageCount, images.data()));
	for (size_t i = 0; i < imageCount; i++)
	{
		ImageViewCreateInfo_Default viewCI = ImageViewCreateInfo_Default(images[i], format);
		VK_CHECK(vkCreateImageView(gState.device.logical, &viewCI, nullptr, &imageViews[i]));
	}
	gState.initBits |= STATE_INIT_IO_SWAPCHAIN_BIT;
	shouldRecreateSwapchain = false;
	swapchainWasRecreated   = true;
}
bool IOController::swapchainRecreated()
{
	return swapchainWasRecreated;
}
Window *IOController::getWindow()
{
	return window;
}
void IOController::requestSwapchainRecreation()
{
	shouldRecreateSwapchain = true;
}

void IOController::readInputs()
{
	memset(&keyEvent, 0, VKA_KEY_COUNT * sizeof(bool));
	mouse.resetEvents();
	window->pollEvents();
}

void IOController::destroy()
{
	vkDestroySwapchainKHR(gState.device.logical, swapchain, nullptr);
	for (size_t i = 0; i < imageCount; i++)
	{
		vkDestroyImageView(gState.device.logical, imageViews[i], nullptr);
	}
	vkDestroySurfaceKHR(gState.device.instance, surface, nullptr);
	window->destroy();
}

void IOController::terminateWindowManager()
{
	window->terminateWindowManager();
}

bool IOController::shouldTerminate()
{
	return window->shouldClose();
}
}        // namespace vka