#pragma once
#include <vka/advanced_state/AdvancedState.h>
#include <vka/advanced_state/vr/VrIOController.h>
#include <vka/advanced_state/vr/VrDevice.h>

#include "XrContext.h"
#include "XrHeadset.h"
#include "XrControllers.h"
#include "XrSwapchainImage.h"



namespace vka
{	

// Preliminary vr integration
class VrState
{
	bool vrEnabled = false;
  public:
	VrState() = default;
	~VrState() = default;

	void       enableVr();
	void       disableVr();
	bool 	 isVrEnabled() const;

	XrContext xrContext;
	XrHeadset xrHeadset;
	XrControllers xrControllers;
	std::vector<XrSwapchainImage_R*> xrSwapchainImages;


	//Core State
  public:
	uint32_t        initBits;
	VrDevice        device;
	VrIOController  io;
	IResourceCache *cache;
	Frame          *frame;
	MemAllocator    memAlloc;
	CmdAllocator    cmdAlloc;
	uint32_t        frameTime;        // last frame time in ms

	std::string shaderLog;

	SubmitSynchronizationInfo acquireNextSwapchainImage();
	void                      presentFrame();

  protected:
	virtual void coreNextFrame();
	virtual void coreDestroy();
	void         coreInit(DeviceCI &deviceCI, IOControlerCI ioControllerCI, Window *window);
	void         initFrames();
	void         destroyFrames();
	uint32_t     lastTimeStamp;
	std::vector<Frame> frames;


	// Advanced State
  public:
	// Caches
	FramebufferCache    *framebufferCache;
	ModelCache          *modelCache;
	TextureCache        *textureCache;
	DepthBufferCache    *depthBufferCache;
	DataCache           *dataCache;
	ImGuiTextureIDCache *imguiTextureIDCache;
	BinaryLoadCache     *binaryLoadCache;
	ImageCache          *imageCache;
	UniqueResourceCache *uniqueResourceCache;

	UploadQueue   *uploadQueue;
	Exporter      *exporter;
	IResourcePool *heap;

	IResourcePool     *hostCachedHeap;
	HostCache         *hostCache;
	FeedbackDataCache *feedbackDataCache;

	// Swapchain attachments
	IResourcePool    *swapchainAttachmentPool;
	SwapchainImage_R *swapchainImage;

	// Gui
	ImGuiWrapper        *imguiWrapper;
	RenderPassDefinition guiRenderPassDef;
	bool                 guiRendered   = false;
	bool                 guiEnabled    = false;
	bool                 guiConfigured = false;

	void         init(DeviceCI &deviceCI, IOControlerCI &ioControllerCI, Window *window, AdvancedStateConfig &config);
	void         destroy();
	virtual void nextFrame();
	void         updateSwapchainAttachments();

  protected:
	IResourcePool *hostCacheLocalPool;
	DELETE_COPY_CONSTRUCTORS(VrState);
};
} // namespace vka