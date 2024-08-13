#include "DepthBufferCache.h"
namespace vka
{
void DepthBufferCache::clear()
{
	for (auto &pair : map)
	{
		pair.second->garbageCollect();
	}
	map.clear();
}

Image vka::DepthBufferCache::fetch(const VkExtent2D &extent)
{
	auto it = map.find(extent);
	if (it == map.end())
	{
		Image image = createImage(pPool, VK_FORMAT_D32_SFLOAT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, extent);
		map.insert({extent, image});
		return image;
	}
	else
	{
		return it->second;
	}
}
}        // namespace vka
