#include "CmdBuffer.h"
#include <vka/globals.h>

namespace vka
{
void CmdBufferVK_R::free()
{
	vkFreeCommandBuffers(gState.device.logical, cmdPool, 1, &handle);
}
void CmdBuffer_R::createHandles(CmdBufferCapabitlityMask capability, VkCommandBufferUsageFlags usage, VkCommandBufferLevel level, uint32_t poolIdx)
{
	VkCommandPool cmdPool;
	switch (capability)
	{
		case CMD_BUF_CAPABILITY_MASK_TRANSFER:
			if (gState.cmdAlloc.createCmdBuffersCompute(poolIdx, level, 1, handle, cmdPool))
			{
				break;
			}
		case CMD_BUF_CAPABILITY_MASK_COMPUTE:
			if (gState.cmdAlloc.createCmdBuffersCompute(poolIdx, level, 1, handle, cmdPool))
			{
				break;
			}
		case CMD_BUF_CAPABILITY_MASK_UNIVERSAL:
			if (gState.cmdAlloc.createCmdBuffersUniversal(poolIdx, level, 1, handle, cmdPool))
			{
				break;
			}
		default:
			printVka("Unable to allocate command buffer with capability %d, from pool index %d!", capability, poolIdx);
			DEBUG_BREAK;
			break;
	}
	VkCommandBufferBeginInfo beginInfo{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
	beginInfo.flags = usage;
	VK_CHECK(vkBeginCommandBuffer(handle, &beginInfo));
	stateBits |= CMD_BUF_STATE_BITS_RECORDING;
	res = new CmdBufferVK_R(handle, cmdPool);
	res->track(getPool());
}
hash_t CmdBuffer_R::hash() const
{
	return (hash_t) this->handle << VKA_RESOURCE_META_DATA_HASH_SHIFT;
}
void CmdBuffer_R::free()
{
}

void CmdBuffer_R::track(IResourcePool *pPool)
{
	if (res)
	{
		res->track(pPool);
	}
	Resource::track(pPool);
}

std::vector<VkClearValue> CmdBufferState::getClearValues() const
{
	std::vector<VkClearValue> clearValues;
	for (size_t i = 0; i < this->clearValues.size(); i++)
	{
		clearValues.push_back(this->clearValues[i].value);
	}
	return clearValues;
}

}        // namespace vka