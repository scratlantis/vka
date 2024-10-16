#include "misc.h"
#include "../utility/compare.h"
#include "../utility/hash.h"
#include "../utility/misc.h"

namespace vka
{

ClearValue::ClearValue()
{
	type = CLEAR_VALUE_NONE;
}

ClearValue::ClearValue(glm::vec4 color)
{
	type                   = CLEAR_VALUE_FLOAT;
	value.color.float32[0] = color.r;
	value.color.float32[1] = color.g;
	value.color.float32[2] = color.b;
	value.color.float32[3] = color.a;
}

ClearValue::ClearValue(float r, float g, float b, float a)
{
	type                   = CLEAR_VALUE_FLOAT;
	value.color.float32[0] = r;
	value.color.float32[1] = g;
	value.color.float32[2] = b;
	value.color.float32[3] = a;
}
ClearValue::ClearValue(int32_t r, int32_t g, int32_t b, int32_t a)
{
	type                 = CLEAR_VALUE_INT;
	value.color.int32[0] = r;
	value.color.int32[1] = g;
	value.color.int32[2] = b;
	value.color.int32[3] = a;
}
ClearValue::ClearValue(uint32_t r, uint32_t g, uint32_t b, uint32_t a)
{
	type                  = CLEAR_VALUE_UINT;
	value.color.uint32[0] = r;
	value.color.uint32[1] = g;
	value.color.uint32[2] = b;
	value.color.uint32[3] = a;
}
ClearValue::ClearValue(float depth, uint32_t stencil)
{
	type                       = CLEAR_VALUE_DEPTH_STENCIL;
	value.depthStencil.depth   = depth;
	value.depthStencil.stencil = stencil;
}
bool ClearValue::operator==(ClearValue const &other) const
{
	if (type != other.type)
		return false;
	switch (type)
	{
		case CLEAR_VALUE_NONE:
			return other.type == CLEAR_VALUE_NONE;
		case CLEAR_VALUE_FLOAT:
			return value.color.float32[0] == other.value.color.float32[0] &&
			       value.color.float32[1] == other.value.color.float32[1] &&
			       value.color.float32[2] == other.value.color.float32[2] &&
			       value.color.float32[3] == other.value.color.float32[3];
		case CLEAR_VALUE_INT:
			return value.color.int32[0] == other.value.color.int32[0] &&
			       value.color.int32[1] == other.value.color.int32[1] &&
			       value.color.int32[2] == other.value.color.int32[2] &&
			       value.color.int32[3] == other.value.color.int32[3];
		case CLEAR_VALUE_UINT:
			return value.color.uint32[0] == other.value.color.uint32[0] &&
			       value.color.uint32[1] == other.value.color.uint32[1] &&
			       value.color.uint32[2] == other.value.color.uint32[2] &&
			       value.color.uint32[3] == other.value.color.uint32[3];
		case CLEAR_VALUE_DEPTH_STENCIL:
			return value.depthStencil.depth == other.value.depthStencil.depth &&
			       value.depthStencil.stencil == other.value.depthStencil.stencil;
	}
	return false;
}
bool ClearValue::operator!=(ClearValue const &other) const
{
	return !(*this == other);
}
bool ClearValue::hash() const
{
	// clang-format off
		switch (type)
		{
			case CLEAR_VALUE_NONE:
				return 0;
			case CLEAR_VALUE_FLOAT:
				return value.color.float32[0]
				       HASHC value.color.float32[1]
				       HASHC value.color.float32[2]
				       HASHC value.color.float32[3];
			case CLEAR_VALUE_INT:
				return value.color.int32[0]
				       HASHC value.color.int32[1]
				       HASHC value.color.int32[2]
				       HASHC value.color.int32[3];
			case CLEAR_VALUE_UINT:
				return value.color.uint32[0]
				       HASHC value.color.uint32[1]
				       HASHC value.color.uint32[2]
				       HASHC value.color.uint32[3];
			case CLEAR_VALUE_DEPTH_STENCIL:
				return value.depthStencil.depth
				       HASHC value.depthStencil.stencil;
		}
	// clang-format on
}

bool ClearValue::joinable(ClearValue const &previous) const
{
	return type == CLEAR_VALUE_NONE || *this == previous;
}

ClearValue ClearValue::white()
{
	return ClearValue(1.f, 1.f, 1.f, 1.f);
}

ClearValue ClearValue::black()
{
	return ClearValue(0.f, 0.f, 0.f, 1.f);
}

ClearValue ClearValue::max_depth()
{
	return ClearValue(0.f, 1.f);
}

ClearValue ClearValue::none()
{
	return ClearValue();
}

// RenderPassStuff

VkAttachmentReference_OP::VkAttachmentReference_OP(VkAttachmentReference const &other)
{
	this->attachment = other.attachment;
	this->layout     = other.layout;
}
bool VkAttachmentReference_OP::operator==(VkAttachmentReference_OP const &other) const
{
	// clang-format off
		return attachment == other.attachment
			&& layout == other.layout;
	// clang-format on
}
bool VkAttachmentReference_OP::operator!=(VkAttachmentReference_OP const &other) const
{
	return !(*this == other);
}
bool VkAttachmentReference_OP::hash() const
{
	// clang-format off
		return attachment
			HASHC layout;
	// clang-format on
}

SubpassDescription::SubpassDescription(VkSubpassDescription const &other)
{
	this->flags             = other.flags;
	this->pipelineBindPoint = other.pipelineBindPoint;
	this->inputAttachments.resize(other.inputAttachmentCount);
	for (size_t i = 0; i < other.inputAttachmentCount; i++)
	{
		this->inputAttachments[i] = other.pInputAttachments[i];
	}
	this->colorAttachments.resize(other.colorAttachmentCount);
	for (size_t i = 0; i < other.colorAttachmentCount; i++)
	{
		this->colorAttachments[i] = other.pColorAttachments[i];
	}
	if (other.pDepthStencilAttachment)
	{
		this->depthStencilAttachment = *other.pDepthStencilAttachment;
		hasDepthStencilAttachment    = true;
	}
	else
	{
		this->depthStencilAttachment = {};
	}
	this->preserveAttachments.resize(other.preserveAttachmentCount);
	for (size_t i = 0; i < other.preserveAttachmentCount; i++)
	{
		this->preserveAttachments[i] = other.pPreserveAttachments[i];
	}
}

VkSubpassDescription SubpassDescription::getVulkanStruct() const
{
	VkSubpassDescription subpassDescription{};
	subpassDescription.flags             = flags;
	subpassDescription.pipelineBindPoint = pipelineBindPoint;

	subpassDescription.inputAttachmentCount = inputAttachments.size();
	subpassDescription.pInputAttachments    = inputAttachments.data();

	subpassDescription.colorAttachmentCount = colorAttachments.size();
	subpassDescription.pColorAttachments    = colorAttachments.data();

	subpassDescription.pResolveAttachments = nullptr;

	if (hasDepthStencilAttachment)
	{
		subpassDescription.pDepthStencilAttachment = &depthStencilAttachment;
	}
	else
	{
		subpassDescription.pDepthStencilAttachment = nullptr;
	}
	subpassDescription.preserveAttachmentCount = preserveAttachments.size();

	subpassDescription.pPreserveAttachments = preserveAttachments.data();
	return subpassDescription;
}

bool SubpassDescription::operator==(SubpassDescription const &other) const
{
	// clang-format off
		return flags == other.flags
			&& pipelineBindPoint == other.pipelineBindPoint
			&& cmpVector(inputAttachments, other.inputAttachments)
			&& cmpVector(colorAttachments, other.colorAttachments)
			&& depthStencilAttachment == other.depthStencilAttachment
			&& cmpVector(preserveAttachments, other.preserveAttachments);
	// clang-format on
}
bool SubpassDescription::operator!=(SubpassDescription const &other) const
{
	return !(*this == other);
}
bool SubpassDescription::hash() const
{
	// clang-format off
		return flags
			HASHC pipelineBindPoint
			HASHC hashVector(inputAttachments)
			HASHC hashVector(colorAttachments)
			HASHC depthStencilAttachment.hash()
			HASHC hashVector(preserveAttachments);
	// clang-format on
}

PipelineMultisampleStateCreateInfo_OP::PipelineMultisampleStateCreateInfo_OP(VkPipelineMultisampleStateCreateInfo const &other)
{
	this->sType                 = other.sType;
	this->pNext                 = other.pNext;
	this->flags                 = other.flags;
	this->rasterizationSamples  = other.rasterizationSamples;
	this->sampleShadingEnable   = other.sampleShadingEnable;
	this->minSampleShading      = other.minSampleShading;
	this->pSampleMask           = other.pSampleMask;
	this->alphaToCoverageEnable = other.alphaToCoverageEnable;
	this->alphaToOneEnable      = other.alphaToOneEnable;
}
bool PipelineMultisampleStateCreateInfo_OP::operator==(PipelineMultisampleStateCreateInfo_OP const &other) const
{
	// clang-format off
		return flags == other.flags
			&& rasterizationSamples == other.rasterizationSamples
			&& sampleShadingEnable == other.sampleShadingEnable
			&& minSampleShading == other.minSampleShading
			&& alphaToCoverageEnable == other.alphaToCoverageEnable
			&& alphaToOneEnable == other.alphaToOneEnable;
	// clang-format on
}
bool PipelineMultisampleStateCreateInfo_OP::operator!=(PipelineMultisampleStateCreateInfo_OP const &other) const
{
	return !(*this == other);
}
bool PipelineMultisampleStateCreateInfo_OP::hash() const
{
	// clang-format off
		return flags
			HASHC rasterizationSamples
			HASHC sampleShadingEnable
			HASHC minSampleShading
			HASHC alphaToCoverageEnable
			HASHC alphaToOneEnable;
	// clang-format on
}

VkRect2D_OP::VkRect2D_OP(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{
	this->offset.x      = x;
	this->offset.y      = y;
	this->extent.width  = width;
	this->extent.height = height;
}
VkRect2D_OP::VkRect2D_OP(VkRect2D const &other)
{
	this->offset = other.offset;
	this->extent = other.extent;
}

VkRect2D_OP::VkRect2D_OP(VkExtent2D const &extent)
{
	this->offset = {0, 0};
	this->extent = extent;
}
bool VkRect2D_OP::operator==(VkRect2D_OP const &other) const
{
	// clang-format off
		return offset.x == other.offset.x
			&& offset.y == other.offset.y
			&& extent.width == other.extent.width
			&& extent.height == other.extent.height;
	// clang-format on
}
bool VkRect2D_OP::isValid(VkExtent2D const &extent) const
{
	return offset.x > 0 && offset.y > 0 && this->extent.width > 0 && this->extent.height > 0 && offset.x + this->extent.width <= extent.width && offset.y + this->extent.height <= extent.height;
}
VkRect2D_OP VkRect2D_OP::operator*(Rect2D<float> const &other) const
{
	// clang-format off
		VkRect2D_OP result;
		result.offset.x = other.x * extent.width;
		result.offset.y = other.y * extent.height;
		result.extent.width = extent.width * static_cast<uint32_t>(other.width);
		result.extent.height = extent.height * static_cast<uint32_t>(other.height);
		return result;
	// clang-format on
}

VkRect2D_OP VkRect2D_OP::absRegion(VkRect2D_OP const &other, Rect2D<float> const &relative)
{
	return other * Rect2D<float>{relative.x, relative.y, std::min(relative.width, 1.0f - relative.x), std::min(relative.height, 1.0f - relative.y)};
}

Rect2D<float> VkRect2D_OP::relRegion(VkRect2D_OP const &outer, VkRect2D_OP const &inner)
{
	Rect2D<float> relative;
	relative.x      = (float) inner.offset.x / outer.extent.width;
	relative.y      = (float) inner.offset.y / outer.extent.height;
	relative.width  = (float) inner.extent.width / outer.extent.width;
	relative.height = (float) inner.extent.height / outer.extent.height;
	return relative;
}

void VkRect2D_OP::operator*=(Rect2D<float> const &other)
{
	// clang-format off
		*this = *this * other;
	// clang-format on
}
bool VkRect2D_OP::operator!=(VkRect2D_OP const &other) const
{
	return !(*this == other);
}
bool VkRect2D_OP::hash() const
{
	// clang-format off
		return offset.x
			HASHC offset.y
			HASHC extent.width
			HASHC extent.height;
	// clang-format on
}

VkAttachmentDescription_OP::VkAttachmentDescription_OP(VkAttachmentDescription const &other)
{
	this->flags          = other.flags;
	this->format         = other.format;
	this->samples        = other.samples;
	this->loadOp         = other.loadOp;
	this->storeOp        = other.storeOp;
	this->stencilLoadOp  = other.stencilLoadOp;
	this->stencilStoreOp = other.stencilStoreOp;
	this->initialLayout  = other.initialLayout;
	this->finalLayout    = other.finalLayout;
}
bool VkAttachmentDescription_OP::operator==(VkAttachmentDescription_OP const &other) const
{
	// clang-format off
        return flags == other.flags
            && format == other.format
            && samples == other.samples
            && loadOp == other.loadOp
            && storeOp == other.storeOp
            && stencilLoadOp == other.stencilLoadOp
            && stencilStoreOp == other.stencilStoreOp
            && initialLayout == other.initialLayout
            && finalLayout == other.finalLayout;
	// clang-format on
}
bool VkAttachmentDescription_OP::operator!=(VkAttachmentDescription_OP const &other) const
{
	return !(*this == other);
}
bool VkAttachmentDescription_OP::hash() const
{
	// clang-format off
        return flags
            HASHC format
            HASHC samples
            HASHC loadOp
            HASHC storeOp
            HASHC stencilLoadOp
            HASHC stencilStoreOp
            HASHC initialLayout
            HASHC finalLayout;
	// clang-format on
}
bool VkAttachmentDescription_OP::joinable(VkAttachmentDescription_OP const &previous) const
{
	// clang-format off
        // these attributes must match exactly
        bool isJoinable =
            format == previous.format
            && samples == previous.samples
            && stencilLoadOp == previous.stencilLoadOp
            && stencilStoreOp == previous.stencilStoreOp
            && finalLayout == previous.finalLayout;


        // load/store
        isJoinable = isJoinable && (
                loadOp == previous.loadOp
                || loadOp == VK_ATTACHMENT_LOAD_OP_LOAD && previous.loadOp == VK_ATTACHMENT_LOAD_OP_CLEAR);

        // layout
        isJoinable = isJoinable && (
                initialLayout == previous.initialLayout
                || previous.finalLayout == initialLayout
                || initialLayout == VK_IMAGE_LAYOUT_UNDEFINED );

        return isJoinable;
	// clang-format on
}

VkSubpassDependency_OP::VkSubpassDependency_OP(VkSubpassDependency const &other)
{
	this->srcSubpass      = other.srcSubpass;
	this->dstSubpass      = other.dstSubpass;
	this->srcStageMask    = other.srcStageMask;
	this->dstStageMask    = other.dstStageMask;
	this->srcAccessMask   = other.srcAccessMask;
	this->dstAccessMask   = other.dstAccessMask;
	this->dependencyFlags = other.dependencyFlags;
}
bool VkSubpassDependency_OP::operator==(VkSubpassDependency_OP const &other) const
{
	// clang-format off
        return srcSubpass == other.srcSubpass
            && dstSubpass == other.dstSubpass
            && srcStageMask == other.srcStageMask
            && dstStageMask == other.dstStageMask
            && srcAccessMask == other.srcAccessMask
            && dstAccessMask == other.dstAccessMask
            && dependencyFlags == other.dependencyFlags;
	// clang-format on
}
bool VkSubpassDependency_OP::operator!=(VkSubpassDependency_OP const &other) const
{
	return !(*this == other);
}
bool VkSubpassDependency_OP::hash() const
{
	// clang-format off
        return srcSubpass
            HASHC dstSubpass
            HASHC srcStageMask
            HASHC dstStageMask
            HASHC srcAccessMask
            HASHC dstAccessMask
            HASHC dependencyFlags;
	// clang-format on
}

VkDescriptorSetLayoutBinding_OP::VkDescriptorSetLayoutBinding_OP(VkDescriptorSetLayoutBinding const &other)
{
	this->binding         = other.binding;
	this->descriptorType  = other.descriptorType;
	this->descriptorCount = other.descriptorCount;
	this->stageFlags      = other.stageFlags;
}
bool VkDescriptorSetLayoutBinding_OP::operator==(VkDescriptorSetLayoutBinding_OP const &other) const
{
	// clang-format off
        return binding == other.binding
            && descriptorType == other.descriptorType
            && descriptorCount == other.descriptorCount
            && stageFlags == other.stageFlags;
	// clang-format on
}
bool VkDescriptorSetLayoutBinding_OP::operator!=(VkDescriptorSetLayoutBinding_OP const &other) const
{
	return !(*this == other);
}
bool VkDescriptorSetLayoutBinding_OP::hash() const
{
	// clang-format off
        return binding
            HASHC descriptorType
            HASHC descriptorCount
            HASHC stageFlags;
	// clang-format on
}

VkPushConstantRange_OP::VkPushConstantRange_OP(VkPushConstantRange const &other)
{
	this->stageFlags = other.stageFlags;
	this->offset     = other.offset;
	this->size       = other.size;
}
bool VkPushConstantRange_OP::operator==(VkPushConstantRange_OP const &other) const
{
	// clang-format off
        return stageFlags == other.stageFlags
            && offset == other.offset
            && size == other.size;
	// clang-format on
}
bool VkPushConstantRange_OP::operator!=(VkPushConstantRange_OP const &other) const
{
	return !(*this == other);
}
bool VkPushConstantRange_OP::hash() const
{
	// clang-format off
        return stageFlags
            HASHC offset
            HASHC size;
	// clang-format on
}

VkPipelineInputAssemblyStateCreateInfo_OP::VkPipelineInputAssemblyStateCreateInfo_OP(VkPipelineInputAssemblyStateCreateInfo const &other)
{
	this->sType                  = other.sType;
	this->pNext                  = other.pNext;
	this->flags                  = other.flags;
	this->topology               = other.topology;
	this->primitiveRestartEnable = other.primitiveRestartEnable;
}
bool VkPipelineInputAssemblyStateCreateInfo_OP::operator==(VkPipelineInputAssemblyStateCreateInfo_OP const &other) const
{
	// clang-format off
        return flags == other.flags
            && topology == other.topology
            && primitiveRestartEnable == other.primitiveRestartEnable;
	// clang-format on
}
bool VkPipelineInputAssemblyStateCreateInfo_OP::operator!=(VkPipelineInputAssemblyStateCreateInfo_OP const &other) const
{
	return !(*this == other);
}
bool VkPipelineInputAssemblyStateCreateInfo_OP::hash() const
{
	// clang-format off
        return flags
            HASHC topology
            HASHC primitiveRestartEnable;
	// clang-format on
}

VkPipelineTessellationStateCreateInfo_OP::VkPipelineTessellationStateCreateInfo_OP(VkPipelineTessellationStateCreateInfo const &other)
{
	this->sType              = other.sType;
	this->pNext              = other.pNext;
	this->flags              = other.flags;
	this->patchControlPoints = other.patchControlPoints;
}
bool VkPipelineTessellationStateCreateInfo_OP::operator==(VkPipelineTessellationStateCreateInfo_OP const &other) const
{
	// clang-format off
        return flags == other.flags
            && patchControlPoints == other.patchControlPoints;
	// clang-format on
}
bool VkPipelineTessellationStateCreateInfo_OP::operator!=(VkPipelineTessellationStateCreateInfo_OP const &other) const
{
	return !(*this == other);
}
bool VkPipelineTessellationStateCreateInfo_OP::hash() const
{
	// clang-format off
        return flags
            HASHC patchControlPoints;
	// clang-format on
}

VkPipelineRasterizationStateCreateInfo_OP::VkPipelineRasterizationStateCreateInfo_OP(VkPipelineRasterizationStateCreateInfo const &other)
{
	this->sType                   = other.sType;
	this->pNext                   = other.pNext;
	this->flags                   = other.flags;
	this->depthClampEnable        = other.depthClampEnable;
	this->rasterizerDiscardEnable = other.rasterizerDiscardEnable;
	this->polygonMode             = other.polygonMode;
	this->cullMode                = other.cullMode;
	this->frontFace               = other.frontFace;
	this->depthBiasEnable         = other.depthBiasEnable;
	this->depthBiasConstantFactor = other.depthBiasConstantFactor;
	this->depthBiasClamp          = other.depthBiasClamp;
	this->depthBiasSlopeFactor    = other.depthBiasSlopeFactor;
	this->lineWidth               = other.lineWidth;
}
bool VkPipelineRasterizationStateCreateInfo_OP::operator==(VkPipelineRasterizationStateCreateInfo_OP const &other) const
{
	// clang-format off
        return flags == other.flags
            && depthClampEnable == other.depthClampEnable
            && rasterizerDiscardEnable == other.rasterizerDiscardEnable
            && polygonMode == other.polygonMode
            && cullMode == other.cullMode
            && frontFace == other.frontFace
            && depthBiasEnable == other.depthBiasEnable
            && depthBiasConstantFactor == other.depthBiasConstantFactor
            && depthBiasClamp == other.depthBiasClamp
            && depthBiasSlopeFactor == other.depthBiasSlopeFactor
            && lineWidth == other.lineWidth;
	// clang-format on
}
bool VkPipelineRasterizationStateCreateInfo_OP::operator!=(VkPipelineRasterizationStateCreateInfo_OP const &other) const
{
	return !(*this == other);
}
bool VkPipelineRasterizationStateCreateInfo_OP::hash() const
{
	// clang-format off
        return flags
            HASHC depthClampEnable
            HASHC rasterizerDiscardEnable
            HASHC polygonMode
            HASHC cullMode
            HASHC frontFace
            HASHC depthBiasEnable
            HASHC depthBiasConstantFactor
            HASHC depthBiasClamp
            HASHC depthBiasSlopeFactor
            HASHC lineWidth;
	// clang-format on
}

VkStencilOpState_OP::VkStencilOpState_OP(VkStencilOpState const &other)
{
	this->failOp      = other.failOp;
	this->passOp      = other.passOp;
	this->depthFailOp = other.depthFailOp;
	this->compareOp   = other.compareOp;
	this->compareMask = other.compareMask;
	this->writeMask   = other.writeMask;
	this->reference   = other.reference;
}

bool VkStencilOpState_OP::operator==(VkStencilOpState_OP const &other) const
{
	// clang-format off
        return failOp == other.failOp
            && passOp == other.passOp
            && depthFailOp == other.depthFailOp
            && compareOp == other.compareOp
            && compareMask == other.compareMask
            && writeMask == other.writeMask
            && reference == other.reference;
	// clang-format on
}
bool VkStencilOpState_OP::operator!=(VkStencilOpState_OP const &other) const
{
	return !(*this == other);
}
bool VkStencilOpState_OP::hash() const
{
	// clang-format off
        return failOp
            HASHC passOp
            HASHC depthFailOp
            HASHC compareOp
            HASHC compareMask
            HASHC writeMask
            HASHC reference;
	// clang-format on
}

VkPipelineDepthStencilStateCreateInfo_OP::VkPipelineDepthStencilStateCreateInfo_OP(VkPipelineDepthStencilStateCreateInfo const &other)
{
	this->sType                 = other.sType;
	this->pNext                 = other.pNext;
	this->flags                 = other.flags;
	this->depthTestEnable       = other.depthTestEnable;
	this->depthWriteEnable      = other.depthWriteEnable;
	this->depthCompareOp        = other.depthCompareOp;
	this->depthBoundsTestEnable = other.depthBoundsTestEnable;
	this->stencilTestEnable     = other.stencilTestEnable;
	this->front                 = other.front;
	this->back                  = other.back;
	this->minDepthBounds        = other.minDepthBounds;
	this->maxDepthBounds        = other.maxDepthBounds;
}
bool VkPipelineDepthStencilStateCreateInfo_OP::operator==(VkPipelineDepthStencilStateCreateInfo_OP const &other) const
{
	// clang-format off
        return flags == other.flags
            && depthTestEnable == other.depthTestEnable
            && depthWriteEnable == other.depthWriteEnable
            && depthCompareOp == other.depthCompareOp
            && depthBoundsTestEnable == other.depthBoundsTestEnable
            && stencilTestEnable == other.stencilTestEnable
            && VkStencilOpState_OP(front) ==  VkStencilOpState_OP(other.front)
            && VkStencilOpState_OP(back) == VkStencilOpState_OP(other.back)
            && minDepthBounds == other.minDepthBounds
            && maxDepthBounds == other.maxDepthBounds;
	// clang-format on
}
bool VkPipelineDepthStencilStateCreateInfo_OP::operator!=(VkPipelineDepthStencilStateCreateInfo_OP const &other) const
{
	return !(*this == other);
}
bool VkPipelineDepthStencilStateCreateInfo_OP::hash() const
{
	// clang-format off
        return flags
            HASHC depthTestEnable
            HASHC depthWriteEnable
            HASHC depthCompareOp
            HASHC depthBoundsTestEnable
            HASHC stencilTestEnable
            HASHC VkStencilOpState_OP(front)
            HASHC VkStencilOpState_OP(back)
            HASHC minDepthBounds
            HASHC maxDepthBounds;
	// clang-format on
}

VkPipelineColorBlendAttachmentState_OP::VkPipelineColorBlendAttachmentState_OP(VkPipelineColorBlendAttachmentState const &other)
{
	this->blendEnable         = other.blendEnable;
	this->srcColorBlendFactor = other.srcColorBlendFactor;
	this->dstColorBlendFactor = other.dstColorBlendFactor;
	this->colorBlendOp        = other.colorBlendOp;
	this->srcAlphaBlendFactor = other.srcAlphaBlendFactor;
	this->dstAlphaBlendFactor = other.dstAlphaBlendFactor;
	this->alphaBlendOp        = other.alphaBlendOp;
	this->colorWriteMask      = other.colorWriteMask;
}
bool VkPipelineColorBlendAttachmentState_OP::operator==(VkPipelineColorBlendAttachmentState_OP const &other) const
{
	// clang-format off
        return blendEnable == other.blendEnable
            && srcColorBlendFactor == other.srcColorBlendFactor
            && dstColorBlendFactor == other.dstColorBlendFactor
            && colorBlendOp == other.colorBlendOp
            && srcAlphaBlendFactor == other.srcAlphaBlendFactor
            && dstAlphaBlendFactor == other.dstAlphaBlendFactor
            && alphaBlendOp == other.alphaBlendOp
            && colorWriteMask == other.colorWriteMask;
	// clang-format on
}
bool VkPipelineColorBlendAttachmentState_OP::operator!=(VkPipelineColorBlendAttachmentState_OP const &other) const
{
	return !(*this == other);
}
bool VkPipelineColorBlendAttachmentState_OP::hash() const
{
	// clang-format off
        return blendEnable
            HASHC srcColorBlendFactor
            HASHC dstColorBlendFactor
            HASHC colorBlendOp
            HASHC srcAlphaBlendFactor
            HASHC dstAlphaBlendFactor
            HASHC alphaBlendOp
            HASHC colorWriteMask;
	// clang-format on
}

VkVertexInputBindingDescription_OP::VkVertexInputBindingDescription_OP(VkVertexInputBindingDescription const &other)
{
	this->binding   = other.binding;
	this->stride    = other.stride;
	this->inputRate = other.inputRate;
}
bool VkVertexInputBindingDescription_OP::operator==(VkVertexInputBindingDescription_OP const &other) const
{
	// clang-format off
        return binding == other.binding
            && stride == other.stride
            && inputRate == other.inputRate;
	// clang-format on
}
bool VkVertexInputBindingDescription_OP::operator!=(VkVertexInputBindingDescription_OP const &other) const
{
	return !(*this == other);
}
bool VkVertexInputBindingDescription_OP::hash() const
{
	// clang-format off
        return binding
            HASHC stride
            HASHC inputRate;
	// clang-format on
}

VkVertexInputAttributeDescription_OP::VkVertexInputAttributeDescription_OP(VkVertexInputAttributeDescription const &other)
{
	this->location = other.location;
	this->binding  = other.binding;
	this->format   = other.format;
	this->offset   = other.offset;
}
bool VkVertexInputAttributeDescription_OP::operator==(VkVertexInputAttributeDescription_OP const &other) const
{
	// clang-format off
        return location == other.location
            && binding == other.binding
            && format == other.format
            && offset == other.offset;
	// clang-format on
}
bool VkVertexInputAttributeDescription_OP::operator!=(VkVertexInputAttributeDescription_OP const &other) const
{
	return !(*this == other);
}
bool VkVertexInputAttributeDescription_OP::hash() const
{
	// clang-format off
        return location
            HASHC binding
            HASHC format
            HASHC offset;
	// clang-format on
}

VkViewport_OP::VkViewport_OP(VkViewport const &other)
{
	this->x        = other.x;
	this->y        = other.y;
	this->width    = other.width;
	this->height   = other.height;
	this->minDepth = other.minDepth;
	this->maxDepth = other.maxDepth;
}
bool VkViewport_OP::operator==(VkViewport_OP const &other) const
{
	// clang-format off
        return x == other.x
            && y == other.y
            && width == other.width
            && height == other.height
            && minDepth == other.minDepth
            && maxDepth == other.maxDepth;
	// clang-format on
}
bool VkViewport_OP::operator!=(VkViewport_OP const &other) const
{
	return !(*this == other);
}
bool VkViewport_OP::hash() const
{
	// clang-format off
        return x
            HASHC y
            HASHC width
            HASHC height
            HASHC minDepth
            HASHC maxDepth;
	// clang-format on
}

VkExtent2D_OP::VkExtent2D_OP(uint32_t width, uint32_t height)
{
	this->width  = width;
	this->height = height;
}
VkExtent2D_OP::VkExtent2D_OP(VkExtent2D const &other)
{
	this->width  = other.width;
	this->height = other.height;
}
bool VkExtent2D_OP::operator==(VkExtent2D_OP const &other) const
{
	// clang-format off
        return width == other.width
            && height == other.height;
	// clang-format on
}
bool VkExtent2D_OP::operator!=(VkExtent2D_OP const &other) const
{
	return !(*this == other);
}
bool VkExtent2D_OP::hash() const
{
	// clang-format off
        return width
            HASHC height;
	// clang-format on
}

}        // namespace vka