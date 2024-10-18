#pragma once
#include <vka/core/core_common.h>
namespace vka
{
	struct HostCacheEntry
	{
		Buffer hostBuffer;
	    uint32_t writeFlag;
	};
class HostCache
{
	std::vector<std::unordered_map<Buffer, HostCacheEntry>> map;

  public:
	IResourcePool *const pCachedPool;
	IResourcePool *const pLocalPool;
	HostCache(IResourcePool *pCachedPool, IResourcePool *pLocalPool);
	void clear();
	void update(CmdBuffer cmdBuf);
	bool fetch(Buffer deviceBuffer, Buffer &hostBuffer);
};
}        // namespace vka