#include "HostCache.h"
#include <vka/globals.h>
namespace vka
{
HostCache::HostCache(IResourcePool *pCachedPool, IResourcePool *pLocalPool) :
    pCachedPool(pCachedPool), pLocalPool(pLocalPool)
{
	map = std::vector<std::unordered_map<Buffer, HostCacheEntry>>(gState.io.imageCount);
}
void HostCache::clear()
{
	for (auto &m : map)
	{
		for (auto &pair : m)
		{
			pair.second.hostBuffer->garbageCollect();
		}
	}
}
void HostCache::update(CmdBuffer cmdBuf)
{
	auto &currentMap = map[gState.frame->frameIndex];
	for (auto it = currentMap.begin(); it != currentMap.end();)
	{
		it->second.writeFlag = 0;
	}
	for (auto it = pCachedPool->getBuffersBegin(); it != pCachedPool->getBuffersEnd(); ++it)
	{
		Buffer deviceBuf = *it;

		if (deviceBuf->getSize() != 0)
		{
			if (currentMap.find(deviceBuf) != currentMap.end())
			{
				auto &entry     = currentMap[deviceBuf];
				entry.writeFlag = 1;
				entry.hostBuffer->changeSize(deviceBuf->getSize());
				entry.hostBuffer->recreate();
				cmdCopyBuffer(cmdBuf, deviceBuf, entry.hostBuffer);
			}
			else
			{
				HostCacheEntry entry;
				entry.hostBuffer = createBuffer(pLocalPool, VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_CPU_ONLY, deviceBuf->getSize());
				entry.writeFlag  = 1;
				cmdCopyBuffer(cmdBuf, deviceBuf, entry.hostBuffer);
				currentMap[deviceBuf] = entry;
			}
		}

	}
	for (auto it = currentMap.begin(); it != currentMap.end();)
	{
		if (it->second.writeFlag == 0)
		{
			it->second.hostBuffer->garbageCollect();
			it = currentMap.erase(it);
		}
		else
		{
			++it;
		}
	}
}
bool HostCache::fetch(Buffer deviceBuffer, Buffer &hostBuffer)
{
	auto &currentMap = map[gState.frame->frameIndex];
	if (currentMap.find(deviceBuffer) != currentMap.end())
	{
		hostBuffer = currentMap[deviceBuffer].hostBuffer;
		return true;
	}
	return false;
}
}        // namespace vka