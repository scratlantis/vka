#ifndef PT_PLOT_INTERFACE_SMD_H
#define PT_PLOT_INTERFACE_SMD_H

#include "../../lib/pt_plot/interface_structs.glsl"

layout(binding = PT_PLOT_INTERFACE_SMD_BINDING_OFFSET) uniform PT_PLOT_OPTIONS
{
GLSLPtPlotOptions ptPlotOptions;
};
layout(binding = PT_PLOT_INTERFACE_SMD_BINDING_OFFSET + 1) writeonly buffer PT_PLOT
{
	GLSLPtPlot ptPlot;
};


GLSLPtPlotOptions ptPlotGetOptions()
{
	GLSLPtPlotOptions options = ptPlotOptions;
	return options;
}

void ptPlotSetPlot(GLSLPtPlot plot)
{
	ptPlot = plot;
}

#endif