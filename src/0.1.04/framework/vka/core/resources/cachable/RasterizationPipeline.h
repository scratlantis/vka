#pragma once
#include "DescriptorSetLayout.h"
#include "PipelineLayout.h"
#include "RenderPass.h"
#include "../Resource.h"
#include "Shader.h"

namespace vka
{
struct GlobalColorBlendState
{
	GlobalColorBlendState(){};
	VkPipelineColorBlendStateCreateFlags flags;
	VkBool32                             logicOpEnable;
	VkLogicOp                            logicOp;
	float                                blendConstants[4];

	bool operator==(const GlobalColorBlendState &other) const
	{
		return flags == other.flags && logicOpEnable == other.logicOpEnable && logicOp == other.logicOp && blendConstants[0] == other.blendConstants[0] &&
		       blendConstants[1] == other.blendConstants[1] && blendConstants[2] == other.blendConstants[2] && blendConstants[3] == other.blendConstants[3];
	}

	bool operator!=(const GlobalColorBlendState &other) const
	{
		return !(*this == other);
	}

	hash_t hash() const
	{
		// clang-format off
		return flags
			HASHC logicOpEnable
			HASHC logicOp
			HASHC blendConstants[0]
			HASHC blendConstants[1]
			HASHC blendConstants[2]
			HASHC blendConstants[3];
		// clang-format on
	}
};
class RasterizationPipelineDefinition : public ResourceIdentifier
{
  public:
	VkPipelineCreateFlags                 flags;
	uint32_t                              subpass;
	PipelineMultisampleStateCreateInfo_OP multisampleState;

	VkPipelineInputAssemblyStateCreateInfo_OP inputAssemblyState;
	VkPipelineTessellationStateCreateInfo_OP  tessellationState;
	VkPipelineRasterizationStateCreateInfo_OP rasterizationState;
	VkPipelineDepthStencilStateCreateInfo_OP  depthStencilState;
	GlobalColorBlendState                     globalColorBlendState;

	PipelineLayoutDefinition pipelineLayoutDefinition;
	RenderPassDefinition     renderPassDefinition;

	std::vector<ShaderDefinition> shaderDefinitions;

	std::vector<VkDynamicState> dynamicStates;
	std::vector<VkSampleMask>   sampleMasks;

	std::vector<VkPipelineColorBlendAttachmentState_OP> colorBlendAttachmentStates;
	std::vector<VkVertexInputBindingDescription_OP>     vertexBindingDescriptions;
	std::vector<VkVertexInputAttributeDescription_OP>   vertexAttributeDescriptions;
	std::vector<VkViewport_OP>                          viewports;
	std::vector<VkRect2D_OP>                            scissors;

	bool   operator==(const ResourceIdentifier &other) const override;
	bool   operator==(const RasterizationPipelineDefinition &other) const;
	hash_t hash() const override;
};

class RasterizationPipeline_R : public Cachable_T<VkPipeline>
{
  public:
	virtual void     free() override;
	RasterizationPipeline_R(IResourceCache *pCache, RasterizationPipelineDefinition const &def);
};
}        // namespace vka
DECLARE_HASH(vka::RasterizationPipelineDefinition, hash)