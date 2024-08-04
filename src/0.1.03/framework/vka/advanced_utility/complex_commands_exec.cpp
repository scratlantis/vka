#include "complex_commands.h"
#include <vka/core/core_utility/general_commands.h>
#include <vka/globals.h>


namespace vka
{

void ComputeCmd::exec(CmdBuffer cmdBuf) const
{
	cmdClearState(cmdBuf);
	CmdBufferState newState;
	try
	{
		newState = getCmdBufferState();
	}
	catch (const ShaderNotFoundException &e)
	{
		printVka("Shader not found: %s", e.what());
		return;
	}
	cmdBuf->state = newState;
	cmdBindPipeline(cmdBuf);
	cmdPushDescriptors(cmdBuf, 0, descriptors);        // only one descriptor set for now
	cmdPushConstants(cmdBuf, pushConstantsSizes, pushConstantsData.data());
	cmdDispatch(cmdBuf, workGroupCount);
}

void DrawCmd::exec(CmdBuffer cmdBuf) const
{
	VKA_ASSERT(pipelineDef.subpass == 0);
	CmdBufferState newState;
	try
	{
		newState = getCmdBufferState(cmdBuf->state);
	}
	catch (const ShaderNotFoundException& e)
	{
		printVka("Shader not found: %s", e.what());
		return;
	}
	uint32_t    diffBits       = cmdBuf->state.calculateDifferenceBits(newState);

	// Fetch descriptor image layout transforms
	std::vector<Image>         pendingTransformImages;
	std::vector<VkImageLayout> pendingTransformLayouts;
	for (size_t i = 0; i < descriptors.size(); i++)
	{
		descriptors[i].getLayoutTransforms(pendingTransformImages, pendingTransformLayouts);
	}
	if (!pendingTransformImages.empty() && !(diffBits & RENDER_STATE_ACTION_BIT_START_RENDER_PASS))
	{
		diffBits |= RENDER_STATE_ACTION_BIT_END_RENDER_PASS;
		diffBits |= RENDER_STATE_ACTION_BIT_START_RENDER_PASS;
		for (size_t i = 0; i < newState.clearValues.size(); i++)
		{
			newState.clearValues[i] = ClearValue::none();
		}
	}

	// End render pass if needed
	if (diffBits & RENDER_STATE_ACTION_BIT_END_RENDER_PASS)
	{
		cmdEndRenderPass(cmdBuf);
	}

	// Perform image layout transitions
	// Descriptors
	for (size_t i = 0; i < pendingTransformImages.size(); i++)
	{
		cmdTransitionLayout(cmdBuf, pendingTransformImages[i], pendingTransformLayouts[i]);
	}
	// Attachments
	if (diffBits & RENDER_STATE_ACTION_BIT_START_RENDER_PASS)
	{
		VKA_ASSERT(attachments.size() == pipelineDef.renderPassDefinition.attachmentDescriptions.size())
		for (size_t i = 0; i < attachments.size(); i++)
		{
			cmdTransitionLayout(cmdBuf, attachments[i], pipelineDef.renderPassDefinition.attachmentDescriptions[i].initialLayout);
			attachments[i]->setLayout(pipelineDef.renderPassDefinition.attachmentDescriptions[i].finalLayout);
		}
	}
	// Aquire new render state
	cmdBuf->state = newState;
	// Start render pass if needed
	if (diffBits & RENDER_STATE_ACTION_BIT_START_RENDER_PASS)
	{
		cmdStartRenderPass(cmdBuf, cmdBuf->state.renderPass, cmdBuf->state.framebuffer, cmdBuf->state.renderArea, cmdBuf->state.getClearValues());
	}
	// Bind pipeline if needed
	if (diffBits & RENDER_STATE_ACTION_BIT_BIND_PIPELINE)
	{
		cmdBindPipeline(cmdBuf);
	}
	// for now, only one descriptor set, always rebind
	if (!descriptors.empty())
	{
		cmdPushDescriptors(cmdBuf, 0, descriptors);
	}
	cmdPushConstants(cmdBuf, pushConstantsSizes, pushConstantsData.data());

	if (diffBits & RENDER_STATE_ACTION_BIT_BIND_VERTEX_BUFFER && !cmdBuf->state.vertexBuffers.empty())
	{
		cmdBindVertexBuffers(cmdBuf);
	}

	if (cmdBuf->state.indexBuffer)
	{
		cmdBindIndexBuffer(cmdBuf, surf.offset);
		cmdDrawIndexed(cmdBuf, surf.count, instanceCount, 0, 0, 0);
	}
	else
	{
		cmdDraw(cmdBuf, surf.count, instanceCount, 0, 0);
	}
}
}        // namespace vka