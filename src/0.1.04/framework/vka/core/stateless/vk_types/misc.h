#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include "../utility/macros.h"
#include <glm/glm.hpp>

namespace vka
{

struct SubmitSynchronizationInfo
{
	std::vector<VkSemaphore>          waitSemaphores{};
	std::vector<VkSemaphore>          signalSemaphores{};
	std::vector<VkPipelineStageFlags> waitDstStageMask{};
	VkFence                           signalFence = VK_NULL_HANDLE;
};

template <typename T>
struct Rect2D
{
	T    x;
	T    y;
	T    width;
	T    height;
	bool operator==(const Rect2D &other) const
	{
		return x == other.x && y == other.y && width == other.width && height == other.height;
	}
};


enum ClearValueType
{
	CLEAR_VALUE_NONE,
	CLEAR_VALUE_FLOAT,
	CLEAR_VALUE_INT,
	CLEAR_VALUE_UINT,
	CLEAR_VALUE_DEPTH_STENCIL
};
struct ClearValue
{
	ClearValueType type;
	VkClearValue   value;

	ClearValue();

	ClearValue(glm::vec4 color);

	ClearValue(float r, float g, float b, float a);
	ClearValue(int32_t r, int32_t g, int32_t b, int32_t a);
	ClearValue(uint32_t r, uint32_t g, uint32_t b, uint32_t a);
	ClearValue(float depth, uint32_t stencil);
	bool operator==(ClearValue const &other) const;
	bool operator!=(ClearValue const &other) const;
	bool hash() const;

	bool              joinable(ClearValue const &previous) const;
	static ClearValue white();

	static ClearValue black();
	static ClearValue max_depth();
	static ClearValue none();
};




// RenderPassStuff

struct VkAttachmentReference_OP : public VkAttachmentReference
{
	VkAttachmentReference_OP() = default;
	VkAttachmentReference_OP(VkAttachmentReference const &other);
	bool operator==(VkAttachmentReference_OP const &other) const;
	bool operator!=(VkAttachmentReference_OP const &other) const;
	bool hash() const;
};

struct SubpassDescription
{
	VkSubpassDescriptionFlags             flags;
	VkPipelineBindPoint                   pipelineBindPoint;
	std::vector<VkAttachmentReference_OP> inputAttachments;
	std::vector<VkAttachmentReference_OP> colorAttachments;
	VkAttachmentReference_OP              depthStencilAttachment;
	bool                                  hasDepthStencilAttachment = false;
	std::vector<uint32_t>                 preserveAttachments;
	SubpassDescription() = default;
	SubpassDescription(VkSubpassDescription const &other);
	VkSubpassDescription getVulkanStruct() const;

	bool operator==(SubpassDescription const &other) const;
	bool operator!=(SubpassDescription const &other) const;
	bool hash() const;
};

struct PipelineMultisampleStateCreateInfo_OP : public VkPipelineMultisampleStateCreateInfo
{
	PipelineMultisampleStateCreateInfo_OP() = default;
	PipelineMultisampleStateCreateInfo_OP(VkPipelineMultisampleStateCreateInfo const &other);
	bool operator==(PipelineMultisampleStateCreateInfo_OP const &other) const;
	bool operator!=(PipelineMultisampleStateCreateInfo_OP const &other) const;
	bool hash() const;
};

struct VkRect2D_OP : public VkRect2D
{
	VkRect2D_OP() = default;
	VkRect2D_OP(uint32_t x, uint32_t y, uint32_t width, uint32_t height);
	VkRect2D_OP(VkRect2D const &other);

	VkRect2D_OP(VkExtent2D const &extent);
	bool operator==(VkRect2D_OP const &other) const;
	bool        isValid(VkExtent2D const &extent) const;
	VkRect2D_OP        operator*(Rect2D<float> const &other) const;
	static VkRect2D_OP absRegion(VkRect2D_OP const &other, Rect2D<float> const &relative);

