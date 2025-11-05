#pragma once
#include <vka/core/core_state/IOControler.h>

namespace vka
{
class SwapchainImage_R;
class VrIOController
{
  public:
	VkExtent2D                 extent;
	VkSurfaceKHR               surface;
	VkFormat                   format;
	VkSwapchainKHR             swapchain;
	VkPresentModeKHR           presentMode;
	uint32_t                   imageCount;
	std::vector<VkImage>       images;
	std::vector<VkImageView>   imageViews;
	std::vector<VkImageLayout> imageLayouts;
	VkImageUsageFlags          imageUsage;
	Mouse                      mouse;
	bool                       keyPressed[VKA_KEY_COUNT];
	bool                       keyEvent[VKA_KEY_COUNT];
	bool                       keyPressedEvent[VKA_KEY_COUNT];
	bool                       keyReleasedEvent[VKA_KEY_COUNT];

	VrIOController() = default;
	~VrIOController() = default;

	void    configure(IOControlerCI &controllerCI, Window *window);
	void    init();
	void    requestSwapchainRecreation();
	void    readInputs();
	void    clearEvents();
	void    destroy();
	void    terminateWindowManager();
	void    updateSwapchain();
	bool    swapchainRecreated();
	bool    shouldTerminate();
	void    buildShaderLib();
	void    requestTerminate();
	Window *getWindow();

	DELETE_COPY_CONSTRUCTORS(VrIOController);

  protected:
	bool               shouldRecreateSwapchain;
	bool               swapchainWasRecreated;
	Window            *window;
	IOControlerCI      controllerCI;
	SwapChainDetails   swapChainDetails;
	VkSurfaceFormatKHR surfaceFormat;
};
}        // namespace vka