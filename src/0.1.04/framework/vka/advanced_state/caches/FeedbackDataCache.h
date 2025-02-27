#pragma once
#include <unordered_map>
#include <vka/core/core_common.h>
#include "HostCache.h"
#include "DataCache.h"

namespace vka
{
class FeedbackDataCache
{
	HostCache *const                  pHostCache;
	DataCache                         dataCache;
  public:
	FeedbackDataCache(HostCache *pHostCache);
	void clear();
	bool fetch(Buffer &buf, hash_t key);
	bool FeedbackDataCache::fetchHostData(void *&data, hash_t key, uint32_t *pSize = nullptr);
};
}        // namespace vka