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
	bool fetchHostData(void *&data, uint32_t &size, hash_t key);
};
}        // namespace vka