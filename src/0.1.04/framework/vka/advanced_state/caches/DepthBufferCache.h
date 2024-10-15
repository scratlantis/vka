#pragma once
#include <vka/core/core_common.h>
#include <unordered_map>
namespace vka
{

class DepthBufferCache
{
	std::unordered_map<VkExtent2D_OP, Image> map;
	IResourcePool *pPool;
  public:
	DepthBufferCache(IResourcePool *pPool) :
	    pPool(pPool){};
	  void clear();
	  Image fetch(const VkExtent2D &key);
};
}