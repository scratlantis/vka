#pragma once
#include "interface_structs.h"
#include <vka/advanced_utility/complex_commands.h>

namespace vka
{
namespace shader_plot
{
void bindYListPlot(ComputeCmd &cmd, Buffer plotBuf, Buffer plotDataBuf, Buffer plotCountBuf);
void cmdResetYListPlot(CmdBuffer cmdBuf, Buffer plotBuf, Buffer plotDataBuf, Buffer plotCountBuf, uint32_t maxPlotCount, uint32_t maxPlotValueCount);
void bindHistogram(ComputeCmd &cmd, Buffer histBuf, Buffer histDataBuf, Buffer histCountBuf);
void cmdResetHistogram(CmdBuffer cmdBuf, Buffer histBuf, Buffer histDataBuf, Buffer histCountBuf, uint32_t maxhistCount, uint32_t maxhistValueCount);
}

}        // namespace vka