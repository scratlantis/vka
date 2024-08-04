#pragma once
#include "../Resource.h"
#include <vka/core/stateless/vk_types/misc.h>
namespace vka
{
class RenderPassDefinition : public ResourceIdentifier
{
  public:
	std::vector<VkAttachmentDescription_OP> attachmentDescriptions;
	std::vector<SubpassDescription>			subpassDescriptions;
	std::vector<VkSubpassDependency_OP>     subpassDependencies;
	uint32_t                                currentSubpass = 0;
	bool                                    operator==(const ResourceIdentifier &other) const override;
	bool                                    operator==(const RenderPassDefinition &other) const;
	hash_t                                  hash() const override;
	bool 								    joinable(const RenderPassDefinition &other) const;

  protected:
};

class RenderPass_R : public Cachable_T<VkRenderPass>
{
  public:
	virtual void free() override;
	RenderPass_R(IResourceCache *pCache, RenderPassDefinition const &def);
};
}        // namespace vka
DECLARE_HASH(vka::RenderPassDefinition, hash)