#include "misc_utility.h"
#include <chrono>
#include <vka/core/core_utility/cmd_buffer_utility.h>
#include <vka/core/core_utility/general_commands.h>
#include <vka/core/stateless/vk_types/default_values.h>
#include <vka/advanced_state/vr/XrSwapchainImage.h>
#include <vka/globals.h>

namespace vka
{
namespace
{
using Clock = std::chrono::high_resolution_clock;

struct SwapBuffersTiming
{
	uint64_t frames      = 0;
	double   hostMs      = 0.0;
	double   acquireMs   = 0.0;
	double   submitMs    = 0.0;
	double   presentMs   = 0.0;
	double   nextFrameMs = 0.0;
};

SwapBuffersTiming gSwapTiming{};

double elapsedMs(Clock::time_point start, Clock::time_point end)
{
	return std::chrono::duration<double, std::milli>(end - start).count();
}

void reportSwapTiming()
{
	if (gSwapTiming.frames == 0 || gSwapTiming.frames % 120 != 0)
	{
		return;
	}

	const double inv = 1.0 / static_cast<double>(gSwapTiming.frames);
	printVka("swapBuffers avg over %llu frames: host=%.3fms acquire=%.3fms submit=%.3fms present=%.3fms next=%.3fms\n",
	         static_cast<unsigned long long>(gSwapTiming.frames),
	         gSwapTiming.hostMs * inv,
	         gSwapTiming.acquireMs * inv,
	         gSwapTiming.submitMs * inv,
	         gSwapTiming.presentMs * inv,
	         gSwapTiming.nextFrameMs * inv);
	gSwapTiming = {};
}
}        // namespace


void swapBuffersVr(std::vector<CmdBuffer> cmdBufs)
{
	for (auto &cmdBuf : cmdBufs)
	{
		cmdClearState(cmdBuf);
	}
	cmdBarrier(cmdBufs[0], VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
	           VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_MEMORY_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_TRANSFER_READ_BIT);
	gState.hostCache->update(cmdBufs[0]);
	SubmitSynchronizationInfo syncInfo = gState.acquireNextSwapchainImage();
	submit(cmdBufs, gState.device.universalQueues[0], syncInfo);
	gState.presentFrame();
	if (gState.isVrEnabled())
	{
		gState.xrHeadset.endFrame();
	}
	gState.nextFrame();
	if (gState.isVrEnabled())
	{
		uint32_t imageIdxLeft, imageIdxRight;
		gState.xrHeadset.beginFrame(imageIdxLeft, imageIdxRight);
	}
}

void swapBuffers(std::vector<CmdBuffer> cmdBufs)
{
	if (gState.isVrEnabled())
	{
		swapBuffersVr(cmdBufs);
		return;
	}
	for (auto &cmdBuf : cmdBufs)
	{
		cmdClearState(cmdBuf);
	}
	cmdBarrier(cmdBufs[0], VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
	           VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_MEMORY_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_TRANSFER_READ_BIT);
	auto t0 = Clock::now();
	gState.hostCache->update(cmdBufs[0]);
	auto t1 = Clock::now();
	SubmitSynchronizationInfo syncInfo = gState.acquireNextSwapchainImage();
	auto t2 = Clock::now();
	submit(cmdBufs, gState.device.universalQueues[0], syncInfo);
	auto t3 = Clock::now();
	gState.presentFrame();
	auto t4 = Clock::now();
	gState.nextFrame();
	auto t5 = Clock::now();

	gSwapTiming.frames += 1;
	gSwapTiming.hostMs += elapsedMs(t0, t1);
	gSwapTiming.acquireMs += elapsedMs(t1, t2);
	gSwapTiming.submitMs += elapsedMs(t2, t3);
	gSwapTiming.presentMs += elapsedMs(t3, t4);
	gSwapTiming.nextFrameMs += elapsedMs(t4, t5);
	reportSwapTiming();

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

glm::vec2 mouseViewCoord(vka::Rect2D<float> viewRect)
{
	glm::vec2 coord;
	coord.x = (gState.io.mouse.pos.x - viewRect.x * gState.io.extent.width) / (viewRect.width * gState.io.extent.width);
	coord.y = (gState.io.mouse.pos.y - viewRect.y * gState.io.extent.height) / (viewRect.height * gState.io.extent.height);
	return coord;
}

bool mouseInView(vka::Rect2D<float> viewRect)
{
	glm::vec2 pos = mouseViewCoord(viewRect);
	bool inView = true;
	inView      = inView && pos.x >= 0 && pos.x <= 1;
	inView      = inView && pos.y >= 0 && pos.y <= 1;
	return inView;
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
Image getVrSwapchainImage(XrEye eye)
{
	if (gState.xrSwapchainImages[eye] != nullptr)
	{
		delete gState.xrSwapchainImages[eye];
	}
	gState.xrSwapchainImages[eye] = new XrSwapchainImage_R(eye);
	gState.xrSwapchainImages[eye]->setClearValue({0.f, 0.f, 0.f, 1.f});
	return gState.xrSwapchainImages[eye];
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
