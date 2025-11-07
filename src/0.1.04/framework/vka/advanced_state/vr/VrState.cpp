#include "VrState.h"
#include <vka/core/core_state/ResourcePool.h>
#include <vka/core/core_utility/cmd_buffer_utility.h>
#include <vka/core/core_utility/general_commands.h>
#include <implot.h>



XrInstance createInstance()
{
	XrInstance instance;

	static const char *const  applicationName  = "OpenXR Example";
	static const unsigned int majorVersion     = 0;
	static const unsigned int minorVersion     = 1;
	static const unsigned int patchVersion     = 0;
	static const char *const  extensionNames[] = {
        "XR_KHR_vulkan_enable",
        "XR_KHR_vulkan_enable2"};

	XrInstanceCreateInfo instanceCreateInfo{};
	instanceCreateInfo.type        = XR_TYPE_INSTANCE_CREATE_INFO;
	instanceCreateInfo.createFlags = 0;
	strcpy(instanceCreateInfo.applicationInfo.applicationName, applicationName);
	instanceCreateInfo.applicationInfo.applicationVersion = XR_MAKE_VERSION(majorVersion, minorVersion, patchVersion);
	strcpy(instanceCreateInfo.applicationInfo.engineName, applicationName);
	instanceCreateInfo.applicationInfo.engineVersion = XR_MAKE_VERSION(majorVersion, minorVersion, patchVersion);
	instanceCreateInfo.applicationInfo.apiVersion    = XR_MAKE_VERSION(1, 0, 34);
	instanceCreateInfo.enabledApiLayerCount          = 0;
	instanceCreateInfo.enabledApiLayerNames          = nullptr;
	instanceCreateInfo.enabledExtensionCount         = sizeof(extensionNames) / sizeof(const char *);
	instanceCreateInfo.enabledExtensionNames         = extensionNames;

	XrResult result = xrCreateInstance(&instanceCreateInfo, &instance);

	if (result != XR_SUCCESS)
	{
		std::cerr << "Failed to create OpenXR instance: " << result << std::endl;
		return XR_NULL_HANDLE;
	}

	return instance;
}
void destroyInstance(XrInstance instance)
{
	xrDestroyInstance(instance);
}


