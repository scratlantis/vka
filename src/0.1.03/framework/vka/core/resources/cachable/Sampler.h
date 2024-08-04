#pragma once
#include "../Resource.h"
#include <vka/core/stateless/vk_types/default_values.h>
namespace vka
{
class SamplerDefinition : public ResourceIdentifier, public SamplerCreateInfo_Default
{
  public:
	SamplerDefinition() :
	    SamplerCreateInfo_Default(){};
	bool   operator==(const ResourceIdentifier &other) const override;
	bool   operator==(const SamplerDefinition &other) const;
	hash_t hash() const override;

  protected:
};

class Sampler_R : public Cachable_T<VkSampler>
{
  public:
	virtual void free() override;
	Sampler_R(IResourceCache *pCache, SamplerDefinition const &definition);
};
}        // namespace vka
DECLARE_HASH(vka::SamplerDefinition, hash)