	static Rect2D<float> relRegion(VkRect2D_OP const &outer, VkRect2D_OP const &inner);


	void operator*=(Rect2D<float> const &other);
	bool operator!=(VkRect2D_OP const &other) const;
	bool hash() const;
};
//----------------------------------------------------------------

struct VkAttachmentDescription_OP : public VkAttachmentDescription
{
	VkAttachmentDescription_OP() = default;
	VkAttachmentDescription_OP(VkAttachmentDescription const &other);
	bool operator==(VkAttachmentDescription_OP const &other) const;
	bool operator!=(VkAttachmentDescription_OP const &other) const;
	bool hash() const;
	bool joinable(VkAttachmentDescription_OP const &previous) const;
};

struct VkSubpassDependency_OP : public VkSubpassDependency
{
	VkSubpassDependency_OP() = default;
	VkSubpassDependency_OP(VkSubpassDependency const &other);
	bool operator==(VkSubpassDependency_OP const &other) const;
	bool operator!=(VkSubpassDependency_OP const &other) const;
	bool hash() const;
};

struct VkDescriptorSetLayoutBinding_OP : public VkDescriptorSetLayoutBinding
{
	VkDescriptorSetLayoutBinding_OP() = default;
	VkDescriptorSetLayoutBinding_OP(VkDescriptorSetLayoutBinding const &other);
	bool operator==(VkDescriptorSetLayoutBinding_OP const &other) const;
	bool operator!=(VkDescriptorSetLayoutBinding_OP const &other) const;
	bool hash() const;
};


struct VkPushConstantRange_OP : public VkPushConstantRange
{
	VkPushConstantRange_OP() = default;
	VkPushConstantRange_OP(VkPushConstantRange const &other);
	bool operator==(VkPushConstantRange_OP const &other) const;
	bool operator!=(VkPushConstantRange_OP const &other) const;
	bool hash() const;
};

struct VkPipelineInputAssemblyStateCreateInfo_OP : public VkPipelineInputAssemblyStateCreateInfo
{
	VkPipelineInputAssemblyStateCreateInfo_OP() = default;
	VkPipelineInputAssemblyStateCreateInfo_OP(VkPipelineInputAssemblyStateCreateInfo const &other);
	bool operator==(VkPipelineInputAssemblyStateCreateInfo_OP const &other) const;
	bool operator!=(VkPipelineInputAssemblyStateCreateInfo_OP const &other) const;
	bool hash() const;
};
struct VkPipelineTessellationStateCreateInfo_OP : public VkPipelineTessellationStateCreateInfo
{
	VkPipelineTessellationStateCreateInfo_OP() = default;
	VkPipelineTessellationStateCreateInfo_OP(VkPipelineTessellationStateCreateInfo const &other);
	bool operator==(VkPipelineTessellationStateCreateInfo_OP const &other) const;
	bool operator!=(VkPipelineTessellationStateCreateInfo_OP const &other) const;
	bool hash() const;
};

struct VkPipelineRasterizationStateCreateInfo_OP : public VkPipelineRasterizationStateCreateInfo
{
	VkPipelineRasterizationStateCreateInfo_OP() = default;
	VkPipelineRasterizationStateCreateInfo_OP(VkPipelineRasterizationStateCreateInfo const &other);
	bool operator==(VkPipelineRasterizationStateCreateInfo_OP const &other) const;
	bool operator!=(VkPipelineRasterizationStateCreateInfo_OP const &other) const;
	bool hash() const;
};
struct VkStencilOpState_OP : public VkStencilOpState
{
	VkStencilOpState_OP() = default;
	VkStencilOpState_OP(VkStencilOpState const &other);

