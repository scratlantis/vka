#include "misc_utility.h"
#include <vka/core/core_utility/cmd_buffer_utility.h>
#include <vka/core/core_utility/general_commands.h>
#include <vka/core/stateless/vk_types/default_values.h>
#include <vka/globals.h>

namespace vka
{
void swapBuffers(std::vector<CmdBuffer> cmdBufs)
{
	for (auto &cmdBuf : cmdBufs)
	{
		cmdClearState(cmdBuf);
	}
	SubmitSynchronizationInfo syncInfo = gState.acquireNextSwapchainImage();
	submit(cmdBufs, gState.device.universalQueues[0], syncInfo);
	//vkDeviceWaitIdle(gState.device.logical);
	gState.presentFrame();
	gState.nextFrame();
}

vka::VkRect2D_OP getScissorRect()
{
	return VkRect2D_OP::absRegion(VkRect2D_OP(gState.io.extent), Rect2D<float>{0, 0, 1, 1});
}

vka::VkRect2D_OP getScissorRect(float x, float y, float width, float height)
{
	return VkRect2D_OP::absRegion(VkRect2D_OP(gState.io.extent), Rect2D<float>{x, y,width, height});
}
vka::VkRect2D_OP getScissorRect(Rect2D<float> rect)
{
	return VkRect2D_OP::absRegion(VkRect2D_OP(gState.io.extent), rect);
}

vka::Rect2D<float> addAbsOffsetToRect(Rect2D<float> rect, float x, float y)
{
	glm::vec2 offset = {x / float(gState.io.extent.width), y / float(gState.io.extent.height)};
	return Rect2D<float>{rect.x + offset.x, rect.y + offset.y, rect.width - offset.x, rect.height - offset.y};
}


vka::Rect2D<float> addMarginToRect(Rect2D<float> rect, float margin)
{
	return Rect2D<float>{ rect.x + margin, rect.y + margin, rect.width - 2 * margin, rect.height - 2 * margin };
}


Image createSwapchainAttachment(VkFormat format, VkImageUsageFlags usageFlags, VkImageLayout initialLayout)
{
	VkImageCreateInfo ci = ImageCreateInfo_Default(usageFlags, gState.io.extent, format);
	ci.initialLayout     = initialLayout;
	Image img         = new Image_R(gState.swapchainAttachmentPool, ci, true);
	img->createHandles();
	if (usageFlags & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
	{
		img->setClearValue({1.f, 0});
	}
	else
	{
		img->setClearValue({0.f, 0.f, 0.f, 1.f});
	}
	return img;
}

Image createSwapchainAttachment(VkFormat format, VkImageUsageFlags usageFlags, VkImageLayout initialLayout, float widthCoef, float heightCoef)
{
	VkImageCreateInfo ci = ImageCreateInfo_Default(usageFlags, gState.io.extent, format);
	ci.initialLayout     = initialLayout;
	Image img            = new Image_R(gState.swapchainAttachmentPool, ci, true, widthCoef, heightCoef);
	img->createHandles();
	if (usageFlags & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
	{
		img->setClearValue({1.f, 0});
	}
	else
	{
		img->setClearValue({0.f, 0.f, 0.f, 1.f});
	}
	return img;
}

Image getSwapchainImage()
{
	if (gState.swapchainImage != nullptr)
	{
		delete gState.swapchainImage;
	}
	gState.swapchainImage = new SwapchainImage_R();
	gState.swapchainImage->setClearValue({0.f, 0.f, 0.f, 1.f});
	return gState.swapchainImage;
}

void clearShaderCache()
{
	vkDeviceWaitIdle(gState.device.logical);
	gState.cache->clearShaders();
	gState.shaderLog = "";
}

void waitIdle()
{
	printVka("Waiting for device idle");
	vkDeviceWaitIdle(gState.device.logical);
}

}		// namespace vka