namespace vka
{


void VrState::initFrames()
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

void VrState::destroyFrames()
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

void VrState::coreNextFrame()
{
	frame = frame->next;
	VK_CHECK(vkWaitForFences(device.logical, 1, &frame->inFlightFence, VK_TRUE, UINT64_MAX));
	frame->stack->clear();
	io.readInputs();
	io.getWindow()->checkToggleFullScreen();
	io.updateSwapchain();
	if (io.swapchainRecreated())
	{
		frame = &frames[0];
	}
	io.imageLayouts[frame->frameIndex] = VK_IMAGE_LAYOUT_UNDEFINED;

	uint32_t timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
	VKA_ASSERT(timestamp >= lastTimeStamp);
	if (lastTimeStamp != 0)
	{
		frameTime = timestamp - lastTimeStamp;
		if (frameTime == 0)
		{
			frameTime = 1;
		}
	}
	lastTimeStamp = timestamp;
}

void VrState::coreInit(DeviceCI &deviceCI, IOControlerCI ioControllerCI, Window *window)
{
	initBits = 0;
	window->initWindowManager();
	window->addInstanceExtensions(deviceCI.enabledInstanceExtensions);
	if (vrEnabled)
	{
		xrContext.initInstance(deviceCI);
	}
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
	io.buildShaderLib();
	frameTime     = 0;
	lastTimeStamp = 0;
	initBits |= STATE_INIT_ALL_BIT;

	if (vrEnabled)
	{
		xrHeadset.init();
		xrControllers.init();
		uint32_t imageIdxLeft, imageIdxRight;
		xrHeadset.beginFrame(imageIdxLeft, imageIdxRight);
	}


}
void VrState::enableVr()
{
	vrEnabled = true;
}
void VrState::disableVr()
{
	vrEnabled = false;
}
bool VrState::isVrEnabled() const
{
	return vrEnabled;
}
SubmitSynchronizationInfo VrState::acquireNextSwapchainImage()
{
	uint32_t imageIndex;
	VK_CHECK(vkAcquireNextImageKHR(device.logical, io.swapchain, UINT64_MAX, frame->imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex));
	VKA_ASSERT(imageIndex == frame->frameIndex);
	VK_CHECK(vkResetFences(device.logical, 1, &frame->inFlightFence));
	SubmitSynchronizationInfo syncInfo{};
	syncInfo.waitSemaphores   = {frame->imageAvailableSemaphore};
	syncInfo.waitDstStageMask = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
	syncInfo.signalSemaphores = {frame->renderFinishedSemaphore};
	syncInfo.signalFence      = frame->inFlightFence;
	return syncInfo;
}

void VrState::presentFrame()
{
	VkPresentInfoKHR presentInfo{VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores    = &frame->renderFinishedSemaphore;
	presentInfo.swapchainCount     = 1;
	presentInfo.pSwapchains        = &io.swapchain;
	presentInfo.pImageIndices      = &frame->frameIndex;
	VK_CHECK(vkQueuePresentKHR(device.universalQueues[0], &presentInfo));
}

void VrState::coreDestroy()
{
	vkDeviceWaitIdle(device.logical);

	if (vrEnabled)
	{
		//xrHeadset.endSession();
		xrControllers.destroy();
		xrHeadset.destroy();
		xrContext.destroy();
	}


	io.destroy();
	destroyFrames();
	cache->clearAll();
	delete cache;
	cmdAlloc.destroy();
	memAlloc.destroy();
	device.destroy();
	io.terminateWindowManager();
}



void VrState::init(DeviceCI &deviceCI, IOControlerCI &ioControllerCI, Window *window, AdvancedStateConfig &config)
{
	coreInit(deviceCI, ioControllerCI, window);
	xrSwapchainImages.resize(2U);
	swapchainAttachmentPool = new ResourcePool();
	heap                    = new ResourcePool();
	hostCachedHeap          = new ResourcePool();
	hostCacheLocalPool      = new ResourcePool();
	hostCache               = new HostCache(hostCachedHeap, hostCacheLocalPool);
	feedbackDataCache       = new FeedbackDataCache(hostCache);
	framebufferCache        = new FramebufferCache();
	modelCache              = new ModelCache(heap, config.modelUsage);
	textureCache            = new TextureCache(heap);
	swapchainImage          = nullptr;
	imguiWrapper            = new ImGuiWrapper();
	depthBufferCache        = new DepthBufferCache(heap);
	dataCache               = new DataCache(heap);
	imguiTextureIDCache     = new ImGuiTextureIDCache(heap);
	binaryLoadCache         = new BinaryLoadCache(heap);
	imageCache              = new ImageCache(heap);
	uniqueResourceCache     = new UniqueResourceCache(heap);
	exporter                = new Exporter();
	uploadQueue             = new UploadQueue();
}

void VrState::destroy()
{
	vkDeviceWaitIdle(device.logical);
	swapchainAttachmentPool->clear();
	framebufferCache->clear();
	modelCache->clear();
	dataCache->clear();
	imguiTextureIDCache->clear();
	imageCache->clear();
	uniqueResourceCache->clear();
	if (guiEnabled)
	{
		imguiWrapper->destroy();
		guiEnabled = false;
	};
	if (guiConfigured)
	{
		ImPlot::DestroyContext();
		ImGui::DestroyContext();
		guiConfigured = false;
	}
	for (auto &frame : frames)
	{
		frame.stack->clear();
	}
	heap->clear();
	hostCachedHeap->clear();
	hostCacheLocalPool->clear();
	delete exporter;
	delete framebufferCache;
	delete modelCache;
	delete textureCache;
	delete heap;
	delete swapchainAttachmentPool;
	delete imguiWrapper;
	delete depthBufferCache;
	delete imageCache;
	delete uniqueResourceCache;
	delete uploadQueue;
	if (swapchainImage != nullptr)
	{
		delete swapchainImage;
	}
	coreDestroy();
}

void VrState::nextFrame()
{
	coreNextFrame();
	if (vrEnabled)
	{
		xrControllers.sync(xrHeadset.space, xrHeadset.frameState.predictedDisplayTime);
	}
	if (io.swapchainRecreated())
	{
		if (initBits & STATE_INIT_ALL_BIT)
		{
			updateSwapchainAttachments();
		}
	}
	guiRendered = false;
	exporter->processExports();
	imguiTextureIDCache->processGarbage();
}

// resize attachments to swapchain size, and aquire desired layout
void VrState::updateSwapchainAttachments()
{
	// Resize swapchain attachments
	CmdBuffer cmdBuf = createCmdBuffer(frame->stack);
	for (auto it = swapchainAttachmentPool->getImagesBegin(); it != swapchainAttachmentPool->getImagesEnd(); ++it)
	{
		(*it)->changeExtent({io.extent.width, io.extent.height, 1});
		(*it)->recreate();
		cmdTransitionLayout(cmdBuf, (*it), (*it)->getInitialLayout());
	}
	executeImmediat(cmdBuf);
}

}        // namespace vka
