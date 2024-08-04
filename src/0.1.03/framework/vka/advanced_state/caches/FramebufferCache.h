#pragma once
#include <unordered_map>
#include <vka/core/core_utility/types.h>
namespace vka
{

class FramebufferCache
{
	std::unordered_map<hash_t, VkFramebuffer> cache;

  public:
	void          clear();
	VkFramebuffer fetch(VkRenderPass renderPass, std::vector<Image> images);
};
}        // namespace vka