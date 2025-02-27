#include "submodule.h"

namespace vka
{
namespace pt_plot
{

void bindPtPlot(ComputeCmd &cmd, Buffer optionsBuf, Buffer plotBuf)
{
	cmd.pushSubmodule(cVkaShaderModulePath + "pt_plot/pt_plot_interface_smd.glsl");
	cmd.pushDescriptor(optionsBuf, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
	cmd.pushDescriptor(plotBuf, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
}

void cmdConfigurePtPlot(CmdBuffer cmdBuf, Buffer optionsBuf, Buffer plotBuf, PtPlotOptions options)
{
	optionsBuf->addUsage(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
	GLSLPtPlotOptions glslOptions{};
	glslOptions.bounce                        = options.bounce;
	glslOptions.histSampleCount               = options.histSampleCount;
	glslOptions.writeTotalContribution        = options.writeTotalContribution ? 1U : 0U;
	glslOptions.writeIndirectDir              = options.writeIndirectDir ? 1U : 0U;
	glslOptions.writeIndirectT                = options.writeIndirectT ? 1U : 0U;
	glslOptions.writeIndirectWeight           = options.writeIndirectWeight ? 1U : 0U;
	glslOptions.writeDirectSampleContribution = options.writeDirectSampleContribution ? 1U : 0U;
	cmdWriteCopy(cmdBuf, optionsBuf, &glslOptions, sizeof(GLSLPtPlotOptions));

	plotBuf->addUsage(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
	plotBuf->changeSize(sizeof(GLSLPtPlot));
	plotBuf->recreate();
	cmdFillBuffer<int>(cmdBuf, plotBuf, -1);
}

}        // namespace pt_plot
}        // namespace vka