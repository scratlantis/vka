#include "BinaryLoadCache.h"

namespace vka
{
void BinaryLoadCache::clear()
{
	for (auto &pair : map)
	{
		pair.second->garbageCollect();
	}
	map.clear();
}
bool BinaryLoadCache::fetch(CmdBuffer cmdBuf, Buffer &buf, std::string path, VkBufferUsageFlagBits usage)
{
	if (map.find(path) != map.end())
	{
		buf = map[path];
		return true;
	}
	else
	{
		buf       = createBuffer(pPool, usage);
		map[path] = buf;
		try
		{
			std::vector<char> data = readFile(path);
			cmdWriteCopy(cmdBuf, buf, data.data(), data.size());
		}
		catch (std::runtime_error &e)
		{
			DEBUG_BREAK;
		}
		return false;
	}
}
}        // namespace vka