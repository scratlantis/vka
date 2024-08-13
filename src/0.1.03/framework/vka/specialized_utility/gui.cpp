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
	ImGuiIO &io         = ImGui::GetIO();
	float    sizePixels = 16;
	io.Fonts->AddFontFromFileTTF("../external/IMGUI/misc/fonts/Roboto-Medium.ttf", sizePixels);
	io.Fonts->AddFontFromFileTTF("../external/IMGUI/misc/fonts/Cousine-Regular.ttf", sizePixels);
	io.Fonts->AddFontFromFileTTF("../external/IMGUI/misc/fonts/DroidSans.ttf", sizePixels);
	io.Fonts->AddFontFromFileTTF("../external/IMGUI/misc/fonts/Karla-Regular.ttf", sizePixels);
	io.Fonts->AddFontFromFileTTF("../external/IMGUI/misc/fonts/ProggyClean.ttf", sizePixels);
	io.Fonts->AddFontFromFileTTF("../external/IMGUI/misc/fonts/ProggyTiny.ttf", sizePixels);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);
	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0);
	ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1);
	ImGui::StyleColorsDark();
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
	CmdBuffer cmdBuffer = createCmdBuffer(gState.frame->stack);
	gState.imguiWrapper->upload(cmdBuffer);
	executeImmediat(cmdBuffer);
	gState.imguiWrapper->freeStaging();
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



namespace gvar_gui
{
void addGVar(GVar *gv)
{
	std::stringstream ss;
	switch (gv->type)
	{
		case GVAR_EVENT:
			gv->val.v_bool = ImGui::Button(gv->path.c_str());
			break;
		case GVAR_BOOL:
			ImGui::Checkbox(gv->path.c_str(), &gv->val.v_bool);
			break;
		case GVAR_FLOAT:
			ImGui::InputScalar(gv->path.c_str(), ImGuiDataType_Float, &gv->val.v_float);
			break;
		case GVAR_UNORM:
			ImGui::SliderFloat(gv->path.c_str(), &gv->val.v_float, 0.0f, 1.0f);
			break;
		case GVAR_UINT:
			ImGui::InputScalar(gv->path.c_str(), ImGuiDataType_U32, &gv->val.v_uint);
			break;
		case GVAR_INT:
			ImGui::InputInt(gv->path.c_str(), &gv->val.v_int);
			break;
		case GVAR_VEC3:
			ImGui::InputFloat3(gv->path.c_str(), gv->val.v_vec3);
			break;
		case GVAR_DISPLAY_VALUE:
			ImGui::Text(gv->path.c_str(), gv->val.v_float);
			break;
		case GVAR_ENUM:
			for (size_t i = 0; i < gv->enumVal.size(); i++)
			{
				ss << gv->enumVal[i] << '\0';
			}
			ImGui::Combo(gv->path.c_str(), &gv->val.v_int, ss.str().c_str(), 5);
			break;
		default:
			break;
	}
}

void buildGui(std::vector<GVar *> gvar, std::vector<std::string> categories, VkRect2D_OP viewport)
{
	if (!gState.guiRendered)
	{
		gState.imguiWrapper->newFrame();
		gState.guiRendered = true;
	}
	ImGui::SetNextWindowPos({(float) viewport.offset.x, (float) viewport.offset.y});
	ImGui::SetNextWindowSize({(float) viewport.extent.width, (float) viewport.extent.height});
	ImGui::Begin("Gvar", 0, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoResize);
	std::sort(gvar.begin(), gvar.end(), [](GVar *a, GVar *b) { return a->sortId < b->sortId; });
	uint32_t currentCategory = 0;
	for (uint32_t i = 0; i < gvar.size(); i++)
	{
		GVar *gv = gvar[i];
		if (i == 0 || currentCategory != gv->sortId)
		{
			ImGui::SetNextItemOpen(true, ImGuiCond_Once);
		}
		std::string name;
		if (categories.size() > gv->sortId)
		{
			name = categories[gv->sortId];
		}
		else
		{
			name = "Unknown_" + gv->sortId;
		}
		if (ImGui::CollapsingHeader(name.c_str()))
		{
			ImGui::PushItemWidth(ImGui::GetWindowSize().x * 0.3);
			addGVar(gv);
		}
		currentCategory = gvar[i]->sortId;
	}
	ImGui::End();
}
}        // namespace gvar_gui

namespace shader_console_gui
{
void buildGui(VkRect2D_OP viewport)
{
	if (gState.shaderLog == "")
		return;
	if (!gState.guiRendered)
	{
		gState.imguiWrapper->newFrame();
		gState.guiRendered = true;
	}
	ImGui::SetNextWindowPos({(float) viewport.offset.x, (float) viewport.offset.y});
	ImGui::SetNextWindowSize({(float) viewport.extent.width, (float) viewport.extent.height});
	ImGui::Begin("ShaderLog", 0, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
	ImGui::TextWrapped(gState.shaderLog.c_str());
	ImGui::End();
}
}        // namespace shader_console_gui

}        // namespace vka