#include "submodule.h"

namespace vka
{
namespace shader_plot
{
void bindYListPlot(ComputeCmd &cmd, Buffer plotBuf, Buffer plotDataBuf, Buffer plotCountBuf)
{
	cmd.pushSubmodule(cVkaShaderModulePath + "plot/y_list_plot_smd.glsl");
	cmd.pushDescriptor(plotBuf, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
	cmd.pushDescriptor(plotDataBuf, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
	cmd.pushDescriptor(plotCountBuf, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
}
void cmdResetYListPlot(CmdBuffer cmdBuf, Buffer plotBuf, Buffer plotDataBuf, Buffer plotCountBuf, uint32_t maxPlotCount, uint32_t maxPlotValueCount)
{
	plotBuf->addUsage(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
	plotDataBuf->addUsage(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
	plotCountBuf->addUsage(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);

	plotBuf->changeSize(maxPlotCount * sizeof(GLSLYListPlot));
	plotDataBuf->changeSize(maxPlotValueCount * sizeof(float));
	plotCountBuf->changeSize(sizeof(uint32_t));

	plotBuf->recreate();
	plotDataBuf->recreate();
	plotCountBuf->recreate();

	cmdZeroBuffer(cmdBuf, plotBuf);
	cmdZeroBuffer(cmdBuf, plotDataBuf);
	cmdZeroBuffer(cmdBuf, plotCountBuf);
}

void bindHistogram(ComputeCmd &cmd, Buffer histBuf, Buffer histDataBuf, Buffer histCountBuf)
{
	cmd.pushSubmodule(cVkaShaderModulePath + "plot/histogram_smd.glsl");
	cmd.pushDescriptor(histBuf, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
	cmd.pushDescriptor(histDataBuf, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
	cmd.pushDescriptor(histCountBuf, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
}

void cmdResetHistogram(CmdBuffer cmdBuf, Buffer histBuf, Buffer histDataBuf, Buffer histCountBuf, uint32_t maxhistCount, uint32_t maxhistValueCount)
{
	histBuf->addUsage(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
	histDataBuf->addUsage(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
	histCountBuf->addUsage(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);

	histBuf->changeSize(maxhistCount * sizeof(GLSLHistogram));
	histDataBuf->changeSize(maxhistValueCount * sizeof(float));
	histCountBuf->changeSize(sizeof(uint32_t));

	histBuf->recreate();
	histDataBuf->recreate();
	histCountBuf->recreate();

	cmdZeroBuffer(cmdBuf, histBuf);
	cmdFillBuffer<float>(cmdBuf, histDataBuf, std::numeric_limits<float>::max());
	cmdZeroBuffer(cmdBuf, histCountBuf);
}
}        // namespace shader_plot
}		// namespace vka