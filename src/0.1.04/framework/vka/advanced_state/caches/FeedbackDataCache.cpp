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
	bool found = dataCache.fetch(buf, key);
	buf->addUsage(VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
	return found;
}

bool FeedbackDataCache::fetchHostData(void *&data, hash_t key, uint32_t* pSize)
{
	Buffer deviceBuf, hostBuffer;
	bool found = dataCache.fetch(deviceBuf, key);
	if (found && pHostCache->fetch(deviceBuf, hostBuffer))
	{
		data = hostBuffer->map();
		if (pSize)
		{
			*pSize = hostBuffer->getSize();
		}
		return true;
	}
	return false;
}


}        // namespace vka