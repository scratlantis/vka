#ifndef PT_PLOT_INTERFACE_STUCTS
#define PT_PLOT_INTERFACE_STUCTS

struct GLSLPtPlotOptions
{
// General Options:
	uint bounce;
	uint histSampleCount;
// Select witch histograms to generate:
	uint writeTotalContribution;
	uint writeIndirectDir;


	uint writeIndirectT;
	uint writeIndirectWeight;
	uint writeDirectSampleContribution;
	uint padding1;
};

// init to minus 1
struct GLSLPtPlotDirectSample
{
	ivec3 totalContribution; // Combined 1D Histogram
	int domain;
};

// init to -1
struct GLSLPlotIndirectRay
{
	int dir; // 2D Histogram
	int t; // 1D Histogram
	int weight; // Combined 1D Histogram
	int padding0;
};

// Stores histogram ids of shader state data for the specified bounce
// init to -1
#define MAX_DIRECT_SAMPLE_COUNT_PER_RAY 2
struct GLSLPtPlot
{
	int totalContribution; // Combined 1D Histogram
	uint directSampleCount;
	uint padding1;
	uint padding2;

	GLSLPlotIndirectRay indirectRay;
	GLSLPtPlotDirectSample directSamples[MAX_DIRECT_SAMPLE_COUNT_PER_RAY];
};


#endif