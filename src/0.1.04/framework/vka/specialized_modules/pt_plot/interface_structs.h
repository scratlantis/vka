#pragma once
#include <implot.h>
#include <vka/advanced_utility/complex_commands.h>
#include <vka/specialized_modules/plot/interface_structs.h>

using namespace glm;
namespace vka
{
namespace pt_plot
{
#include <vka/shaders/lib/pt_plot/interface_structs.glsl>
}

template <typename T>
struct render_plot_family;

template <>
struct render_plot_family<pt_plot::GLSLPtPlot>
{
	void operator()(const pt_plot::GLSLPtPlot &plotFamily, void *pPlots, void *pPlotData )
	{
		shader_plot::GLSLHistogram *pHist = static_cast<shader_plot::GLSLHistogram *>(pPlots);
		if (plotFamily.totalContribution != -1)
		{
			uint32_t id = plotFamily.totalContribution;
			render_plot<shader_plot::GLSLHistogram>{}(pHist[id], pPlotData);
		}
		if (plotFamily.indirectRay.dir != -1)
		{
			uint32_t id = plotFamily.indirectRay.dir;
			render_plot<shader_plot::GLSLHistogram>{}(pHist[id], pPlotData);
		}
		if (plotFamily.indirectRay.t != -1)
		{
			uint32_t id = plotFamily.indirectRay.t;
			render_plot<shader_plot::GLSLHistogram>{}(pHist[id], pPlotData);
		}
		if (plotFamily.indirectRay.weight != -1)
		{
			uint32_t id = plotFamily.indirectRay.weight;
			render_plot<shader_plot::GLSLHistogram>{}(pHist[id], pPlotData);
		}

	}
};

}        // namespace vka