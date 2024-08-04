#include "CoreState.h"
#include <vka/core/core_state/ResourcePool.h>
#include <vka/core/core_state/ResourceCache.h>

namespace vka
{
void CoreState::initFrames()
{
	VKA_CHECK(initBits & (STATE_INIT_DEVICE_BIT | STATE_INIT_IO_BIT));
	VkSemaphoreCreateInfo semaphoreCI{VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
	VkFenceCreateInfo     fenceCI{VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
	fenceCI.flags = VK_FENCE_CREATE_SIGNALED_BIT;
	frames.resize(io.imageCount);
	for (size_t i = 0; i < io.imageCount; i++)
	{
		frames[i].frameIndex = i;
		frames[i].next       = &frames[NEXT_INDEX(i, io.imageCount)];
		frames[i].previous   = &frames[PREVIOUS_INDEX(i, io.imageCount)];
		frames[i].stack      = new ResourcePool();
		VK_CHECK(vkCreateSemaphore(device.logical, &semaphoreCI, nullptr, &frames[i].imageAvailableSemaphore));
		VK_CHECK(vkCreateSemaphore(device.logical, &semaphoreCI, nullptr, &frames[i].renderFinishedSemaphore));
		VK_CHECK(vkCreateFence(device.logical, &fenceCI, nullptr, &frames[i].inFlightFence));
	}
	frame = frames.data();
	initBits |= STATE_INIT_FRAME_SEMAPHORE_BIT;
	initBits |= STATE_INIT_FRAME_BIT;
}

void CoreState::destroyFrames()
{
	for (auto &frame : frames)
	{
		vkDestroySemaphore(device.logical, frame.imageAvailableSemaphore, nullptr);
		vkDestroySemaphore(device.logical, frame.renderFinishedSemaphore, nullptr);
		vkDestroyFence(device.logical, frame.inFlightFence, nullptr);
		frame.stack->clear();
		delete frame.stack;
	}
}

void CoreState::nextFrame()
{
	frame = frame->next;
	VK_CHECK(vkWaitForFences(device.logical, 1, &frame->inFlightFence, VK_TRUE, UINT64_MAX));
	frame->stack->clear();
	io.readInputs();
	io.updateSwapchain();
	if (io.swapchainRecreated())
	{
		frame = &frames[0];
	}
	io.imageLayouts[frame->frameIndex] = VK_IMAGE_LAYOUT_UNDEFINED;
}

CoreState::CoreState()
{
}

void CoreState::init(DeviceCI &deviceCI, IOControlerCI ioControllerCI, Window *window)
{
	initBits = 0;
	window->initWindowManager();
	window->addInstanceExtensions(deviceCI.enabledInstanceExtensions);
	device.configure(deviceCI);
	device.createInstance();
	io.configure(ioControllerCI, window);
	device.selectPhysicalDevice();
	device.createLogicalDevice();
	io.init();
	initFrames();
	memAlloc.init();
	cmdAlloc.init();
	cache = new ResourceCache();
	initBits |= STATE_INIT_ALL_BIT;
}
SubmitSynchronizationInfo CoreState::acquireNextSwapchainImage()
{
	uint32_t imageIndex;
	VK_CHECK(vkAcquireNextImageKHR(device.logical, io.swapchain, UINT64_MAX, frame->imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex));
	VK_CHECK(vkResetFences(device.logical, 1, &frame->inFlightFence));
	SubmitSynchronizationInfo syncInfo{};
	syncInfo.waitSemaphores   = {frame->imageAvailableSemaphore};
	syncInfo.waitDstStageMask = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
	syncInfo.signalSemaphores = {frame->renderFinishedSemaphore};
	syncInfo.signalFence      = frame->inFlightFence;
	return syncInfo;
}

void CoreState::presentFrame()
{
	VkPresentInfoKHR presentInfo{VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores    = &frame->renderFinishedSemaphore;
	presentInfo.swapchainCount     = 1;
	presentInfo.pSwapchains        = &io.swapchain;
	presentInfo.pImageIndices      = &frame->frameIndex;
	VK_CHECK(vkQueuePresentKHR(device.universalQueues[0], &presentInfo));
}

void CoreState::destroy()
{
	vkDeviceWaitIdle(device.logical);

	io.destroy();
	destroyFrames();
	cache->clearAll();
	delete cache;
	cmdAlloc.destroy();
	memAlloc.destroy();
	device.destroy();
	io.terminateWindowManager();
}

}        // namespace vka