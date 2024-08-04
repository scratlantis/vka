#include "cmd_buffer_utility.h"
#include <vka/globals.h>
namespace vka
{
CmdBuffer createCmdBuffer(IResourcePool *pPool)
{
	CmdBuffer cmdBuffer = new CmdBuffer_R(pPool, CMD_BUF_CAPABILITY_MASK_UNIVERSAL, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 0);
	return cmdBuffer;
}

void executeImmediat(CmdBuffer cmdBuffer, VkQueue queue)
{
	cmdBuffer->end();
	submit(cmdBuffer->getHandle(), queue, {});
	vkDeviceWaitIdle(gState.device.logical);
}

void executeImmediat(CmdBuffer cmdBuffer)
{
	cmdBuffer->end();
	submit(cmdBuffer->getHandle(), gState.device.universalQueues[0], {});
	vkDeviceWaitIdle(gState.device.logical);
}

void submit(std::vector<CmdBuffer> cmdBufs, VkQueue queue, const SubmitSynchronizationInfo syncInfo)
{
	std::vector<VkCommandBuffer> vkCmdBufs;
	for (size_t i = 0; i < cmdBufs.size(); i++)
	{
		cmdBufs[i]->end();
		cmdBufs[i]->garbageCollect();
		vkCmdBufs.push_back(cmdBufs[i]->getHandle());
	}
	VkSubmitInfo submit{VK_STRUCTURE_TYPE_SUBMIT_INFO};
	submit.waitSemaphoreCount   = syncInfo.waitSemaphores.size();
	submit.pWaitSemaphores      = syncInfo.waitSemaphores.data();
	submit.pWaitDstStageMask    = syncInfo.waitDstStageMask.data();
	submit.signalSemaphoreCount = syncInfo.signalSemaphores.size();
	submit.pSignalSemaphores    = syncInfo.signalSemaphores.data();
	submit.pCommandBuffers      = vkCmdBufs.data();
	submit.commandBufferCount   = vkCmdBufs.size();
	VK_CHECK(vkQueueSubmit(queue, 1, &submit, syncInfo.signalFence));
}
}        // namespace vka