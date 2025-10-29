#pragma once
#include <vka/core/core_common.h>
#include <unordered_map>

namespace vka
{
	class DataCache
	{
		std::unordered_map<hash_t, Buffer> map;
		std::unordered_map<std::string, Buffer> strMap;
	    IResourcePool                     *pPool;
		public:
	    DataCache(IResourcePool *pPool) :
	        pPool(pPool){};
		void clear();
		bool fetch(Buffer &buf, hash_t key);
	    bool fetch(Buffer &buf, std::string key);
	};
}