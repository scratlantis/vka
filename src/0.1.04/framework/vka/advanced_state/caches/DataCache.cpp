#include "DataCache.h"

namespace vka
{
void DataCache::clear()
{
	for (auto &pair : map)
	{
		pair.second->garbageCollect();
	}
	map.clear();
}
bool DataCache::fetch(Buffer &buf, hash_t key)
{
	if (map.find(key) != map.end())
	{
		buf = map[key];
		return true;
	}
	else
	{
		buf = createBuffer(pPool, 0);
		map[key] = buf;
		return false;
	}
}

}        // namespace vka