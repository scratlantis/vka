#pragma once
#include <vka/core/stateless/vk_types/misc.h>
#include "../Resource.h"
#include <vka/core/resources/cachable/PipelineLayout.h>
#include <vka/core/resources/cachable/RenderPass.h>
namespace vka
{
class CmdBufferVK_R : public Resource_T<VkCommandBuffer>
{
  public:
	CmdBufferVK_R(VkCommandBuffer handle, VkCommandPool cmdPool) :
	    Resource_T<VkCommandBuffer>(handle), cmdPool(cmdPool)
	{}
	void free() override;

  private:
	VkCommandPool cmdPool;
};

enum CmdBufferCapabitlityBits
{
	CMD_BUF_CAPABILITY_BIT_TRANSFER = 1 << 0,
	CMD_BUF_CAPABILITY_BIT_COMPUTE  = 1 << 1,
	CMD_BUF_CAPABILITY_BIT_GRAPHICS = 1 << 2,
	CMD_BUF_CAPABILITY_BIT_PRESENT  = 1 << 3
};

enum CmdBufferCapabitlityMask
{
	CMD_BUF_CAPABILITY_MASK_NONE = 0,
	CMD_BUF_CAPABILITY_MASK_TRANSFER =
	    CMD_BUF_CAPABILITY_BIT_TRANSFER,
	CMD_BUF_CAPABILITY_MASK_COMPUTE =
	    CMD_BUF_CAPABILITY_BIT_TRANSFER |
	    CMD_BUF_CAPABILITY_BIT_COMPUTE,
	CMD_BUF_CAPABILITY_MASK_UNIVERSAL =
	    CMD_BUF_CAPABILITY_BIT_TRANSFER |
	    CMD_BUF_CAPABILITY_BIT_COMPUTE |
	    CMD_BUF_CAPABILITY_BIT_GRAPHICS |
	    CMD_BUF_CAPABILITY_BIT_PRESENT
};

enum CmdBufferStateBits
{
	CMD_BUF_STATE_BITS_RECORDING      = 1 << 0,
	CMD_BUF_STATE_BITS_BOUND_PIPELINE = 1 << 1
};

enum RenderStateActionBits
{
	RENDER_STATE_ACTION_BIT_START_RENDER_PASS  = 1 << 0,
	RENDER_STATE_ACTION_BIT_END_RENDER_PASS    = 1 << 1,
	RENDER_STATE_ACTION_BIT_NEXT_SUBPASS       = 1 << 2,
	RENDER_STATE_ACTION_BIT_BIND_PIPELINE      = 1 << 3,
	RENDER_STATE_ACTION_BIT_BIND_VERTEX_BUFFER = 1 << 4,
};

class Buffer_R;

enum CmdBufferStateType
{
	CMD_BUFFER_STATE_NONE,
	CMD_BUFFER_STATE_COMPUTE,
	CMD_BUFFER_STATE_RASTERIZATION
};

struct CmdBufferState
{
	CmdBufferStateType type = CMD_BUFFER_STATE_NONE;
	// -> RenderPassBegin
	VkFramebuffer           framebuffer;
	std::vector<ClearValue> clearValues;
	VkRect2D_OP             renderArea;
	VkRenderPass            renderPass;
	RenderPassDefinition	renderPassDef; // Must match renderPass

	// -> Subpass
	uint32_t subpassIdx;

	// -> BindPipeline
	VkPipeline               pipeline;
	VkPipelineLayout		 pipelineLayout;
	PipelineLayoutDefinition pipelineLayoutDef; // Must match pipelineLayout
	VkPipelineBindPoint      bindPoint;

	// -> Bind Buffers
	std::vector<const Buffer_R *> vertexBuffers;
	Buffer_R                     *indexBuffer;

	void clear()
	{
		type = CMD_BUFFER_STATE_NONE;
		renderPass = VK_NULL_HANDLE;
		framebuffer = VK_NULL_HANDLE;
		clearValues.clear();
		renderArea = {};
		pipeline = VK_NULL_HANDLE;
		pipelineLayout = VK_NULL_HANDLE;
		vertexBuffers.clear();
		indexBuffer = nullptr;
	}

	uint32_t calculateDifferenceBits(const CmdBufferState &other) const
	{
		uint32_t diffBits = 0;
		// todo subpasses
		if (renderPass == VK_NULL_HANDLE)
		{
			diffBits |= RENDER_STATE_ACTION_BIT_START_RENDER_PASS;
		}
		else if (other.renderPass != renderPass || !cmpVector(clearValues, other.clearValues) || renderArea != other.renderArea)
		{
			diffBits |= RENDER_STATE_ACTION_BIT_END_RENDER_PASS;
			diffBits |= RENDER_STATE_ACTION_BIT_START_RENDER_PASS;
		}
		if (pipeline != other.pipeline)
		{
			diffBits |= RENDER_STATE_ACTION_BIT_BIND_PIPELINE;
		}
		if (!cmpVector(vertexBuffers, other.vertexBuffers))
		{
			diffBits |= RENDER_STATE_ACTION_BIT_BIND_VERTEX_BUFFER;
		}
		return diffBits;
	}

	std::vector<VkClearValue> getClearValues() const;
};

class CmdBuffer_R : public Resource_T<VkCommandBuffer>
{
  protected:
	CmdBufferVK_R *res                = nullptr;

  public:
	// State
	uint32_t                 stateBits;
	CmdBufferCapabitlityMask capability;

	CmdBufferState state;

	CmdBuffer_R(IResourcePool *pPool) :
	    Resource_T<VkCommandBuffer>(VK_NULL_HANDLE)
	{
		state = {};
		capability  = CMD_BUF_CAPABILITY_MASK_NONE;
		stateBits   = 0;
		track(pPool);
	}
	CmdBuffer_R(IResourcePool *pPool, CmdBufferCapabitlityMask capability, VkCommandBufferUsageFlags usage, VkCommandBufferLevel level, uint32_t poolIdx) :
	    CmdBuffer_R(pPool)
	{
		createHandles(capability, usage, level, poolIdx);
	}
	~CmdBuffer_R()
	{
		free();
	}

	void CmdBuffer_R::end()
	{
		if (stateBits & CMD_BUF_STATE_BITS_RECORDING)
		{
			VK_CHECK(vkEndCommandBuffer(handle));
			stateBits &= ~CMD_BUF_STATE_BITS_RECORDING;
		}
	}

	void   createHandles(CmdBufferCapabitlityMask capability, VkCommandBufferUsageFlags usage, VkCommandBufferLevel level, uint32_t poolIdx);
	hash_t hash() const override;
	void   free() override;
	void   track(IResourcePool *pPool) override;
};
}        // namespace vka