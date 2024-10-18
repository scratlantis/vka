#pragma once
#include "ImageCache.h"
#include <vka/specialized_utility/draw_2D.h>

namespace vka
{
void ImageCache::clear()
{
	for (auto &pair : map)
	{
		pair.second->garbageCollect();
	}
	map.clear();
}

bool ImageCache::fetch(CmdBuffer cmdBuf, Image &img, hash_t key, const VkImageCreateInfo &ci, VkImageLayout layout, glm::vec4 color)
{
	bool transitionPerformed = false;
	if (map.find(key) != map.end())
	{
		img = map[key];
		transitionPerformed = img->getLayout() != layout;
		cmdTransitionLayout(cmdBuf, img, layout); // is conditional anyway so no need to check if transition is needed
	}
	else
	{
		img = new Image_R(pPool, ci, true);
		img->createHandles();
		map[key] = img;
		cmdFill(cmdBuf, img, layout, color); // fill color and transition layout
		transitionPerformed = true;
	}
	return transitionPerformed;
}

}        // namespace vka