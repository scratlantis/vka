#pragma once
#include <unordered_map>
#include <vka/core/core_common.h>

namespace vka
{
class ImageCache
{
	std::unordered_map<hash_t, Image> map;
	IResourcePool                     *pPool;

  public:
	ImageCache(IResourcePool *pPool) :
	    pPool(pPool){};
	void clear();
	bool fetch(CmdBuffer cmdBuf, Image &img, hash_t key, const VkImageCreateInfo &ci, VkImageLayout layout, glm::vec4 color);
};
}        // namespace vka