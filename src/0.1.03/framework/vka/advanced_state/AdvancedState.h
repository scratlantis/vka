#pragma once
#include "ImGuiWrapper.h"
#include "caches/FramebufferCache.h"
#include "caches/ModelCache.h"
#include "caches/TextureCache.h"
#include <vka/core/core_state/CoreState.h>
#include <vka/core/resources/cachable/RenderPass.h>

namespace vka
{
struct AdvancedStateConfig
{
	std::string        modelPath;
	std::string        texturePath;
	VkBufferUsageFlags modelUsage;
};

class AdvancedState : public CoreState
{
  public:
	// Caches
	FramebufferCache *framebufferCache;
	ModelCache       *modelCache;
	TextureCache     *textureCache;

	IResourcePool *heap;

	// Swapchain attachments
	IResourcePool    *swapchainAttachmentPool;
	SwapchainImage_R *swapchainImage;

	// Gui
	ImGuiWrapper        *imguiWrapper;
	RenderPassDefinition guiRenderPassDef;
	bool                 guiRendered = false;
	bool                 guiEnabled  = false;
	bool                 guiConfigured  = false;

	AdvancedState(){};
	~AdvancedState(){};
	void         init(DeviceCI &deviceCI, IOControlerCI &ioControllerCI, Window *window, AdvancedStateConfig &config);
	void         destroy() override;
	virtual void nextFrame() override;
};
}        // namespace vka
