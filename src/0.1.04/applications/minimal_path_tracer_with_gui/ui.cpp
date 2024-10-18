#include "ui.h"


std::vector<bool> buildGui()
{
	// GVar Gui
	std::vector<bool> changed;
	beginGui("Settings", leftGuiDimensions, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize, nullptr);
	for (GuiCatergories cat = eIterator_begin<GuiCatergories>()(); cat != eIterator_end<GuiCatergories>()(); cat = eIterator_next<GuiCatergories>()(cat))
	{
		std::string         name  = eString_val<GuiCatergories>()(cat);
		std::vector<GVar *> gvars = GVar::filterSortID(GVar::getAll(), cat);
		changed.push_back(false);
		//ImGui::PushItemWidth(ImGui::GetWindowSize().x * 0.3);
		if (ImGui::CollapsingHeader(name.c_str()))
		{
			changed.back() = GVar::addToGui(gvars, name);
		}
	}
	endGui();
	// Shader Log Gui
	if (gState.shaderLog.size() > 0)
	{
		beginGui("Shader Log", viewDimensions, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize, nullptr);
		ImGui::TextWrapped(gState.shaderLog.c_str());
		endGui();
	}
	return changed;
}