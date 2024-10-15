#include "gui.h"
#include <vka/advanced_utility/complex_commands.h>
#include <vka/advanced_utility/misc_utility.h>
#include <vka/core/core_utility/cmd_buffer_utility.h>
#include <vka/core/core_utility/general_commands.h>
#include <vka/globals.h>

#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

#include <nlohmann/json.hpp>
#include <fstream>
using json = nlohmann::json;
namespace vka
{
	void writeGVarToJson(GVar* gv, json& j)
	{
		switch (gv->type)
		{
		case GVAR_EVENT:
			    j[gv->id] = gv->val.v_bool;
			break;
		case GVAR_BOOL:
			j[gv->id] = gv->val.v_bool;
			break;
		case GVAR_FLOAT:
			j[gv->id] = gv->val.v_float;
			break;
		case GVAR_UNORM:
			j[gv->id] = gv->val.v_float;
			break;
		case GVAR_UINT:
			j[gv->id] = gv->val.v_uint;
			break;
		case GVAR_INT:
			j[gv->id] = gv->val.v_int;
			break;
		case GVAR_VEC3:
			j[gv->id] = {gv->val.v_vec3[0], gv->val.v_vec3[1], gv->val.v_vec3[2]};
			break;
		case GVAR_DISPLAY_VALUE:
			j[gv->id] = gv->val.v_float;
			break;
		case GVAR_ENUM:
			j[gv->id] = gv->val.v_int;
			break;
		case GVAR_UINT_RANGE:
			j[gv->id] = {gv->val.v_uint, gv->set.range.min.v_uint, gv->set.range.max.v_uint};
			break;
		case GVAR_FLOAT_RANGE:
			j[gv->id] = {gv->val.v_float, gv->set.range.min.v_float, gv->set.range.max.v_float};
			break;
		case GVAR_VEC3_RANGE:
			j[gv->id] = {gv->val.v_vec3[0], gv->val.v_vec3[1], gv->val.v_vec3[2], gv->set.range.min.v_float, gv->set.range.max.v_float};
			break;
		default:
			//DEBUG_BREAK;
			break;
		}
	}

	void readGVarFromJson(GVar* gv, json& j)
	{
	    if (j.find(gv->id) == j.end())
		{
			return;
		}
		if ((gv->flags & GUI_FLAGS_NO_LOAD) != 0)
		{
		    return;
		}
		switch (gv->type)
		{
		case GVAR_EVENT:
			    gv->val.v_bool = j[gv->id];
			break;
		case GVAR_BOOL:
			gv->val.v_bool = j[gv->id];
			break;
		case GVAR_FLOAT:
			gv->val.v_float = j[gv->id];
			break;
		case GVAR_UNORM:
			gv->val.v_float = j[gv->id];
			break;
		case GVAR_UINT:
			gv->val.v_uint = j[gv->id];
			break;
		case GVAR_INT:
			gv->val.v_int = j[gv->id];
			break;
		case GVAR_VEC3:
			gv->val.v_vec3[0] = j[gv->id][0];
			gv->val.v_vec3[1] = j[gv->id][1];
			gv->val.v_vec3[2] = j[gv->id][2];
			break;
		case GVAR_DISPLAY_VALUE:
			gv->val.v_float = j[gv->id];
			break;
		case GVAR_ENUM:
			gv->val.v_int  = j[gv->id];
			gv->val.v_uint = gv->val.v_int;
			break;
		case GVAR_UINT_RANGE:
			gv->val.v_uint = j[gv->id][0];
			break;
		case GVAR_FLOAT_RANGE:
			gv->val.v_float = j[gv->id][0];
			break;
		case GVAR_VEC3_RANGE:
			gv->val.v_vec3[0] = j[gv->id][0];
			gv->val.v_vec3[1] = j[gv->id][1];
			gv->val.v_vec3[2] = j[gv->id][2];
			break;
		default:
			//DEBUG_BREAK;
			break;
		}
	}

