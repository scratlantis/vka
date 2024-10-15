#include "complex_commands.h"
#include <vka/globals.h>

namespace vka
{
// Render Pass
VkSubpassDependency_OP initialSubpassDependency()
{
	VkSubpassDependency_OP dependency{};
	dependency.srcSubpass      = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass      = 0;
	dependency.srcStageMask    = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	dependency.dstStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask   = VK_ACCESS_MEMORY_READ_BIT;
	dependency.dstAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
	return dependency;
}
VkSubpassDependency_OP finalSubpassDependency()
{
	VkSubpassDependency_OP dependency{};
	dependency.srcSubpass      = 0;
	dependency.dstSubpass      = VK_SUBPASS_EXTERNAL;
	dependency.srcStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstStageMask    = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	dependency.srcAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependency.dstAccessMask   = VK_ACCESS_MEMORY_READ_BIT;
	dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
	return dependency;
}

RenderPassDefinition defaultRenderPass()
{
	RenderPassDefinition def = {};
	SubpassDescription   subpassDescription{};
	subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	def.subpassDescriptions              = {subpassDescription};
	def.subpassDependencies              = {initialSubpassDependency(), finalSubpassDependency()};
	return def;
}

void addAttachment(RenderPassDefinition &def, AttachmentLayoutDescription layout, VkFormat format, bool clear)
{
	VkAttachmentDescription attachment{};
	attachment.format         = format;
	attachment.samples        = VK_SAMPLE_COUNT_1_BIT;
	attachment.loadOp         = clear ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
	attachment.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
	attachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachment.initialLayout  = layout.in;
	attachment.finalLayout    = layout.out;
	def.attachmentDescriptions.push_back(attachment);
	VkAttachmentReference_OP ref{};
	ref.attachment = def.attachmentDescriptions.size() - 1;
	ref.layout     = layout.internal;
	switch (layout.internal)
	{
		case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
			def.subpassDescriptions[def.currentSubpass].colorAttachments.push_back(ref);
			break;
		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
			def.subpassDescriptions[def.currentSubpass].depthStencilAttachment    = ref;
			def.subpassDescriptions[def.currentSubpass].hasDepthStencilAttachment = true;
			break;
		default:
			printVka("Unsupported internal layout");
			DEBUG_BREAK;
			break;
	}
}
void addColorAttachment(RenderPassDefinition &def, VkImageLayout layoutIn, VkImageLayout layoutOut, VkFormat format, bool clear)
{
	addAttachment(def, {layoutIn, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, layoutOut}, format, clear);
}

void addDepthAttachment(RenderPassDefinition &def, VkFormat format, bool clear)
{
	addAttachment(def, {VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL}, format, clear);
}

void nextSubpass(RenderPassDefinition &def)
{
	auto &subpassDescription             = def.subpassDescriptions.emplace_back();
	subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	def.currentSubpass++;
}

// Rasterization Pipeline
RasterizationPipelineDefinition defaultRasterizationPipeline()
{
	RasterizationPipelineDefinition def       = {};
	def.multisampleState.sType                = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	def.multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	def.multisampleState.minSampleShading     = 1.f;
	def.inputAssemblyState.sType              = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	def.inputAssemblyState.topology           = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	def.tessellationState.sType               = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
	def.rasterizationState.sType              = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	def.rasterizationState.lineWidth          = 1.0;
	def.depthStencilState.sType               = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	def.depthStencilState.minDepthBounds      = 0.0;
	def.depthStencilState.maxDepthBounds      = 1.0;
	def.globalColorBlendState.logicOp         = VK_LOGIC_OP_NO_OP;
	def.dynamicStates                         = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
	def.renderPassDefinition                  = defaultRenderPass();
	return def;
}

void addInput(RasterizationPipelineDefinition &def, VertexDataLayout inputLayout, VkVertexInputRate inputRate)
{
	VKA_ASSERT(inputLayout.formats.size() == inputLayout.offsets.size());
	uint32_t locationOffset = 0;
	if (!def.vertexAttributeDescriptions.empty())
	{
		locationOffset = 1 + def.vertexAttributeDescriptions.back().location;
	}
	for (size_t i = 0; i < inputLayout.formats.size(); i++)
	{
		VkVertexInputAttributeDescription_OP attributeDesc{};
		attributeDesc.location = locationOffset + i;
		attributeDesc.binding  = def.vertexBindingDescriptions.size();
		attributeDesc.format   = inputLayout.formats[i];
		attributeDesc.offset   = inputLayout.offsets[i];
		def.vertexAttributeDescriptions.push_back(attributeDesc);
	}
	VkVertexInputBindingDescription_OP bindingDesc{};
	bindingDesc.binding   = def.vertexBindingDescriptions.size();
	bindingDesc.stride    = inputLayout.stride;
	bindingDesc.inputRate = inputRate;
	def.vertexBindingDescriptions.push_back(bindingDesc);
}

void addShader(RasterizationPipelineDefinition &def, std::string path, std::vector<ShaderArgs> args)
{
	def.shaderDefinitions.push_back(ShaderDefinition(path, args));
}

void addWriteAttachmentState(RasterizationPipelineDefinition &def)
{
	VkPipelineColorBlendAttachmentState_OP attachmentBlendState{};
	attachmentBlendState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	def.colorBlendAttachmentStates.push_back(attachmentBlendState);
}

void addBlendAttachmentState(RasterizationPipelineDefinition &def, BlendOperation colorBlendOp, BlendOperation alphaBlendOp,
                             VkColorComponentFlags colorWriteFlags = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT)
{
	VkPipelineColorBlendAttachmentState_OP attachmentBlendState{};
	attachmentBlendState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	attachmentBlendState.blendEnable    = VK_TRUE;

	attachmentBlendState.srcAlphaBlendFactor = colorBlendOp.srcFactor;
	attachmentBlendState.dstAlphaBlendFactor = colorBlendOp.dstFactor;
	attachmentBlendState.alphaBlendOp        = colorBlendOp.op;

	attachmentBlendState.srcColorBlendFactor = alphaBlendOp.srcFactor;
	attachmentBlendState.dstColorBlendFactor = alphaBlendOp.dstFactor;
	attachmentBlendState.colorBlendOp        = alphaBlendOp.op;
	def.colorBlendAttachmentStates.push_back(attachmentBlendState);
}

void addDepthAttachment(RasterizationPipelineDefinition &def, Image depthImage, VkBool32 enableWrite, VkCompareOp compareOp, bool clear)
{
	if (enableWrite || compareOp != VK_COMPARE_OP_ALWAYS)
	{
		def.depthStencilState.depthTestEnable = VK_TRUE;
	}
	def.depthStencilState.depthWriteEnable = enableWrite;
	def.depthStencilState.depthCompareOp   = compareOp;
	addDepthAttachment(def.renderPassDefinition, depthImage->getFormat(), clear);
}

void addWriteColorAttachment(RasterizationPipelineDefinition &def, Image image,
                             VkImageLayout layoutIn, VkImageLayout layoutOut, bool clear)
{
	addWriteAttachmentState(def);
	addColorAttachment(def.renderPassDefinition, layoutIn, layoutOut, image->getFormat(), clear);
}

void addBlendColorAttachment(RasterizationPipelineDefinition &def, Image image,
                             VkImageLayout layoutIn, VkImageLayout layoutOut, bool clear,
                             BlendOperation colorBlendOp, BlendOperation alphaBlendOp)
{
	addBlendAttachmentState(def, colorBlendOp, alphaBlendOp);
	addColorAttachment(def.renderPassDefinition, layoutIn, layoutOut, image->getFormat(), clear);
}

void addDescriptor(RasterizationPipelineDefinition &def, VkDescriptorType type, VkShaderStageFlags shaderStage)
{
	if (def.pipelineLayoutDefinition.descSetLayoutDef.empty())
	{
		def.pipelineLayoutDefinition.descSetLayoutDef.push_back({});
	}
	def.pipelineLayoutDefinition.descSetLayoutDef.back().addDescriptor(shaderStage, type);
}

void nextDescriptorSet(RasterizationPipelineDefinition &def)
{
	def.pipelineLayoutDefinition.descSetLayoutDef.push_back({});
}

void addDescriptor(ComputePipelineDefinition &def, VkDescriptorType type)
{
	if (def.pipelineLayoutDefinition.descSetLayoutDef.empty())
	{
		def.pipelineLayoutDefinition.descSetLayoutDef.push_back({});
	}
	def.pipelineLayoutDefinition.descSetLayoutDef.back().addDescriptor(VK_SHADER_STAGE_COMPUTE_BIT, type);
}

void addPushConstant(RasterizationPipelineDefinition &def, uint32_t size, VkShaderStageFlags shaderStage)
{
	uint32_t offset = 0;
	for (auto &range : def.pipelineLayoutDefinition.pcRanges)
	{
		offset += range.size;
		offset = alignUp(offset, VKA_PUSH_CONSTANT_RANGE_ALLIGNMENT);
	}
	def.pipelineLayoutDefinition.pcRanges.push_back(VkPushConstantRange_OP({shaderStage, offset, size}));
}

void addPushConstant(ComputePipelineDefinition &def, uint32_t size)
{
	uint32_t offset = 0;
	for (auto &range : def.pipelineLayoutDefinition.pcRanges)
	{
		offset += range.size;
		offset = alignUp(offset, VKA_PUSH_CONSTANT_RANGE_ALLIGNMENT);
	}
	def.pipelineLayoutDefinition.pcRanges.push_back(VkPushConstantRange_OP({VK_SHADER_STAGE_COMPUTE_BIT, offset, size}));
}
}        // namespace vka