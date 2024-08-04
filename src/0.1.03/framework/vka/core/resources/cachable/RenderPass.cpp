#include "RenderPass.h"
#include <vka/globals.h>
namespace vka
{
hash_t RenderPassDefinition::hash() const
{
	// clang-format off
	return hashVector(attachmentDescriptions)
	    HASHC hashVector(subpassDescriptions)
	    HASHC hashVector(subpassDependencies);
	// clang-format on
}

bool RenderPassDefinition::joinable(const RenderPassDefinition &previous) const
{
	if (*this == previous)
	{
		return true;
	}
	else if (
		// clang-format off
		subpassDescriptions.size() == 1
		&& cmpVector(subpassDescriptions, previous.subpassDescriptions)
		&& cmpVector(subpassDependencies, previous.subpassDependencies)
		&& attachmentDescriptions.size() == previous.attachmentDescriptions.size()
		// clang-format on
	)
	{
		bool isJoinable = true;
		for (size_t i = 0; i < attachmentDescriptions.size(); i++)
		{
			isJoinable = isJoinable && attachmentDescriptions[i].joinable(previous.attachmentDescriptions[i]);
		}
		return isJoinable;
	}
	return false;
}

DEFINE_EQUALS_OVERLOAD(RenderPassDefinition, ResourceIdentifier)

bool RenderPassDefinition::operator==(const RenderPassDefinition &other) const
{
	// clang-format off
	return cmpVector(attachmentDescriptions, other.attachmentDescriptions)
		&& cmpVector(subpassDescriptions, other.subpassDescriptions)
	    && cmpVector(subpassDependencies, other.subpassDependencies);
	// clang-format on
}

void RenderPass_R::free()
{
	vkDestroyRenderPass(gState.device.logical, handle, nullptr);
}

RenderPass_R::RenderPass_R(IResourceCache *pCache, RenderPassDefinition const &def) :
    Cachable_T<VkRenderPass>(pCache)
{
	VkRenderPassCreateInfo renderPassCreateInfo{VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
	renderPassCreateInfo.attachmentCount = VKA_COUNT(def.attachmentDescriptions);
	renderPassCreateInfo.pAttachments    = def.attachmentDescriptions.data();
	renderPassCreateInfo.subpassCount    = VKA_COUNT(def.subpassDescriptions);
	std::vector<VkSubpassDescription> vkSubpassDescriptions(def.subpassDescriptions.size());
	for (size_t i = 0; i < vkSubpassDescriptions.size(); i++)
	{
		vkSubpassDescriptions[i] = def.subpassDescriptions[i].getVulkanStruct();
	}
	renderPassCreateInfo.pSubpasses      = vkSubpassDescriptions.data();
	renderPassCreateInfo.dependencyCount = VKA_COUNT(def.subpassDependencies);
	renderPassCreateInfo.pDependencies   = def.subpassDependencies.data();
	VK_CHECK(vkCreateRenderPass(gState.device.logical, &renderPassCreateInfo, nullptr, &handle));
}

}        // namespace vka