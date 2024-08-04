#include "FramebufferCache.h"
#include <vka/globals.h>
namespace vka
{
void FramebufferCache::clear()
{
	for (auto &it : cache)
	{
		vkDestroyFramebuffer(gState.device.logical, it.second, nullptr);
	}
	cache.clear();
}

VkFramebuffer FramebufferCache::fetch(VkRenderPass renderPass, std::vector<Image> images)
{
	hash_t                   hash = (hash_t) renderPass;
	std::vector<VkImageView> imageViews;
	for (size_t i = 0; i < images.size(); i++)
	{
		hash = hash HASHC images[i]->hash();
		imageViews.push_back(images[i]->getViewHandle());
	}
	auto          it = cache.find(hash);
	VkFramebuffer framebuffer;
	if (it == cache.end())
	{
		printVka("Framebuffer created.");
		VkFramebufferCreateInfo framebufferCI{VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
		framebufferCI.renderPass      = renderPass;
		framebufferCI.attachmentCount = imageViews.size();
		framebufferCI.pAttachments    = imageViews.data();
		framebufferCI.width           = images[0]->getExtent().width;
		framebufferCI.height          = images[0]->getExtent().height;
		framebufferCI.layers          = 1;
		vkCreateFramebuffer(gState.device.logical, &framebufferCI, nullptr, &framebuffer);
		cache.insert({hash, framebuffer});
	}
	else
	{
		framebuffer = it->second;
	}
	return framebuffer;
}
}        // namespace vka