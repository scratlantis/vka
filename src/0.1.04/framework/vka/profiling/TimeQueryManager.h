#pragma once
#include <vka/core/core_common.h>

namespace vka
{
class TimeQueryManager
{
	enum class TQState
	{
		TQ_STATE_READY,
		TQ_STATE_RECORDING,
		TQ_STATE_NOT_READY,
	};

  public:
	TimeQueryManager(IResourcePool *pPool, uint32_t queryCount);
	void cmdReset(CmdBuffer cmdBuffer);
	TimeQueryManager() = default;
	~TimeQueryManager();

	void cmdResetQueryPool(CmdBuffer cmdBuffer);
	void startTiming(CmdBuffer cmdBuffer, uint32_t queryID, VkPipelineStageFlagBits stage);

	void endTiming(CmdBuffer cmdBuffer, uint32_t queryID, VkPipelineStageFlagBits stage);

	bool updateTimings();
	void garbageCollect();

	std::vector<float> timings;

  private:
	Resource *queryPoolRes = nullptr;
	VkQueryPool           queryPool;
	std::vector<uint64_t> queryResults;
	TQState               state;
	bool                  firstUse;
	bool                  mHasTimings = false;
};
}        // namespace vka