#include "UniqueResourceCache.h"

namespace vka
{
void UniqueResourceCache::clear()
{
	for (auto &pair : map)
	{
		pair.second->garbageCollect();
	}
	map.clear();
}
bool UniqueResourceCache::fetch(Resource* &pResource, std::string name)
{
	if (map.find(name) != map.end())
	{
		pResource = map[name];
		return true;
	}
	return false;
}

bool UniqueResourceCache::fetch(BLAS &blas, std::string name)
{
	Resource *pResource;
	if (fetch(pResource, name) && pResource->type() == RESOURCE_TYPE_BLAS)
	{
		blas      = static_cast<BLAS>(pResource);
		return true;
	}
	return false;
}

bool UniqueResourceCache::add(Resource *pResource, std::string name)
{
	if (map.find(name) != map.end())
	{
		return false;
	}
	map[name] = pResource;
	pResource->track(pPool);
	return true;
}
}        // namespace vka