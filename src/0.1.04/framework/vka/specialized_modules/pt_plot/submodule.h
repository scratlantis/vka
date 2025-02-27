#pragma once
#include "interface_structs.h"
#include <vka/advanced_utility/complex_commands.h>

namespace vka
{
namespace pt_plot
{

struct PtPlotOptions
{
	uint bounce;
	uint histSampleCount;

	bool writeTotalContribution;
	bool writeIndirectDir;
	bool writeIndirectT;
	bool writeIndirectWeight;
	bool writeDirectSampleContribution;
};

void bindPtPlot(ComputeCmd &cmd, Buffer optionsBuf, Buffer plotBuf);
void cmdConfigurePtPlot(CmdBuffer cmdBuf, Buffer optionsBuf, Buffer plotBuf, PtPlotOptions options);

}
}        // namespace vka