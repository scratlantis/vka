#include "ResourceCache.h"

namespace vka
{
VkDescriptorSetLayout ResourceCache::fetch(DescriptorSetLayoutDefinition const &rID)
{
	VkDescriptorSetLayout handle;
	fetch(descSetLayouts, rID, handle);
	return handle;
}
VkPipelineLayout ResourceCache::fetch(PipelineLayoutDefinition const &rID)
{
	VkPipelineLayout handle;
	fetch(pipelineLayouts, rID, handle);
	return handle;
}
VkShaderModule ResourceCache::fetch(ShaderDefinition const &rID)
{
	VkShaderModule handle;
	fetch(shaders, rID, handle);
	return handle;
}
VkRenderPass ResourceCache::fetch(RenderPassDefinition const &rID)
{
	VkRenderPass handle;
	fetch(renderPasses, rID, handle);
	return handle;
}
VkPipeline ResourceCache::fetch(RasterizationPipelineDefinition const &rID)
{
	VkPipeline handle;
	fetch(rasterizationPipelines, rID, handle);
	return handle;
}
VkPipeline ResourceCache::fetch(ComputePipelineDefinition const &rID)
{
	VkPipeline handle;
	fetch(computePipelines, rID, handle);
	return handle;
}
VkSampler ResourceCache::fetch(SamplerDefinition const &rID)
{
	VkSampler handle;
	fetch(samplers, rID, handle);
	return handle;
}

void ResourceCache::clearDescSetLayouts()
{
	clear(descSetLayouts);

	// Clear Dependencys
	clearPipelineLayouts();
	clearRasterizationPipelines();
	clearComputePipelines();
}
void ResourceCache::clearPipelineLayouts()
{
	clear(pipelineLayouts);

	// Clear Dependencys
	clearRasterizationPipelines();
	clearComputePipelines();
}
void ResourceCache::clearRasterizationPipelines()
{
	clear(rasterizationPipelines);
}
void ResourceCache::clearComputePipelines()
{
	clear(computePipelines);
}
void ResourceCache::clearRenderPasses()
{
	clear(renderPasses);

	// Clear Dependencys
	clearRasterizationPipelines();
	clearComputePipelines();
}
void ResourceCache::clearSamplers()
{
	clear(samplers);
}
void ResourceCache::clearShaders()
{
	clear(shaders);

	// Clear Dependencys
	clearRasterizationPipelines();
	clearComputePipelines();
}
void ResourceCache::clearAll()
{
	clearDescSetLayouts();
	clearPipelineLayouts();
	clearRasterizationPipelines();
	clearComputePipelines();
	clearRenderPasses();
	clearSamplers();
	clearShaders();
}
}