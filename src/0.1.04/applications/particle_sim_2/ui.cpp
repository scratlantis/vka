#include "ui.h"

GVar gvar_menu{"Menu", 0U, GVAR_ENUM, GUI_CAT_MENU_BAR, std::vector<std::string>({"General", "Render"}), GVAR_FLAGS_V2};

bool guiCatChanged(uint32_t setting, std::vector<bool> settingsChanged)
{
	return gvar_menu.val.v_uint == setting >> GUI_CAT_SHIFT && settingsChanged[setting & GUI_INDEX_MASK];
}

std::vector<bool> buildGui()
{
	// Top
	beginGui("Top", topGuiDimensions, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar, nullptr);
	GVar::addAllToGui<GuiGroupMenuBar>(GUI_FLAGS_MENU_BAR);
	endGui();

	// Left
	std::vector<bool> changed;
	beginGui("Left", leftGuiDimensions, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize, nullptr);
	switch (gvar_menu.val.v_uint)
	{
	case GUI_GROUP_GENERAL:
		changed = GVar::addAllToGui<GuiGroupGeneral>(GUI_FLAGS_OPEN_NODES);
		break;
	case GUI_GROUP_RENDER:
		changed = GVar::addAllToGui<GuiGroupRender>(GUI_FLAGS_OPEN_NODES);
		break;
	}
	endGui();
	//ImPlot::ShowDemoWindow();
	//ImGui::ShowDemoWindow();

	// Shader Log Gui
	if (gState.shaderLog.size() > 0)
	{
		beginGui("Shader Log", viewDimensions, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize, nullptr);
		ImGui::TextWrapped(gState.shaderLog.c_str());
		endGui();
	}
	return changed;
}
