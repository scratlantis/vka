#pragma once
#include <vulkan/vulkan.h>
#include <vka/core/resources/IResourcePool.h>
#include <vka/core/resources/IResourceCache.h>
#include <vka/core/core_state/Device.h>
#include <vka/core/core_state/IOControler.h>
#include <vka/core/core_state/MemAllocator.h>
#include <vka/core/core_state/CmdAllocator.h>

namespace vka
{

struct Frame
{
	VkSemaphore    renderFinishedSemaphore;
	VkSemaphore    imageAvailableSemaphore;
	VkFence        inFlightFence;
	uint32_t       frameIndex;
	IResourcePool *stack;
	Frame         *next;
	Frame         *previous;
};

enum StateInitialisationBits
{
	STATE_INIT_DEVICE_INSTANCE_BIT = 0x1,
	STATE_INIT_DEVICE_PHYSICAL_BIT = 0x2,
	STATE_INIT_DEVICE_LOGICAL_BIT  = 0x4,
	STATE_INIT_DEVICE_BIT          = 0x8,
	STATE_INIT_IO_WINDOW_BIT       = 0x10,
	STATE_INIT_IO_SWAPCHAIN_BIT    = 0x20,
	STATE_INIT_IO_BIT              = 0x40,
	STATE_INIT_FRAME_SEMAPHORE_BIT = 0x80,
	STATE_INIT_FRAME_FENCE_BIT     = 0x100,
	STATE_INIT_FRAME_STACK_BIT     = 0x200,
	STATE_INIT_FRAME_BIT           = 0x400,
	STATE_INIT_HEAP_BIT            = 0x800,
	STATE_INIT_CACHE_BIT           = 0x1000,
	STATE_INIT_MEMALLOC_BIT        = 0x2000,
	STATE_INIT_DESCALLOC_BIT       = 0x4000,
	STATE_INIT_QUERYALLOC_BIT      = 0x8000,
	STATE_INIT_CMDALLOC_BIT        = 0x10000,
	STATE_INIT_ALL_BIT             = 0x20000
};


class CoreState
{
  public:
	uint32_t        initBits;
	Device          device;
	IOController    io;
	IResourceCache *cache;
	Frame          *frame;
	MemAllocator    memAlloc;
	CmdAllocator    cmdAlloc;
	uint32_t		frameTime; // last frame time in ms

	std::string shaderLog;

	CoreState();
	void                      init(DeviceCI &deviceCI, IOControlerCI ioControllerCI, Window *window);
	SubmitSynchronizationInfo acquireNextSwapchainImage();
	void                      presentFrame();
	virtual void			  nextFrame();
	virtual void              destroy();

  private:
	void               initFrames();
	void               destroyFrames();
	uint32_t           lastTimeStamp;

	DELETE_COPY_CONSTRUCTORS(CoreState);
  protected:
	std::vector<Frame> frames;
};
}

