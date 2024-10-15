#pragma once
#include "DescriptorSetLayout.h"
#include "../Resource.h"

#define VKA_PUSH_CONSTANT_RANGE_ALLIGNMENT 16U

namespace vka
{
class PipelineLayoutDefinition : public ResourceIdentifier
{
  public:
	std::vector<VkPushConstantRange_OP>        pcRanges;
	std::vector<DescriptorSetLayoutDefinition> descSetLayoutDef;

	bool   operator==(const ResourceIdentifier &other) const override;
	bool   operator==(const PipelineLayoutDefinition &other) const;
	hash_t hash() const override;

  protected:
};

class PipelineLayout_R : public Cachable_T<VkPipelineLayout>
{
  public:
	virtual void free() override;
	PipelineLayout_R(IResourceCache *pCache, PipelineLayoutDefinition const &definition);
};
}        // namespace vka

DECLARE_HASH(vka::PipelineLayoutDefinition, hash)