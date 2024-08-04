#include "AdvancedState.h"
#include <vka/core/core_state/ResourcePool.h>
#include <vka/core/core_utility/cmd_buffer_utility.h>
#include <vka/core/core_utility/general_commands.h>
namespace vka
{
void AdvancedState::init(DeviceCI &deviceCI, IOControlerCI &ioControllerCI, Window *window, AdvancedStateConfig &config)
{
	CoreState::init(deviceCI, ioControllerCI, window);
	swapchainAttachmentPool = new ResourcePool();
	heap                    = new ResourcePool();
	framebufferCache        = new FramebufferCache();
	modelCache              = new ModelCache(heap, config.modelPath, config.modelUsage);
	textureCache            = new TextureCache(heap, config.texturePath);
	swapchainImage          = nullptr;
	imguiWrapper            = new ImGuiWrapper();
}

void AdvancedState::destroy()
{
	vkDeviceWaitIdle(device.logical);
	swapchainAttachmentPool->clear();
	modelCache->clear();
	textureCache->clear();
	framebufferCache->clear();
	if (guiEnabled)
	{
		imguiWrapper->destroy();
		guiEnabled = false;
	};
	heap->clear();
	delete framebufferCache;
	delete modelCache;
	delete textureCache;
	delete heap;
	delete swapchainAttachmentPool;
	delete imguiWrapper;
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
	}
	guiRendered = false;
}
}        // namespace vka