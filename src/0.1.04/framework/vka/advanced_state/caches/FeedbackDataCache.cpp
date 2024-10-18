#include "FeedbackDataCache.h"

namespace vka
{
FeedbackDataCache::FeedbackDataCache(HostCache *pHostCache) :
    pHostCache(pHostCache), dataCache(pHostCache->pCachedPool) {}

void FeedbackDataCache::clear()
{
	dataCache.clear();
}
bool FeedbackDataCache::fetch(Buffer &buf, hash_t key)
{
	return dataCache.fetch(buf, key);
}

bool FeedbackDataCache::fetchHostData(void *&data, uint32_t &size, hash_t key)
{
	Buffer deviceBuf, hostBuffer;
	if (dataCache.fetch(deviceBuf, key) && pHostCache->fetch(deviceBuf, hostBuffer))
	{
		data = hostBuffer->map();
		size = hostBuffer->getSize();
		return true;
	}
	return false;
}


}        // namespace vka