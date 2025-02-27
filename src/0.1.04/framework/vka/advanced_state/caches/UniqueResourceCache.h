#pragma once
#include <unordered_map>
#include <vka/core/core_common.h>

namespace vka
{
class UniqueResourceCache
{
	std::unordered_map<std::string, Resource*> map;
	IResourcePool                     *pPool;

  public:
	UniqueResourceCache(IResourcePool *pPool) :
	    pPool(pPool){};
	void clear();
	bool fetch(Resource *&pResource, std::string name);
	bool fetch(BLAS &blas, std::string name);
	bool add(Resource *pResource, std::string name);
};
}        // namespace vka