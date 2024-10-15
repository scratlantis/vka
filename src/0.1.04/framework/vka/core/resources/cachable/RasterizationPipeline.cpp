#include "RasterizationPipeline.h"
#include <vka/globals.h>
namespace vka
{
hash_t RasterizationPipelineDefinition::hash() const
{
	// clang-format off
	// may remove parts for performance
	return
	flags
	HASHC subpass
	HASHC multisampleState

	HASHC inputAssemblyState.hash()
	HASHC tessellationState.hash()
	HASHC rasterizationState.hash()
	HASHC depthStencilState.hash()
	HASHC globalColorBlendState.hash()
	
	HASHC pipelineLayoutDefinition.hash()
	HASHC renderPassDefinition.hash()
	HASHC hashVector(shaderDefinitions)

	HASHC hashVector(dynamicStates)
	HASHC hashVector(sampleMasks)

	HASHC hashVector(colorBlendAttachmentStates)
	HASHC hashVector(shaderDefinitions)
	HASHC hashVector(vertexBindingDescriptions)
	HASHC hashVector(vertexAttributeDescriptions)
	HASHC hashVector(viewports)
	HASHC hashVector(scissors);
	// clang-format on
}

DEFINE_EQUALS_OVERLOAD(RasterizationPipelineDefinition, ResourceIdentifier)

bool RasterizationPipelineDefinition::operator==(const RasterizationPipelineDefinition &other) const
{
	// clang-format off
	return
	flags == other.flags
	&& subpass == other.subpass
	&& multisampleState == other.multisampleState

	&& cmpPtr(&inputAssemblyState, &other.inputAssemblyState)
	&& cmpPtr(&tessellationState, &other.tessellationState)
	&& cmpPtr(&rasterizationState, &other.rasterizationState)
	&& cmpPtr(&depthStencilState, &other.depthStencilState)
	&& cmpPtr(&globalColorBlendState, &other.globalColorBlendState)
	
	&& pipelineLayoutDefinition == other.pipelineLayoutDefinition
	&& renderPassDefinition == other.renderPassDefinition
	&& cmpVector(shaderDefinitions, other.shaderDefinitions)

	&& cmpArray(dynamicStates.data(), other.dynamicStates.data(), dynamicStates.size())
	&& cmpArray(sampleMasks.data(), other.sampleMasks.data(), sampleMasks.size())

	&& cmpVector(colorBlendAttachmentStates, other.colorBlendAttachmentStates)
	&& cmpVector(shaderDefinitions, other.shaderDefinitions)
	&& cmpVector(vertexBindingDescriptions, other.vertexBindingDescriptions)
	&& cmpVector(vertexAttributeDescriptions, other.vertexAttributeDescriptions)
	&& cmpVector(viewports, other.viewports)
	&& cmpVector(scissors, other.scissors);
	// clang-format on
}

void RasterizationPipeline_R::free()
{
	vkDestroyPipeline(gState.device.logical, handle, nullptr);
}

static VkPipelineVertexInputStateCreateInfo makeVertexInputStateCI(
    std::vector<VkVertexInputBindingDescription_OP> const &bindingDescriptions,
    std::vector<VkVertexInputAttributeDescription_OP>     &attributeDescriptions)
{
	VkPipelineVertexInputStateCreateInfo ci{VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};
	ci.vertexBindingDescriptionCount   = VKA_COUNT(bindingDescriptions);
	ci.pVertexBindingDescriptions      = bindingDescriptions.data();
	ci.vertexAttributeDescriptionCount = VKA_COUNT(attributeDescriptions);
	ci.pVertexAttributeDescriptions    = attributeDescriptions.data();
	return ci;
}

static VkPipelineViewportStateCreateInfo makeViewportStateCI(
    std::vector<VkViewport_OP> const &viewports,
    std::vector<VkRect2D_OP> const   &scissors)
{
	VkPipelineViewportStateCreateInfo ci{VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO};
	ci.viewportCount = VKA_COUNT(viewports);
	ci.pViewports    = viewports.data();
	ci.scissorCount  = VKA_COUNT(scissors);
	ci.pScissors     = scissors.data();
	return ci;
}

static VkPipelineColorBlendStateCreateInfo makeColorBlendStateCI(
    std::vector<VkPipelineColorBlendAttachmentState_OP> const &colorBlendAttachmentStates,
    GlobalColorBlendState const                               &globalColorBlendState)
{
	VkPipelineColorBlendStateCreateInfo ci{VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};
	ci.attachmentCount = VKA_COUNT(colorBlendAttachmentStates);
	ci.pAttachments    = colorBlendAttachmentStates.data();
	ci.logicOpEnable   = globalColorBlendState.logicOpEnable;
	ci.logicOp         = globalColorBlendState.logicOp;
	memcpy(ci.blendConstants, globalColorBlendState.blendConstants, sizeof(ci.blendConstants));
	return ci;
}        // namespace vka

static VkPipelineDynamicStateCreateInfo makeDynamicStateCI(std::vector<VkDynamicState> const &dynamicStates)
{
	VkPipelineDynamicStateCreateInfo ci{VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO};
	ci.dynamicStateCount = VKA_COUNT(dynamicStates);
	ci.pDynamicStates    = dynamicStates.data();
	return ci;
}

RasterizationPipeline_R::RasterizationPipeline_R(IResourceCache *pCache, RasterizationPipelineDefinition const &def) :
    Cachable_T<VkPipeline>(pCache)
{
	std::vector<VkViewport_OP> viewports;
	std::vector<VkRect2D_OP>   scissors;
	if (def.viewports.size() == 0 && def.scissors.size() == 0)
	{
		VkViewport_OP viewport;
		viewport.width    = (float) gState.io.extent.width;
		viewport.height   = (float) gState.io.extent.height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		viewports.push_back(viewport);
		VkRect2D_OP scissor;
		scissor.extent.width  = gState.io.extent.width;
		scissor.extent.height = gState.io.extent.height;
		scissors.push_back(scissor);
	}
	VkGraphicsPipelineCreateInfo ci{VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};
	ci.flags = def.flags;
	std::vector<VkPipelineShaderStageCreateInfo> stageCIs(def.shaderDefinitions.size());
	for (size_t i = 0; i < stageCIs.size(); i++)
	{
		stageCIs[i] = makeShaderStageCI(def.shaderDefinitions[i], pCache->fetch(def.shaderDefinitions[i]));
	}
	ci.stageCount = VKA_COUNT(stageCIs);
	ci.pStages    = stageCIs.data();

	std::vector<VkVertexInputAttributeDescription_OP> attributeDescriptions = def.vertexAttributeDescriptions;
	for (size_t i = 0; i < attributeDescriptions.size(); i++)
	{
		attributeDescriptions[i].location = i;
	}

	VkPipelineVertexInputStateCreateInfo vertexInputStateCI = makeVertexInputStateCI(def.vertexBindingDescriptions, attributeDescriptions);
	ci.pVertexInputState                                    = &vertexInputStateCI;
	ci.pInputAssemblyState                                  = &def.inputAssemblyState;
	ci.pTessellationState                                   = &def.tessellationState;
	VkPipelineViewportStateCreateInfo viewportStateCI       = makeViewportStateCI(viewports, scissors);
	ci.pViewportState                                       = &viewportStateCI;
	ci.pRasterizationState                                  = &def.rasterizationState;
	ci.pMultisampleState                                    = &def.multisampleState;
	ci.pDepthStencilState                                   = &def.depthStencilState;
	VkPipelineColorBlendStateCreateInfo colorBlendStateCI   = makeColorBlendStateCI(def.colorBlendAttachmentStates, def.globalColorBlendState);
	ci.pColorBlendState                                     = &colorBlendStateCI;
	VkPipelineDynamicStateCreateInfo dynamicStateCI         = makeDynamicStateCI(def.dynamicStates);
	ci.pDynamicState                                        = &dynamicStateCI;
	ci.layout                                               = pCache->fetch(def.pipelineLayoutDefinition);
	ci.renderPass                                           = pCache->fetch(def.renderPassDefinition);
	ci.subpass                                              = def.subpass;
	ci.basePipelineHandle                                   = nullptr;        // Dont support pipeline derivatives yet
	ci.basePipelineIndex                                    = -1;
	VK_CHECK(vkCreateGraphicsPipelines(gState.device.logical, VK_NULL_HANDLE, 1, &ci, nullptr, &handle));
}

}        // namespace vka