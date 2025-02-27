#ifndef PLOT_INTERFACE_STRUCTS_H
#define PLOT_INTERFACE_STRUCTS_H

#define PLOT_FLAG_ACCUMULATE 1
#define PLOT_DATA_TYPE_FLOAT 0
#define PLOT_DATA_TYPE_UINT 1
struct GLSLYListPlot
{
// general fields
	uint count;
	uint offset;
	uint invocationID;
	uint plotID;

	uint dataType;
	float stride;
	uint padding[2];
};

struct GLSLHistogram
{
	uint count;
	uint offset;
	uint invocationID;
	uint plotID;

	vec2 rMin;
	vec2 rMax;

	vec2 bins;
	uint dataType;
	uint dataDim; // must be multiple of histDim

	uint histDim; // 1 or 2
	uint padding[3];
};

#endif
