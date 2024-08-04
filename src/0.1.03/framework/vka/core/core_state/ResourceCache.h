#pragma once
#include <unordered_map>

#include <vka/core/resources/cachable/ComputePipeline.h>
#include <vka/core/resources/cachable/DescriptorSetLayout.h>
#include <vka/core/resources/cachable/PipelineLayout.h>
#include <vka/core/resources/cachable/RasterizationPipeline.h>
#include <vka/core/resources/cachable/RenderPass.h>
#include <vka/core/resources/cachable/Sampler.h>
#include <vka/core/resources/cachable/Shader.h>


namespace vka
{

class ResourceCache : public IResourceCache
{
  private:
	std::unordered_map<DescriptorSetLayoutDefinition, DescriptorSetLayout_R>     descSetLayouts;
	std::unordered_map<PipelineLayoutDefinition, PipelineLayout_R>               pipelineLayouts;
	std::unordered_map<RasterizationPipelineDefinition, RasterizationPipeline_R> rasterizationPipelines;
	std::unordered_map<ComputePipelineDefinition, ComputePipeline_R>             computePipelines;
	std::unordered_map<RenderPassDefinition, RenderPass_R>                       renderPasses;
	std::unordered_map<SamplerDefinition, Sampler_R>                             samplers;
	std::unordered_map<ShaderDefinition, Shader_R>                               shaders;

	template <typename Def, typename Obj, typename Handle>
	void fetch(std::unordered_map<Def, Obj> &map, Def const &rID, Handle &handle)
	{
		auto it = map.find(rID);
		if (it != map.end())
			handle = it->second.getHandle();
		else
		{
			printVka("Cachable created.");
			Obj  obj = Obj(this, rID);        // civ
			auto ret = map.insert({rID, obj});
			VKA_ASSERT(ret.second);
			handle = obj.getHandle();
		}
	}
	template <typename Def, typename Obj>
	void clear(std::unordered_map<Def, Obj> &map)
	{
		auto it = map.begin();
		for (auto it = map.begin(); it != map.end(); ++it)
		{
			it->second.free();
		}
		map.clear();
	}

  public:
	

	ResourceCache(){};
	~ResourceCache(){};
	DELETE_COPY_CONSTRUCTORS(ResourceCache);
	VkDescriptorSetLayout fetch(DescriptorSetLayoutDefinition const &rID) override;
	VkPipelineLayout      fetch(PipelineLayoutDefinition const &rID) override;
	VkShaderModule        fetch(ShaderDefinition const &rID) override;
	VkRenderPass          fetch(RenderPassDefinition const &rID) override;
	VkPipeline            fetch(RasterizationPipelineDefinition const &rID) override;
	VkPipeline            fetch(ComputePipelineDefinition const &rID) override;
	VkSampler             fetch(SamplerDefinition const &rID) override;
	void                  clearDescSetLayouts() override;
	void                  clearPipelineLayouts() override;
	void                  clearRasterizationPipelines() override;
	void                  clearComputePipelines() override;
	void                  clearRenderPasses() override;
	void                  clearSamplers() override;
	void                  clearShaders() override;
	void                  clearAll() override;
};
}        // namespace vka
