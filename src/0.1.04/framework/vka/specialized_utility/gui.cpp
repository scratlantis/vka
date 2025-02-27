#include "gui.h"
#include <vka/advanced_utility/complex_commands.h>
#include <vka/advanced_utility/misc_utility.h>
#include <vka/core/core_utility/cmd_buffer_utility.h>
#include <vka/core/core_utility/general_commands.h>
#include <vka/globals.h>

#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>


namespace vka
{



void configureGui_Default()
{
	VKA_ASSERT(!gState.guiConfigured);
	gState.guiConfigured = true;
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImPlot::CreateContext(); 	  // ImPlot context for fancy plots
	ImGuiIO &io         = ImGui::GetIO();
	float    sizePixels = 12;
	io.Fonts->AddFontFromFileTTF("../external/IMGUI/misc/fonts/Roboto-Medium.ttf", sizePixels);
	io.Fonts->AddFontFromFileTTF("../external/IMGUI/misc/fonts/Cousine-Regular.ttf", sizePixels);
	io.Fonts->AddFontFromFileTTF("../external/IMGUI/misc/fonts/DroidSans.ttf", sizePixels);
	io.Fonts->AddFontFromFileTTF("../external/IMGUI/misc/fonts/Karla-Regular.ttf", sizePixels);
	io.Fonts->AddFontFromFileTTF("../external/IMGUI/misc/fonts/ProggyClean.ttf", sizePixels);
	io.Fonts->AddFontFromFileTTF("../external/IMGUI/misc/fonts/ProggyTiny.ttf", sizePixels);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);
	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0);
	ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1);
	ImGui::StyleColorsLight();
}
void enableGui(RenderPassDefinition rpDef, uint32_t subpassIdx)
{
	if (!gState.guiConfigured)
	{
		configureGui_Default();
	}
	VKA_ASSERT(!gState.guiEnabled);
	gState.guiEnabled       = true;
	gState.guiRenderPassDef = rpDef;
	gState.imguiWrapper->init(gState.cache->fetch(rpDef), subpassIdx);
	gState.imguiWrapper->upload();
}

void enableGui()
{
	RenderPassDefinition swapchainLoadRPDef = defaultRenderPass();
	addColorAttachment(swapchainLoadRPDef, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, gState.io.format, false);
	enableGui(swapchainLoadRPDef, 0);
}

void cmdRenderGui(CmdBuffer cmdBuf, Image target, float x, float y, float width, float height)
{
	VKA_ASSERT(gState.guiEnabled);
	VkRect2D_OP   renderArea  = getScissorRect(x, y, width, height);
	VkRenderPass  renderPass  = gState.cache->fetch(gState.guiRenderPassDef);
	VkFramebuffer framebuffer = gState.framebufferCache->fetch(renderPass, {target});
	if (cmdBuf->state.framebuffer != framebuffer || cmdBuf->state.renderPass != renderPass || cmdBuf->state.renderArea != renderArea)
	{
		cmdClearState(cmdBuf);
		cmdBuf->state.framebuffer = framebuffer;
		cmdBuf->state.renderPass  = renderPass;
		cmdBuf->state.renderArea  = renderArea;
		cmdStartRenderPass(cmdBuf, renderPass, framebuffer, renderArea, {});
	}
	if (gState.guiRendered == false)
	{
		gState.imguiWrapper->newFrame();
		gState.guiRendered = true;
	}
	gState.imguiWrapper->render(cmdBuf);
}

void cmdRenderGui(CmdBuffer cmdBuf, Image target)
{
	cmdRenderGui(cmdBuf, target, 0.f, 0.f, 1.f, 1.f);
}

void setGuiDimensions(Rect2D<float> rect)
{
	VkRect2D_OP viewport = getScissorRect(rect.x, rect.y, rect.width, rect.height);
	ImGui::SetNextWindowPos({(float) viewport.offset.x, (float) viewport.offset.y});
	ImGui::SetNextWindowSize({(float) viewport.extent.width, (float) viewport.extent.height});
}
void beginGui(std::string name, Rect2D<float> rect, uint32_t flags, bool *isOpen = nullptr)
{
	if (!gState.guiRendered)
	{
		gState.imguiWrapper->newFrame();
		gState.guiRendered = true;
	}
	setGuiDimensions(rect);
	ImGui::Begin(name.c_str(), isOpen, flags);
}

void endGui()
{
	ImGui::End();
}

}        // namespace vka