	bool operator==(VkStencilOpState_OP const &other) const;
	bool operator!=(VkStencilOpState_OP const &other) const;
	bool hash() const;
};

struct VkPipelineDepthStencilStateCreateInfo_OP : public VkPipelineDepthStencilStateCreateInfo
{
	VkPipelineDepthStencilStateCreateInfo_OP() = default;
	VkPipelineDepthStencilStateCreateInfo_OP(VkPipelineDepthStencilStateCreateInfo const &other);
	bool operator==(VkPipelineDepthStencilStateCreateInfo_OP const &other) const;
	bool operator!=(VkPipelineDepthStencilStateCreateInfo_OP const &other) const;
	bool hash() const;
};
struct VkPipelineColorBlendAttachmentState_OP : public VkPipelineColorBlendAttachmentState
{
	VkPipelineColorBlendAttachmentState_OP() = default;
	VkPipelineColorBlendAttachmentState_OP(VkPipelineColorBlendAttachmentState const &other);
	bool operator==(VkPipelineColorBlendAttachmentState_OP const &other) const;
	bool operator!=(VkPipelineColorBlendAttachmentState_OP const &other) const;
	bool hash() const;
};

struct VkVertexInputBindingDescription_OP : public VkVertexInputBindingDescription
{
	VkVertexInputBindingDescription_OP() = default;
	VkVertexInputBindingDescription_OP(VkVertexInputBindingDescription const &other);
	bool operator==(VkVertexInputBindingDescription_OP const &other) const;
	bool operator!=(VkVertexInputBindingDescription_OP const &other) const;
	bool hash() const;
};

struct VkVertexInputAttributeDescription_OP : public VkVertexInputAttributeDescription
{
	VkVertexInputAttributeDescription_OP() = default;
	VkVertexInputAttributeDescription_OP(VkVertexInputAttributeDescription const &other);
	bool operator==(VkVertexInputAttributeDescription_OP const &other) const;
	bool operator!=(VkVertexInputAttributeDescription_OP const &other) const;
	bool hash() const;
};

struct VkViewport_OP : public VkViewport
{
	VkViewport_OP() = default;
	VkViewport_OP(VkViewport const &other);
	bool operator==(VkViewport_OP const &other) const;
	bool operator!=(VkViewport_OP const &other) const;
	bool hash() const;
};

class VkExtent2D_OP : public VkExtent2D
{
  public:
	VkExtent2D_OP() = default;
	VkExtent2D_OP(uint32_t width, uint32_t height);
	VkExtent2D_OP(VkExtent2D const &other);
	bool operator==(VkExtent2D_OP const &other) const;
	bool operator!=(VkExtent2D_OP const &other) const;
	bool hash() const;
};

}        // namespace vka
DECLARE_HASH(vka::VkExtent2D_OP, hash)
DECLARE_HASH(vka::PipelineMultisampleStateCreateInfo_OP, hash)
DECLARE_HASH(vka::SubpassDescription, hash)
DECLARE_HASH(vka::VkRect2D_OP, hash)
DECLARE_HASH(vka::VkAttachmentDescription_OP, hash)
DECLARE_HASH(vka::VkAttachmentReference_OP, hash)
DECLARE_HASH(vka::VkSubpassDependency_OP, hash)
DECLARE_HASH(vka::VkDescriptorSetLayoutBinding_OP, hash)
DECLARE_HASH(vka::VkPushConstantRange_OP, hash)
DECLARE_HASH(vka::VkPipelineInputAssemblyStateCreateInfo_OP, hash)
DECLARE_HASH(vka::VkPipelineTessellationStateCreateInfo_OP, hash)
DECLARE_HASH(vka::VkPipelineRasterizationStateCreateInfo_OP, hash)
DECLARE_HASH(vka::VkPipelineDepthStencilStateCreateInfo_OP, hash)
DECLARE_HASH(vka::VkStencilOpState_OP, hash)
DECLARE_HASH(vka::VkPipelineColorBlendAttachmentState_OP, hash)
DECLARE_HASH(vka::VkVertexInputBindingDescription_OP, hash)
DECLARE_HASH(vka::VkVertexInputAttributeDescription_OP, hash)
DECLARE_HASH(vka::VkViewport_OP, hash)