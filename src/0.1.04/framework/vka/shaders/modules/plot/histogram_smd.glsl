#ifndef HISTOGRAM_SMD_H
#define HISTOGRAM_SMD_H
#extension GL_EXT_debug_printf : enable

#include "../../lib/compute_shader_base.glsl"
#include "../../lib/plot/interface_structs.glsl"

layout(binding = HISTOGRAM_SMD_BINDING_OFFSET) buffer HISTOGRAMS
{
	 GLSLHistogram hist[];
};
layout(binding = HISTOGRAM_SMD_BINDING_OFFSET + 1) buffer HISTOGRAM_DATA
{
	 float hist_data[];
};
layout(binding = HISTOGRAM_SMD_BINDING_OFFSET + 2) buffer HISTOGRAM_COUNT
{
	 uint hist_count;
};

void initHist(uint binCount, uint valueCount, float rMin, float rMax)
{
	//debugPrintfEXT("Init Hist!\n");
	hist_count = 1;
	hist[0].count = valueCount;
	hist[0].offset = 0;
	hist[0].invocationID = invocationID();
	hist[0].plotID = hist_count;
	hist[0].dataType = PLOT_DATA_TYPE_FLOAT;
	hist[0].bins = vec2(binCount);
	hist[0].rMin = vec2(rMin);
	hist[0].rMax = vec2(rMax);
	hist[0].dataDim = 1;
	hist[0].histDim = 1;
}

void nextHist()
{
	hist_count++;
	hist[hist_count - 1] = hist[hist_count - 2];
	hist[hist_count - 1].plotID = hist_count;
	hist[hist_count - 1].offset += hist[hist_count - 1].count * hist[hist_count - 1].dataDim;
}

void setHistDimensions(uint histDim)
{
	hist[hist_count - 1].histDim = histDim;
}

void setHistValue(float val, uint sampleIdx)
{
	uint idx = hist[hist_count - 1].offset + sampleIdx % hist[hist_count - 1].count;
	hist_data[idx] = val;
	hist[hist_count - 1].dataDim = 1;
}

void setHistValue(vec2 val, uint sampleIdx)
{
	uint idx = hist[hist_count - 1].offset + sampleIdx % hist[hist_count - 1].count;
	hist_data[idx] = val.x;
	hist_data[idx + hist[hist_count - 1].count] = val.y;
	hist[hist_count - 1].dataDim = 2;
}

void setHistValue(vec3 val, uint sampleIdx)
{
	uint idx = hist[hist_count - 1].offset + sampleIdx % hist[hist_count - 1].count;
	hist_data[idx] = val.x;
	hist_data[idx + hist[hist_count - 1].count] = val.y;
	hist_data[idx + 2 * hist[hist_count - 1].count] = val.z;
	hist[hist_count - 1].dataDim = 3;
}

#endif