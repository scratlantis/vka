#pragma once
#include <vka/advanced_utility/complex_commands.h>
namespace vka
{
	void cmdRadixSort(CmdBuffer cmdBuf, Buffer buffer, Buffer pingPongBuf = nullptr, Buffer histogramBuf = nullptr);
	void cmdRadixSortPermutation(CmdBuffer cmdBuf, Buffer buffer, Buffer permutationBuffer, Buffer pingPongBuf = nullptr, Buffer permutationPingPongBuf = nullptr, Buffer histogramBuf = nullptr);
}