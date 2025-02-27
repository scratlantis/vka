#pragma once
#include <vka/advanced_state/AdvancedState.h>
#include <vka/core/core_utility/types.h>
#include <vka/core/resources/Descriptor.h>
#include <vka/core/resources/cachable/ComputePipeline.h>
#include <vka/core/resources/cachable/RasterizationPipeline.h>
namespace vka
{

struct BlendOperation
{
	VkBlendFactor srcFactor;
	VkBlendFactor dstFactor;
	VkBlendOp     op;

	static BlendOperation write()
	{
		return BlendOperation{VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD};
	}
	static BlendOperation alpha()
	{
		return BlendOperation{VK_BLEND_FACTOR_SRC_ALPHA, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA, VK_BLEND_OP_ADD};
	}
	static BlendOperation add()
	{
		return BlendOperation{VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ONE, VK_BLEND_OP_ADD};
	}

	bool operator== (const BlendOperation &other) const
	{
		return srcFactor == other.srcFactor && dstFactor == other.dstFactor && op == other.op;
	}
};

struct AttachmentLayoutDescription
{
	VkImageLayout in;
	VkImageLayout internal;
	VkImageLayout out;
};

VkSubpassDependency_OP          initialSubpassDependency();
VkSubpassDependency_OP          finalSubpassDependency();
RenderPassDefinition            defaultRenderPass();
void                            addAttachment(RenderPassDefinition &def, AttachmentLayoutDescription layout, VkFormat format, bool clear);
void                            addColorAttachment(RenderPassDefinition &def, VkImageLayout layoutIn, VkImageLayout layoutOut, VkFormat format, bool clear);
void                            addDepthAttachment(RenderPassDefinition &def, VkFormat format, bool clear);
void                            nextSubpass(RenderPassDefinition &def);
RasterizationPipelineDefinition defaultRasterizationPipeline();
void                            addInput(RasterizationPipelineDefinition &def, VertexDataLayout inputLayout, VkVertexInputRate inputRate);
void                            addShader(RasterizationPipelineDefinition &def, std::string path, std::vector<ShaderArgs> args = {});
void                            addWriteAttachmentState(RasterizationPipelineDefinition &def);
void                            addBlendAttachmentState(RasterizationPipelineDefinition &def, BlendOperation colorBlendOp, BlendOperation alphaBlendOp, VkColorComponentFlags colorWriteFlags);
void                            addDepthAttachment(RasterizationPipelineDefinition &def, Image depthImage, VkBool32 enableWrite, VkCompareOp compareOp, bool clear);
void                            addWriteColorAttachment(RasterizationPipelineDefinition &def, Image image, VkImageLayout layoutIn, VkImageLayout layoutOut, bool clear);
void                            addBlendColorAttachment(RasterizationPipelineDefinition &def, Image image, VkImageLayout layoutIn, VkImageLayout layoutOut, bool clear, BlendOperation colorBlendOp, BlendOperation alphaBlendOp);
void                            addDescriptor(RasterizationPipelineDefinition &def, VkDescriptorType type, VkShaderStageFlags shaderStage);
void                            nextDescriptorSet(RasterizationPipelineDefinition &def);
void                            addDescriptor(ComputePipelineDefinition &def, VkDescriptorType type);
void                            addPushConstant(RasterizationPipelineDefinition &def, uint32_t size, VkShaderStageFlags shaderStage);
void                            addPushConstant(ComputePipelineDefinition &def, uint32_t size);

class DrawCmd
{
  public:
	DrawCmd();

	void pushDepthAttachment(Image depthImage, VkBool32 enableWrite, VkCompareOp compareOp);                                         // no layout transform
	void pushColorAttachment(Image image, VkImageLayout layoutOut, BlendOperation colorBlendOp, BlendOperation alphaBlendOp);        // change to target layout && blend
	void pushColorAttachment(Image image, VkImageLayout layoutOut);                                                                  // change to target layout
	void pushColorAttachment(Image image);                                                                                           // preserve layout

