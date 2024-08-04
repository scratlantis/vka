#pragma once
#define GLFW_INCLUDE_VULKAN
#include "Window.h"
#include <GLFW/glfw3.h>
namespace vka
{

class GlfwWindow : public Window
{
  public:
	GlfwWindow();
	void initWindowManager();
	void terminateWindowManager();

	void         init(const WindowCI &windowCI, VkInstance &instance);
	VkExtent2D   size() const;
	VkSurfaceKHR getSurface() const;
	void         pollEvents();
	void         waitEvents();
	bool         shouldClose();
	void         requestClose();
	void         changeSize(VkExtent2D newSize);
	void         addInstanceExtensions(std::vector<const char *> &extensions);
	void         initImGui();
	void         newFrameImGui();
	void         destroy();

	~GlfwWindow();

  private:
	GLFWwindow  *window;
	GLint        width, height;
	VkSurfaceKHR surface;

	static void framebuffer_size_callback(GLFWwindow *window, int width, int height);
	static void key_callback(GLFWwindow *window, int key, int code, int action, int mode);
	static void mouse_callback(GLFWwindow *window, double x_pos, double y_pos);
	static void mouse_button_callback(GLFWwindow *window, int button, int action, int mods);
	static void mouse_scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
};

}        // namespace vka