	void storeGVar(std::vector<GVar*> gvar, std::string path)
	{
		json j;
	    std::ofstream o(path);
		for (auto gv : gvar)
		{
		    writeGVarToJson(gv, j);
		}
		o << std::setw(4) << j << std::endl;
	}
	void loadGVar(std::vector<GVar*> gvar, std::string path)
	{
		json j;
		std::ifstream i(path);
		i >> j;
		for (auto gv : gvar)
		{
			readGVarFromJson(gv, j);
		}
	}



void configureGui_Default()
{
	VKA_ASSERT(!gState.guiConfigured);
	gState.guiConfigured = true;
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
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
	//ImGuiStyle *style          = &ImGui::GetStyle();
	//ImVec4 *colors             = style->Colors;
	//colors[ImGuiCol_MenuBarBg] = ImVec4(0.86f, 0.86f, 0.86f, 0.00f);
	//colors[ImGuiCol_Border]        = ImVec4(0.00f, 0.00f, 0.00f, 0.0f);
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



namespace gvar_gui
{
void addGVar(GVar *gv)
{
	std::stringstream ss;
	switch (gv->type)
	{
		case GVAR_EVENT:
			gv->val.v_bool = ImGui::Button(gv->id.c_str());
			break;
		case GVAR_BOOL:
			ImGui::Checkbox(gv->id.c_str(), &gv->val.v_bool);
			break;
		case GVAR_FLOAT:
			ImGui::InputScalar(gv->id.c_str(), ImGuiDataType_Float, &gv->val.v_float);
			break;
		case GVAR_UNORM:
			ImGui::SliderFloat(gv->id.c_str(), &gv->val.v_float, 0.0f, 1.0f);
			break;
		case GVAR_UINT:
			ImGui::InputScalar(gv->id.c_str(), ImGuiDataType_U32, &gv->val.v_uint);
			break;
		case GVAR_INT:
			ImGui::InputInt(gv->id.c_str(), &gv->val.v_int);
			break;
		case GVAR_VEC3:
			ImGui::InputFloat3(gv->id.c_str(), gv->val.v_vec3);
			break;
		case GVAR_DISPLAY_VALUE:
			ImGui::Text(gv->id.c_str(), gv->val.v_float);
			break;
		case GVAR_ENUM:
			for (size_t i = 0; i < gv->set.list.size(); i++)
			{
				ss << gv->set.list[i] << '\0';
			}
			ImGui::Combo(gv->id.c_str(), &gv->val.v_int, ss.str().c_str(), 5);
			gv->val.v_uint = gv->val.v_int;
			break;
		case GVAR_UINT_RANGE:
			ImGui::SliderScalar(gv->id.c_str(), ImGuiDataType_U32, &gv->val.v_uint, &gv->set.range.min.v_uint, &gv->set.range.max.v_uint);
			break;
		case GVAR_FLOAT_RANGE:
			ImGui::SliderScalar(gv->id.c_str(), ImGuiDataType_Float, &gv->val.v_float, &gv->set.range.min.v_float, &gv->set.range.max.v_float);
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
	bool categoryOpen = false;
	for (uint32_t i = 0; i < gvar.size(); i++)
	{
		GVar *gv = gvar[i];
		if (i == 0 || currentCategory != gv->sortId)
		{
			ImGui::SetNextItemOpen(true, ImGuiCond_Once);
			std::string name;
			if (categories.size() > gv->sortId)
			{
				name = categories[gv->sortId];
			}
			else
			{
				name = "Unknown_" + gv->sortId;
			}
			categoryOpen = ImGui::CollapsingHeader(name.c_str());
		}
		if (categoryOpen)
		{
			ImGui::PushItemWidth(ImGui::GetWindowSize().x * 0.3);
			addGVar(gv);
		}
		currentCategory = gvar[i]->sortId;
	}
	ImGui::End();
	ImGui::ShowDemoWindow();
}
}        // namespace gvar_gui

namespace gvar_gui_v2
{
void addGVar(GVar *gv)
{
	std::stringstream ss;
	switch (gv->type)
	{
		case GVAR_EVENT:
			gv->val.v_bool = ImGui::Button(gv->id.c_str());
			break;
		case GVAR_BOOL:
			ImGui::Checkbox(gv->id.c_str(), &gv->val.v_bool);
			break;
		case GVAR_FLOAT:
			ImGui::InputScalar(gv->id.c_str(), ImGuiDataType_Float, &gv->val.v_float);
			break;
		case GVAR_UNORM:
			ImGui::SliderFloat(gv->id.c_str(), &gv->val.v_float, 0.0f, 1.0f);
			break;
		case GVAR_UINT:
			ImGui::InputScalar(gv->id.c_str(), ImGuiDataType_U32, &gv->val.v_uint);
			break;
		case GVAR_INT:
			ImGui::InputInt(gv->id.c_str(), &gv->val.v_int);
			break;
		case GVAR_VEC3:
			ImGui::InputFloat3(gv->id.c_str(), gv->val.v_vec3);
			break;
		case GVAR_VEC3_RANGE:
			ImGui::SliderFloat3(gv->id.c_str(), gv->val.v_vec3, gv->set.range.min.v_float, gv->set.range.max.v_float);
			break;
		case GVAR_DISPLAY_VALUE:
			ImGui::Text(gv->id.c_str(), gv->val.v_float);
			// ImGui::Text(gv->path.c_str(),1.0);
			break;
		case GVAR_ENUM:
			for (size_t i = 0; i < gv->set.list.size(); i++)
			{
				ss << gv->set.list[i] << '\0';
			}
			ImGui::Combo(gv->id.c_str(), &gv->val.v_int, ss.str().c_str(), 5);
			gv->val.v_uint = gv->val.v_int;
			break;
		case GVAR_UINT_RANGE:
			ImGui::SliderScalar(gv->id.c_str(), ImGuiDataType_U32, &gv->val.v_uint, &gv->set.range.min.v_uint, &gv->set.range.max.v_uint);
			break;
		case GVAR_FLOAT_RANGE:
			ImGui::SliderScalar(gv->id.c_str(), ImGuiDataType_Float, &gv->val.v_float, &gv->set.range.min.v_float, &gv->set.range.max.v_float);
			break;
		case GVAR_TEXT_INPUT:
			ImGui::InputText(gv->id.c_str(), gv->val.v_char_array.data(), gv->val.v_char_array.size());
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
	int currentCategory = -1;
	bool     categoryOpen    = false;
	for (uint32_t i = 0; i < gvar.size(); i++)
	{
		GVar *gv = gvar[i];
		if (currentCategory < gv->sortId)
		{
			std::string name;
			if (categories.size() > gv->sortId)
			{
				name = categories[gv->sortId];
			}
			else
			{
				name = "Unknown_" + gv->sortId;
			}
			categoryOpen = ImGui::CollapsingHeader(name.c_str());
		}
		if (categoryOpen)
		{
			ImGui::PushItemWidth(ImGui::GetWindowSize().x * 0.3);
			addGVar(gv);
		}
		currentCategory = gvar[i]->sortId;
	}
	ImGui::End();
	/*ImGui::ShowDemoWindow();*/
}

void buildGui(std::vector<GVar *> gvar, std::vector<std::string> categories)
{
	std::stable_sort(gvar.begin(), gvar.end(), [](GVar *a, GVar *b) { return a->sortId < b->sortId; });
	int  currentCategory = -1;
	bool categoryOpen    = false;
	for (uint32_t i = 0; i < gvar.size(); i++)
	{
		GVar *gv = gvar[i];
		if (currentCategory < gv->sortId)
		{
			std::string name;
			if (categories.size() > gv->sortId)
			{
				name = categories[gv->sortId];
			}
			else
			{
				name = "Unknown_" + gv->sortId;
			}
			categoryOpen = ImGui::CollapsingHeader(name.c_str());
		}
		if (categoryOpen)
		{
			ImGui::PushItemWidth(ImGui::GetWindowSize().x * 0.5);
			addGVar(gv);
		}
		currentCategory = gvar[i]->sortId;
	}
	//ImGui::ShowDemoWindow();
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