	void pushDescriptor(BufferRef buffer, VkDescriptorType type, VkShaderStageFlags shaderStage);
	void pushDescriptor(Image image, VkDescriptorType type, VkShaderStageFlags shaderStage);
	void pushDescriptor(const SamplerDefinition sampler, VkShaderStageFlags shaderStage);
	void pushDescriptor(const SamplerDefinition sampler, Image image, VkShaderStageFlags shaderStage);
	void pushDescriptor(std::vector<BufferRef> buffers, VkDescriptorType type, VkShaderStageFlags shaderStage);
	void pushDescriptor(std::vector<Image> images, VkDescriptorType type, VkShaderStageFlags shaderStage);
	void pushDescriptor(std::vector<SamplerDefinition> samplersDefs, VkShaderStageFlags shaderStage);
	void pushDescriptor(TLASRef as, VkShaderStageFlags shaderStage);
	void pushDescriptor(CmdBuffer cmdBuf, IResourcePool *pPool, void *data, VkDeviceSize size, VkShaderStageFlags stageFlags);
	void pushConstant(void *data, VkDeviceSize size, VkShaderStageFlags stageFlags);

	void setGeometry(DrawSurface surface);
	void pushInstanceData(BufferRef buffer, VertexDataLayout layout);
	void pushVertexData(BufferRef buffer, VertexDataLayout layout);


	void exec(CmdBuffer cmdBuf) const;

	std::vector<Descriptor> descriptors;
	std::vector<uint8_t>    pushConstantsData;
	std::vector<uint32_t>   pushConstantsSizes;

	DrawSurface                     surf          = {};
	RasterizationPipelineDefinition pipelineDef   = {};
	uint32_t                        instanceCount = 1;
	VkRect2D_OP                     renderArea    = {};

  private:
	std::vector<Image>      attachments;
	std::vector<ClearValue> clearValues;
	std::vector<BufferRef>  additionalVertexBuffers;

	CmdBufferState getCmdBufferState(const CmdBufferState oldState) const;
};

class ComputeCmd
{
  public:
	ComputeCmd() = default;
	ComputeCmd(uint32_t taskSize, const std::string path, std::vector<ShaderArgs> args = {});
	ComputeCmd(glm::uvec2 taskSize, std::string path, std::vector<ShaderArgs> args = {});
	ComputeCmd(VkExtent2D taskSize, std::string path, std::vector<ShaderArgs> args = {});
	ComputeCmd(VkExtent3D taskSize, std::string path, std::vector<ShaderArgs> args = {});
	ComputeCmd(glm::uvec3 taskSize, std::string path, std::vector<ShaderArgs> args = {});

	void pushSubmodule(const std::string path, std::vector<ShaderArgs> args = {});
	void pushSIDebugHeader();
	void pushLocal();
	void pushDescriptor(BufferRef buffer, VkDescriptorType type);
	void pushDescriptor(Image image, VkDescriptorType type);
	void pushDescriptor(const SamplerDefinition sampler);
	void pushDescriptor(const SamplerDefinition sampler, Image image);
	void pushDescriptor(std::vector<Image> images, VkDescriptorType type);
	void pushDescriptor(std::vector<BufferRef> buffers, VkDescriptorType type);
	void pushDescriptor(std::vector<SamplerDefinition> samplersDefs);
	void pushDescriptor(TLASRef as);
	void pushDescriptor(CmdBuffer cmdBuf, IResourcePool *pPool, void *data, VkDeviceSize size);
	void pushConstant(void *data, VkDeviceSize size);
	void pushSpecializationConst(void *data, uint32_t size);
	void pushSpecializationConst(uint32_t val);
	void pushSpecializationConst(int val);
	void pushSpecializationConst(float val);


	void exec(CmdBuffer cmdBuf) const;

	std::vector<Descriptor>   descriptors;
	std::vector<uint8_t>      pushConstantsData;
	std::vector<uint32_t>     pushConstantsSizes;
	ComputePipelineDefinition pipelineDef;

  private:
	glm::uvec3     workGroupCount;
	CmdBufferState getCmdBufferState() const;
	void           pushBaseModule(glm::uvec3 invocationCount);
};

}        // namespace vka