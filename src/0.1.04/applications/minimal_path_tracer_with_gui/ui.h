#pragma once
#include "config.h"

const float         viewMargin = 0.001;
const float         topGuiHeight = 0.025;
const float         bottomGuiHeight = 0.01;
const float         leftGuiWidth = 0.4;

const Rect2D<float> viewDimensions = { leftGuiWidth, topGuiHeight, (1.0 - leftGuiWidth) - viewMargin, (1.0 - topGuiHeight - bottomGuiHeight) - viewMargin };
const Rect2D<float> leftGuiDimensions = { viewMargin, topGuiHeight, leftGuiWidth - 2.0 * viewMargin, (1.0 - topGuiHeight) - viewMargin };
const Rect2D<float> topGuiDimensions = { viewMargin, viewMargin, 1.0 - 2.0 * viewMargin, topGuiHeight - 2.0 * viewMargin };
const Rect2D<float> bottomGuiDimensions = { leftGuiWidth, 1.0 - bottomGuiHeight, (1.0 - leftGuiWidth) - viewMargin, bottomGuiHeight - viewMargin };

bool guiCatChanged(uint32_t setting, std::vector<bool> settingsChanged);
std::vector<bool> buildGui();
enum GuiGroupMenuBar
{
	GUI_CAT_MENU_BAR = 0x000F0,
};
const uint32_t GUI_INDEX_MASK = 0x0000F;
const uint32_t GUI_CAT_SHIFT = 4;

const std::vector<std::string> GuiGroupMenuBar_names = { "_menu" };
MASKED_ENUM_ADD_STRING_VALUES(GuiGroupMenuBar, GuiGroupMenuBar_names, GUI_INDEX_MASK)
ENUM_ADD_ITERATOR(GuiGroupMenuBar, GUI_CAT_MENU_BAR, GUI_CAT_MENU_BAR)


//############################################################################################################

enum GuiGroups
{
	GUI_GROUP_DEFAULT = 0,
};

enum GuiGroupDefault
{
	GUI_CAT_NOISE = 0x00000,
	GUI_CAT_GENERAL = 0x00001
};
const std::vector<std::string> GuiGroupDefault_names =
{
	"Noise",
	"General"
};

MASKED_ENUM_ADD_STRING_VALUES(GuiGroupDefault, GuiGroupDefault_names, GUI_INDEX_MASK)
ENUM_ADD_ITERATOR(GuiGroupDefault, GUI_CAT_NOISE, GUI_CAT_GENERAL)