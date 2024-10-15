#pragma once
#include <string>
#include <vector>
#include <vulkan/vulkan.h>
namespace vka
{

enum CursorMode
{
	WINDOW_CURSOR_MODE_VISIBLE,
	WINDOW_CURSOR_MODE_HIDDEN,
	WINDOW_CURSOR_MODE_DISABLED
};

struct WindowCI
{
	uint32_t    width;
	uint32_t    height;
	std::string title;
	bool        resizable;
	CursorMode  cursorMode;
};

class Window
{
  public:
	virtual ~Window(){};
	virtual void         initWindowManager()                                          = 0;
	virtual void         terminateWindowManager()                                     = 0;
	virtual void         init(const WindowCI &windowCI, VkInstance &instance)         = 0;
	virtual void         pollEvents()                                                 = 0;
	virtual void         waitEvents()                                                 = 0;
	virtual bool         shouldClose()                                                = 0;
	virtual void         requestClose()                                               = 0;
	virtual VkExtent2D   size() const                                                 = 0;
	virtual void         changeSize(VkExtent2D newSize)                               = 0;
	virtual VkSurfaceKHR getSurface() const                                           = 0;
	virtual void         destroy()                                                    = 0;
	virtual void         initImGui()                                                  = 0;
	virtual void         newFrameImGui()                                              = 0;
	virtual void         checkToggleFullScreen()                                           = 0;
	virtual void         addInstanceExtensions(std::vector<const char *> &extensions) = 0;

  private:
};

}        // namespace vka