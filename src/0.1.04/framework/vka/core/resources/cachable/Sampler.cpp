#include "Sampler.h"
#include <vka/globals.h>
namespace vka
{
hash_t SamplerDefinition::hash() const
{
	// printVka("Sampler Hashfunction called:%d\n", byteHashPtr(this));
	// clang-format off
	return flags                  
	HASHC magFilter              
	HASHC minFilter              
	HASHC mipmapMode             
	HASHC addressModeU           
	HASHC addressModeV           
	HASHC addressModeW           
	HASHC mipLodBias             
	HASHC anisotropyEnable       
	HASHC maxAnisotropy          
	HASHC compareEnable          
	HASHC compareOp              
	HASHC minLod                 
	HASHC maxLod                 
	HASHC borderColor            
	HASHC unnormalizedCoordinates;
	// clang-format on
}

DEFINE_EQUALS_OVERLOAD(SamplerDefinition, ResourceIdentifier)

bool SamplerDefinition::operator==(const SamplerDefinition &other) const
{
	// clang-format off
	return magFilter			== other.magFilter         
	&& minFilter				== other.minFilter         
	&& mipmapMode				== other.mipmapMode
	&& addressModeU				== other.addressModeU
	&& addressModeV				== other.addressModeV
	&& addressModeW				== other.addressModeW
	&& mipLodBias				== other.mipLodBias
	&& anisotropyEnable			== other.anisotropyEnable
	&& maxAnisotropy			== other.maxAnisotropy
	&& compareEnable			== other.compareEnable
	&& compareOp				== other.compareOp
	&& minLod					== other.minLod
	&& maxLod					== other.maxLod
	&& borderColor				== other.borderColor
	&& unnormalizedCoordinates	== other.unnormalizedCoordinates;
	// clang-format on
}

void Sampler_R::free()
{
	vkDestroySampler(gState.device.logical, handle, nullptr);
}

Sampler_R::Sampler_R(IResourceCache *pCache, SamplerDefinition const &definition) :
    Cachable_T<VkSampler>(pCache)
{
	VK_CHECK(vkCreateSampler(gState.device.logical, &definition, nullptr, &handle));
}

}        // namespace vka