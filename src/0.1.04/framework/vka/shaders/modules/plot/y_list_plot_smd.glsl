#ifndef Y_LIST_PLOT_SMD_H
#define Y_LIST_PLOT_SMD_H
#extension GL_EXT_debug_printf : enable

#include "../../lib/compute_shader_base.glsl"
#include "../../lib/plot/interface_structs.glsl"

layout(binding = Y_LIST_PLOT_SMD_BINDING_OFFSET) buffer PLOTS
{
	GLSLYListPlot plots[];
};
layout(binding = Y_LIST_PLOT_SMD_BINDING_OFFSET + 1) buffer PLOT_DATA
{
	float plot_data[];
};
layout(binding = Y_LIST_PLOT_SMD_BINDING_OFFSET + 2) buffer PLOT_COUNT
{
	uint plot_count;
};

void initPlot(float stride)
{
	debugPrintfEXT("Init plot!\n");
	plot_count = 1;
	plots[0].count = 0;
	plots[0].offset = 0;
	plots[0].invocationID = invocationID();
	plots[0].plotID = plot_count;
	plots[0].dataType = PLOT_DATA_TYPE_FLOAT;
	plots[0].stride = stride;

}

void nextPlot()
{
	plot_count++;
	plots[plot_count - 1] = plots[plot_count - 2];
	plots[plot_count - 1].plotID = plot_count;
	plots[plot_count - 1].offset += plots[plot_count - 1].count;
	plots[plot_count - 1].count = 0;
}

void setPlotStride(float stride)
{
	plots[plot_count - 1].stride = stride;
}

void addPlotValue(float value)
{
	plot_data[plots[plot_count - 1].offset + plots[plot_count - 1].count] = value;
	plots[plot_count - 1].count++;
}

#endif