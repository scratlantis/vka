#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <string>
#include "Window.h"
#include <glm/glm.hpp>
#include <vka/core/stateless/utility/macros.h>
#include <vka/core/stateless/utility/setup.h>

#define VKA_KEY_COUNT 1024
namespace vka
{

struct Mouse
{
	glm::vec2 change;
	glm::vec2 pos;
	bool      leftPressed;
	bool      leftEvent;
	bool      rightPressed;
	bool      rightEvent;
	bool	  middlePressed;
	bool	  middleEvent;
	double scrollChange;
	void   resetEvents()
	{
		leftEvent    = false;
		rightEvent   = false;
		middleEvent  = false;
		change       = glm::vec2(0, 0);
		scrollChange = 0.0;
	}
};
struct IOControlerCI
{
	std::string                     windowTitel;
	bool                            resizable;
	VkExtent2D                      size;
	CursorMode                      cursorMode;
	std::vector<VkSurfaceFormatKHR> preferedFormats;
	std::vector<VkPresentModeKHR>   preferedPresentModes;
	uint32_t                        preferedSwapchainImageCount;

	WindowCI getWindowCI();
};

class SwapchainImage_R;
class IOController
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

	IOController();
	void    configure(IOControlerCI &controllerCI, Window *window);
	void    init();
	void    requestSwapchainRecreation();
	void    readInputs();
	void    destroy();
	void    terminateWindowManager();
	void    updateSwapchain();
	bool    swapchainRecreated();
	bool    shouldTerminate();
	Window *getWindow();

	DELETE_COPY_CONSTRUCTORS(IOController);

  private:
	bool               shouldRecreateSwapchain;
	bool               swapchainWasRecreated;
	Window            *window;
	IOControlerCI      controllerCI;
	SwapChainDetails   swapChainDetails;
	VkSurfaceFormatKHR surfaceFormat;
};
}