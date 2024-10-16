#pragma once
#include "types.h"
#include <vka/core/resources/IResourcePool.h>

namespace vka
{
CmdBuffer createCmdBuffer(IResourcePool *pPool);
void      executeImmediat(CmdBuffer cmdBuffer, VkQueue queue);
void      executeImmediat(CmdBuffer cmdBuffer);
void      submit(std::vector<CmdBuffer> cmdBufs, VkQueue queue, const SubmitSynchronizationInfo syncInfo);
}		// namespace vka