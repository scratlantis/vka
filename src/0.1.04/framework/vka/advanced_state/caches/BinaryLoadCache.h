#pragma once
#include <unordered_map>
#include <vka/core/core_common.h>

namespace vka
{
class BinaryLoadCache
{
	std::unordered_map<std::string, Buffer> map;
	IResourcePool                    *pPool;

  public:
	BinaryLoadCache(IResourcePool *pPool) :
	    pPool(pPool){};
	void clear();
	bool fetch(CmdBuffer cmdBuf, Buffer &buf, std::string path, VkBufferUsageFlagBits usage);
};
}        // namespace vka