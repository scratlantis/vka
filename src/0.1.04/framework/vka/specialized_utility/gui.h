#pragma once
#include <vka/core/resources/cachable/RenderPass.h>
#include <vka/core/core_utility/types.h>
#include "GVar.h"
#include <implot.h>

namespace vka
{
void configureGui_Default();
void enableGui(RenderPassDefinition rpDef, uint32_t subpassIdx);
void enableGui();
void cmdRenderGui(CmdBuffer cmdBuf, Image target, float x, float y, float width, float height);
void cmdRenderGui(CmdBuffer cmdBuf, Image target);
void setGuiDimensions(Rect2D<float> rect);

void beginGui(std::string name, Rect2D<float> rect, uint32_t flags, bool *isOpen);

void endGui();
template <class T>
void addPlots(T *pPlots, uint32_t plotCount, void *pPlotData)
{
	for (uint32_t i = 0; i < plotCount; i++)
	{
		render_plot<T>{}(pPlots[i], pPlotData);
	}
}



//bool addShaderLog();
//void addPlot(hash_t key);


}        // namespace vka

