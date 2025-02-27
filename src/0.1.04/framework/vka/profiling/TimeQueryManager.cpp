#include "TimeQueryManager.h"
#include <vka/globals.h>
namespace vka
{
TimeQueryManager::TimeQueryManager(IResourcePool *pPool, uint32_t queryCount)
{
	timings.resize(queryCount);
	for (size_t i = 0; i < queryCount; i++)
	{
		timings[i] = 0.0f;
	}
	queryResults.resize(queryCount * 2);
	VkQueryPoolCreateInfo queryPoolCI{VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO};
	queryPoolCI.queryType  = VK_QUERY_TYPE_TIMESTAMP;
	queryPoolCI.queryCount = queryResults.size();
	queryPool              = createQueryPool(pPool, queryPoolCI, queryPoolRes);
	vkResetQueryPool(gState.device.logical, queryPool, 0, queryResults.size());
	state                  = TQState::TQ_STATE_RECORDING;
	mHasTimings    = false;
}


void TimeQueryManager::cmdReset(CmdBuffer cmdBuffer)
{
	cmdClearState(cmdBuffer);
	vkCmdResetQueryPool(cmdBuffer->getHandle(), queryPool, 0, queryResults.size());
	for (size_t i = 0; i < timings.size(); i++)
	{
		timings[i] = 0.0f;
	}
	state = TQState::TQ_STATE_READY;
	mHasTimings = false;
}


TimeQueryManager::~TimeQueryManager()
{
}

void TimeQueryManager::startTiming(CmdBuffer cmdBuffer, uint32_t queryID, VkPipelineStageFlagBits stage)
{
	if (state == TQState::TQ_STATE_READY)
	{
		cmdClearState(cmdBuffer);
		vkCmdResetQueryPool(cmdBuffer->getHandle(), queryPool, 0, queryResults.size());
		state = TQState::TQ_STATE_RECORDING;
	}

	if (state == TQState::TQ_STATE_RECORDING)
	{
		vkCmdWriteTimestamp(cmdBuffer->getHandle(), stage, queryPool, queryID * 2);
	}
}

void TimeQueryManager::endTiming(CmdBuffer cmdBuffer, uint32_t queryID, VkPipelineStageFlagBits stage)
{
	if (state == TQState::TQ_STATE_RECORDING)
	{
		vkCmdWriteTimestamp(cmdBuffer->getHandle(), stage, queryPool, queryID * 2 + 1);
	}
}

bool TimeQueryManager::updateTimings()
{
	VkResult result = vkGetQueryPoolResults(
	    gState.device.logical,
	    queryPool,
	    0,
	    queryResults.size(),
	    queryResults.size() * sizeof(uint64_t),
	    queryResults.data(),
	    static_cast<VkDeviceSize>(sizeof(uint64_t)),
	    VK_QUERY_RESULT_64_BIT);
	VkPhysicalDeviceProperties physicalDeviceProps{};
	vkGetPhysicalDeviceProperties(gState.device.physical, &physicalDeviceProps);
	if (result != VK_NOT_READY)
	{
		for (size_t i = 0; i < timings.size(); i++)
		{
			uint64_t t_ns      = (queryResults[2 * i + 1] - queryResults[2 * i]) * physicalDeviceProps.limits.timestampPeriod;
			float    t_float   = static_cast<float>(t_ns) / 1000000.f;
			timings[i] = t_float;
		}
		mHasTimings = true;
		state       = TQState::TQ_STATE_READY;
	}
	else if (result == VK_NOT_READY && state == TQState::TQ_STATE_RECORDING)
	{
		state = TQState::TQ_STATE_NOT_READY;
	}
	return mHasTimings;
}
void TimeQueryManager::garbageCollect()
{
	queryPoolRes->garbageCollect();
}
}        // namespace vka
