#include "AdvancedState.h"
#include <vka/core/core_state/ResourcePool.h>
#include <vka/core/core_utility/cmd_buffer_utility.h>
#include <vka/core/core_utility/general_commands.h>
#include <implot.h>
namespace vka
{
void AdvancedState::init(DeviceCI &deviceCI, IOControlerCI &ioControllerCI, Window *window, AdvancedStateConfig &config)
{
	CoreState::init(deviceCI, ioControllerCI, window);
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
	binaryLoadCache			= new BinaryLoadCache(heap);
	imageCache				= new ImageCache(heap);
	uniqueResourceCache		= new UniqueResourceCache(heap);
	exporter				= new Exporter();
	uploadQueue				= new UploadQueue();
}

void AdvancedState::destroy()
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
	CoreState::destroy();
}

void AdvancedState::nextFrame()
{
	CoreState::nextFrame();
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
void AdvancedState::updateSwapchainAttachments()
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