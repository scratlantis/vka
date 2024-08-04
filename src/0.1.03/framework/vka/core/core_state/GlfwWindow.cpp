#include <imgui_impl_glfw.h>
#include "GlfwWindow.h"
#include <vka/globals.h>
#include <vka/core/stateless/utility/macros.h>

namespace vka
{

GlfwWindow::GlfwWindow()
{
}

void GlfwWindow::initWindowManager()
{
	glfwInit();
}
void GlfwWindow::terminateWindowManager()
{
	glfwTerminate();
}

void GlfwWindow::init(const WindowCI &windowCI, VkInstance &instance)
{
	VKA_CHECK(glfwInit());
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

	// allow it to resize
	glfwWindowHint(GLFW_RESIZABLE, windowCI.resizable ? GLFW_TRUE : GLFW_FALSE);
	width  = windowCI.width;
	height = windowCI.height;
	window = glfwCreateWindow(width, height, windowCI.title.c_str(), NULL, NULL);
	VKA_CHECK(window);
	glfwGetFramebufferSize(window, &width, &height);
	glfwMakeContextCurrent(window);

	int cursorMode;
	switch (windowCI.cursorMode)
	{
		case WINDOW_CURSOR_MODE_VISIBLE:
			cursorMode = GLFW_CURSOR_NORMAL;
			break;
		case WINDOW_CURSOR_MODE_HIDDEN:
			cursorMode = GLFW_CURSOR_HIDDEN;
			break;
		case WINDOW_CURSOR_MODE_DISABLED:
			cursorMode = GLFW_CURSOR_DISABLED;
			break;
		default:
			cursorMode = GLFW_CURSOR_NORMAL;
			break;
	}
	glfwSetInputMode(window, GLFW_CURSOR, cursorMode);

	glfwSetWindowUserPointer(window, this);

	glfwSetKeyCallback(window, key_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, mouse_scroll_callback);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	VK_CHECK(glfwCreateWindowSurface(instance, window, nullptr, &surface));
}

VkSurfaceKHR GlfwWindow::getSurface() const
{
	return surface;
}

VkExtent2D GlfwWindow::size() const
{
	VkExtent2D extent{};
	extent.width  = width;
	extent.height = height;
	return extent;
}

void GlfwWindow::pollEvents()
{
	glfwPollEvents();
}

void GlfwWindow::waitEvents()
{
	glfwGetFramebufferSize(window, &width, &height);
	glfwWaitEvents();
}

bool GlfwWindow::shouldClose()
{
	return glfwWindowShouldClose(window);
}

void GlfwWindow::requestClose()
{
	glfwSetWindowShouldClose(window, GL_TRUE);
}

void GlfwWindow::changeSize(VkExtent2D newSize)
{
	glfwSetWindowSize(window, newSize.width, newSize.height);
}

void GlfwWindow::addInstanceExtensions(std::vector<const char *> &extensions)
{
	uint32_t     glfwExtensionsCount = 0;
	const char **glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionsCount);
	for (size_t i = 0; i < glfwExtensionsCount; i++)
	{
		extensions.push_back(glfwExtensions[i]);
	}
}

void GlfwWindow::initImGui()
{
	ImGui_ImplGlfw_InitForVulkan(window, true);
}

void GlfwWindow::newFrameImGui()
{
	ImGui_ImplGlfw_NewFrame();
}

void GlfwWindow::destroy()
{
	glfwDestroyWindow(window);
}

GlfwWindow::~GlfwWindow()
{
}

void GlfwWindow::framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
	GlfwWindow *thisWindow = static_cast<GlfwWindow *>(glfwGetWindowUserPointer(window));
	thisWindow->width      = width;
	thisWindow->height     = height;
	gState.io.requestSwapchainRecreation();
}

void GlfwWindow::key_callback(GLFWwindow *window, int key, int code, int action, int mode)
{
	if (key >= 0 && key < VKA_KEY_COUNT)
	{
		gState.io.keyEvent[key] = true;
		if (action == GLFW_PRESS)
		{
			gState.io.keyPressed[key] = true;
		}
		else if (action == GLFW_RELEASE)
		{
			gState.io.keyPressed[key] = false;
		}
	}
}

void GlfwWindow::mouse_callback(GLFWwindow *window, double x_pos, double y_pos)
{
	gState.io.mouse.change += glm::vec2(static_cast<float>(x_pos), static_cast<float>(y_pos)) - gState.io.mouse.pos;
	gState.io.mouse.pos = glm::vec2(static_cast<float>(x_pos), static_cast<float>(y_pos));
}

void GlfwWindow::mouse_button_callback(GLFWwindow *window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_RIGHT)
	{
		gState.io.mouse.rightEvent = true;
		if (action == GLFW_PRESS)
		{
			gState.io.mouse.rightPressed = true;
		}
		else if (action == GLFW_RELEASE)
		{
			gState.io.mouse.rightPressed = false;
		}
	}
	else if (button == GLFW_MOUSE_BUTTON_LEFT)
	{
		gState.io.mouse.leftEvent = true;
		if (action == GLFW_PRESS)
		{
			gState.io.mouse.leftPressed = true;
		}
		else if (action == GLFW_RELEASE)
		{
			gState.io.mouse.leftPressed = false;
		}
	}
}

void GlfwWindow::mouse_scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
	gState.io.mouse.scrollChange = yoffset;
}

}        // namespace vka