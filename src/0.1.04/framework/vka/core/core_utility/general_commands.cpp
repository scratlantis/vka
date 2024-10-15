#include "general_commands.h"
#include <vka/globals.h>
#include "buffer_utility.h"
#include <vka/core/resources/Descriptor.h>
namespace vka
{
void cmdClearState(CmdBuffer cmdBuf)
{
	if (cmdBuf->state.renderPass != VK_NULL_HANDLE)
	{
		cmdEndRenderPass(cmdBuf);
	}
	cmdBuf->state.clear();
}

// Buffer
void cmdCopyBuffer(CmdBuffer cmdBuf, BufferRef src, BufferRef dst)
{
	VkBufferCopy copyRegion{src->getRange().offset, dst->getRange().offset, src->getSize()};
	cmdClearState(cmdBuf);
    vkCmdCopyBuffer(cmdBuf->getHandle(), src->getHandle(), dst->getHandle(), 1, &copyRegion);
}

void cmdCopyBufferRegion(CmdBuffer cmdBuf, BufferRef src, BufferRef dst, uint32_t srcOffset, uint32_t dstOffset, VkDeviceSize size)
{
	VkBufferCopy copyRegion;
	copyRegion.srcOffset = src->getRange().offset + srcOffset;
	copyRegion.dstOffset = dst->getRange().offset + dstOffset;
	copyRegion.size      = std::min(src->getSize() - srcOffset, size);
    VKA_ASSERT(copyRegion.size <= dst->getSize() - dstOffset);
	cmdClearState(cmdBuf);
	vkCmdCopyBuffer(cmdBuf->getHandle(), src->getHandle(), dst->getHandle(), 1, &copyRegion);
}

void cmdUpload(CmdBuffer cmdBuf, Buffer buf)
{
	VKA_ASSERT(buf->getMemoryType() != VMA_MEMORY_USAGE_GPU_ONLY);
    buf->changeMemoryType(VMA_MEMORY_USAGE_GPU_ONLY);
    buf->addUsage(VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    const Buffer_R localBuf = buf->recreate();
	cmdCopyBuffer(cmdBuf, &localBuf, buf);
}

void cmdWriteCopy(CmdBuffer cmdBuf, Buffer buf, const void* data, VkDeviceSize size)
{
	buf->addUsage(VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    buf->changeSize(size);
	buf->recreate();
	Buffer stagingBuf = createStagingBuffer();
	write(stagingBuf, data, size);
	cmdCopyBuffer(cmdBuf, stagingBuf, buf);
}

void cmdUploadCopy(CmdBuffer cmdBuf, Buffer src, Buffer dst)
{
	dst->addUsage(VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    dst->changeMemoryType(VMA_MEMORY_USAGE_GPU_ONLY);
	dst->changeSize(src->getSize());
	dst->recreate();
	cmdCopyBuffer(cmdBuf, src, dst);
}


void cmdFillBuffer(CmdBuffer cmdBuf, Buffer dst, VkDeviceSize offset, VkDeviceSize size, uint32_t data)
{
	cmdClearState(cmdBuf);
	vkCmdFillBuffer(cmdBuf->getHandle(), dst->getHandle(), offset, size, data);
}

void cmdFillBuffer(CmdBuffer cmdBuf, Buffer dst, uint32_t data)
{
	cmdClearState(cmdBuf);
	vkCmdFillBuffer(cmdBuf->getHandle(), dst->getHandle(), 0, VK_WHOLE_SIZE, data);
}



// Image
void cmdImageMemoryBarrier(CmdBuffer cmdBuf, Image image, VkImageLayout newLayout, uint32_t baseLayer, uint32_t layerCount)
{
    VkImageMemoryBarrier memory_barrier{VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
    memory_barrier.oldLayout                       = image->getLayout();
    memory_barrier.newLayout                       = newLayout;
    memory_barrier.image                           = image->getHandle();
    memory_barrier.subresourceRange.aspectMask     = getAspectFlags(image->getFormat());
    memory_barrier.subresourceRange.baseMipLevel   = 0;
    memory_barrier.subresourceRange.levelCount     = image->getMipLevels();
    memory_barrier.subresourceRange.baseArrayLayer = baseLayer;
    memory_barrier.subresourceRange.layerCount     = layerCount;
    memory_barrier.srcAccessMask                   = getAccessFlags(image->getLayout());
    memory_barrier.dstAccessMask                   = getAccessFlags(newLayout);
    VkPipelineStageFlags src_stage                 = getStageFlags(image->getLayout());
    VkPipelineStageFlags dst_stage                 = getStageFlags(newLayout);
    cmdClearState(cmdBuf);
    vkCmdPipelineBarrier(
        cmdBuf->getHandle(),
        src_stage, dst_stage,
        0,
        0, nullptr,
        0, nullptr,
        1, &memory_barrier);
    image->setLayout(newLayout);
}

void cmdTransitionLayout(CmdBuffer cmdBuf, Image image, VkImageLayout newLayout, uint32_t baseLayer, uint32_t layerCount)
{
    if (image->getLayout() != newLayout && newLayout != VK_IMAGE_LAYOUT_UNDEFINED && newLayout != VK_IMAGE_LAYOUT_PREINITIALIZED)
    {
        cmdImageMemoryBarrier(cmdBuf, image, newLayout, baseLayer, layerCount);
    }
}

void cmdCopyBufferToImage(CmdBuffer cmdBuf, Buffer src, Image dst, uint32_t layer, uint32_t mipLevel)
{
    VkDeviceSize dstSize = dst->getMemorySize();
    VKA_ASSERT(src->getSize() == dstSize);
    VkBufferImageCopy region{};
    region.bufferOffset                    = 0;
    region.bufferRowLength                 = 0;
    region.bufferImageHeight               = 0;
    region.imageSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel       = mipLevel;
    region.imageSubresource.baseArrayLayer = layer;
    region.imageSubresource.layerCount     = 1;
    region.imageOffset                     = {0, 0, 0};
    region.imageExtent                     = dst->getExtent();
    cmdClearState(cmdBuf);
    vkCmdCopyBufferToImage(cmdBuf->getHandle(), src->getHandle(), dst->getHandle(), dst->getLayout(), 1, &region);
}

void cmdUploadImageData(CmdBuffer cmdBuf, void *data, size_t dataSize, Image dst, VkImageLayout finalLayout, uint32_t layer, uint32_t mipLevel)
{
	Buffer stagingBuffer = createStagingBuffer();
    write(stagingBuffer, data, dataSize);
    cmdTransitionLayout(cmdBuf, dst, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, layer, 1);
    cmdCopyBufferToImage(cmdBuf, stagingBuffer, dst, layer, mipLevel);
    cmdTransitionLayout(cmdBuf, dst, finalLayout, layer, 1);
}

void cmdCopyImage(CmdBuffer cmdBuf, Image src, VkImageLayout srcNewLayout, Image dst, VkImageLayout dstNewLayout,
                     ImageSubresourceRange srcSubRange, ImageSubresourceRange dstSubRange)
{
    cmdTransitionLayout(cmdBuf, src, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, srcSubRange.baseArrayLayer, srcSubRange.layerCount);
    cmdTransitionLayout(cmdBuf, dst, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, dstSubRange.baseArrayLayer, dstSubRange.layerCount);

    VkImageSubresourceLayers subresourceLayerSrc{};
    subresourceLayerSrc.aspectMask     = getAspectFlags(src->getFormat());
    subresourceLayerSrc.mipLevel       = srcSubRange.mipLevel;
    subresourceLayerSrc.baseArrayLayer = srcSubRange.baseArrayLayer;
    subresourceLayerSrc.layerCount     = srcSubRange.layerCount;

    VkImageSubresourceLayers subresourceLayerDst{};
    subresourceLayerDst.aspectMask     = getAspectFlags(dst->getFormat());
    subresourceLayerDst.mipLevel       = dstSubRange.mipLevel;
    subresourceLayerDst.baseArrayLayer = dstSubRange.baseArrayLayer;
    subresourceLayerDst.layerCount     = dstSubRange.layerCount;

    VkOffset3D offset{};
    offset.x = 0;
    offset.y = 0;
    offset.z = 0;

    VkImageCopy imageCopy{};
    imageCopy.srcSubresource = subresourceLayerSrc;
    imageCopy.srcOffset      = offset;
    imageCopy.dstSubresource = subresourceLayerDst;
    imageCopy.dstOffset      = offset;
    imageCopy.extent         = src->getExtent();
    cmdClearState(cmdBuf);
    vkCmdCopyImage(cmdBuf->getHandle(), src->getHandle(), src->getLayout(), dst->getHandle(), dst->getLayout(), 1, &imageCopy);
    cmdTransitionLayout(cmdBuf, src, srcNewLayout, srcSubRange.baseArrayLayer, srcSubRange.layerCount);
    cmdTransitionLayout(cmdBuf, dst, dstNewLayout, dstSubRange.baseArrayLayer, dstSubRange.layerCount);
}

void cmdCopyImage(CmdBuffer cmdBuf, Image src, Image dst)
{
    cmdCopyImage(cmdBuf, src, src->getLayout(), dst, dst->getLayout());
}

// Acceleration Structure
void cmdBuildAccelerationStructure(CmdBuffer cmdBuf, BLAS dst, Buffer scratchBuffer, VkAccelerationStructureKHR src)
{
	LOAD_CMD_VK_DEVICE(vkCmdBuildAccelerationStructuresKHR, gState.device.logical);
    dst->configureScratchBuffer(scratchBuffer);
	scratchBuffer->recreate();
	VkAccelerationStructureBuildGeometryInfoKHR buildInfo = dst->getBuildInfo(src, scratchBuffer);
    cmdClearState(cmdBuf);
	pvkCmdBuildAccelerationStructuresKHR(cmdBuf->getHandle(), 1, &buildInfo, dst->getBuildRangePtrs().data());
	dst->setBuilt(true);
}

void cmdBuildAccelerationStructure(CmdBuffer cmdBuf, TLAS dst, Buffer instanceBuffer, Buffer scratchBuffer, VkAccelerationStructureKHR src)
{
	LOAD_CMD_VK_DEVICE(vkCmdBuildAccelerationStructuresKHR, gState.device.logical);
    dst->setInstanceData(instanceBuffer);
	dst->configureScratchBuffer(scratchBuffer);
	scratchBuffer->recreate();
	VkAccelerationStructureBuildGeometryInfoKHR buildInfo = dst->getBuildInfo(src, scratchBuffer);
	cmdClearState(cmdBuf);
	pvkCmdBuildAccelerationStructuresKHR(cmdBuf->getHandle(), 1, &buildInfo, dst->getBuildRangePtrs().data());
	dst->setBuilt(true);
}


// General
void cmdBarrier(CmdBuffer cmdBuf, VkPipelineStageFlags srcStage, VkPipelineStageFlags dstStage, VkAccessFlags srcAccesFlags, VkAccessFlags dstAccesFlags)
{
    VkMemoryBarrier barrier{VK_STRUCTURE_TYPE_MEMORY_BARRIER};
    barrier.srcAccessMask = srcAccesFlags;
    barrier.dstAccessMask = dstAccesFlags;
    cmdClearState(cmdBuf);
    vkCmdPipelineBarrier(cmdBuf->getHandle(), srcStage, dstStage, 0, 1, &barrier, 0, nullptr, 0, nullptr);
}

// Pipelines, render/dispatch
void cmdStartRenderPass(CmdBuffer cmdBuf, VkRenderPass renderpass, VkFramebuffer framebuffer, VkRect2D renderArea, std::vector<VkClearValue> clearValues)
{
    VkCommandBuffer       handle = cmdBuf->getHandle();
    VkRenderPassBeginInfo renderPassBeginInfo;
    renderPassBeginInfo.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginInfo.pNext           = nullptr;
    renderPassBeginInfo.renderPass      = renderpass;
    renderPassBeginInfo.framebuffer     = framebuffer;
    renderPassBeginInfo.renderArea      = renderArea;
    renderPassBeginInfo.clearValueCount = clearValues.size();
    renderPassBeginInfo.pClearValues    = clearValues.data();

    VkExtent2D extent = renderArea.extent;
    vkCmdBeginRenderPass(handle, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    VkViewport viewport;
    viewport.x        = renderArea.offset.x;
    viewport.y        = renderArea.offset.y;
    viewport.width    = extent.width;
    viewport.height   = extent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(handle, 0, 1, &viewport);
    VkRect2D scissor;
    scissor.offset = renderArea.offset;
    scissor.extent = {extent.width, extent.height};
    vkCmdSetScissor(handle, 0, 1, &scissor);
}

void cmdStartRenderPass(CmdBuffer cmdBuf, VkRenderPass renderpass, VkFramebuffer framebuffer, std::vector<VkClearValue> clearValues)
{
	cmdStartRenderPass(cmdBuf, renderpass, framebuffer, {{0, 0}, gState.io.extent}, clearValues);
}
void cmdEndRenderPass(CmdBuffer cmdBuf)
{
    vkCmdEndRenderPass(cmdBuf->getHandle());
    cmdBuf->state.renderPass = VK_NULL_HANDLE;
}

// bind, dispatch/draw, push desc, push constants
void cmdBindPipeline(CmdBuffer cmdBuf)
{
    vkCmdBindPipeline(cmdBuf->getHandle(), cmdBuf->state.bindPoint, cmdBuf->state.pipeline);
    cmdBuf->stateBits |= CMD_BUF_STATE_BITS_BOUND_PIPELINE;
}

void cmdPushDescriptors(CmdBuffer cmdBuf, uint32_t setIdx, std::vector<Descriptor> desc)
{
    LOAD_CMD_VK_DEVICE(vkCmdPushDescriptorSetKHR, gState.device.logical);
    VKA_ASSERT(cmdBuf->stateBits & CMD_BUF_STATE_BITS_BOUND_PIPELINE);
    VKA_ASSERT(cmdBuf->state.pipelineLayoutDef.descSetLayoutDef.size() > setIdx);

    std::vector<VkWriteDescriptorSet>   writes(VKA_COUNT(desc));
    uint32_t bufferInfoCount = 1;
    uint32_t imageInfoCount  = 1;
    uint32_t accelerationStructureWriteCount = 1;
	for (size_t i = 0; i < writes.size(); i++)
	{
		desc[i].countStructures(bufferInfoCount, imageInfoCount, accelerationStructureWriteCount);
	}
	std::vector<VkDescriptorBufferInfo> bufferInfos(bufferInfoCount);
	std::vector<VkDescriptorImageInfo>  imageInfos(imageInfoCount);
	std::vector<VkWriteDescriptorSetAccelerationStructureKHR> asWrites(accelerationStructureWriteCount);
    VkDescriptorBufferInfo             *pBuffInfo  = &bufferInfos[0];
    VkDescriptorImageInfo              *pImageInfo = &imageInfos[0];
	VkWriteDescriptorSetAccelerationStructureKHR *pAccelerationStructureWrite = &asWrites[0];
    for (size_t i = 0; i < writes.size(); i++)
    {
        writes[i]                 = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
        writes[i].dstBinding      = i;
		desc[i].writeDescriptorInfo(writes[i], pBuffInfo, pImageInfo, pAccelerationStructureWrite);
    }
    pvkCmdPushDescriptorSetKHR(cmdBuf->getHandle(), cmdBuf->state.bindPoint, cmdBuf->state.pipelineLayout, setIdx, writes.size(), writes.data());
}

void cmdPushConstants(CmdBuffer cmdBuf, VkShaderStageFlags shaderStage, uint32_t offset, uint32_t size, const void *data)
{
    VKA_ASSERT(cmdBuf->stateBits & CMD_BUF_STATE_BITS_BOUND_PIPELINE);
	vkCmdPushConstants(cmdBuf->getHandle(), cmdBuf->state.pipelineLayout, shaderStage, offset, size, data);
}

void cmdPushConstants(CmdBuffer cmdBuf, const std::vector<uint32_t> &pcSizes, const void *data)
{
	VKA_ASSERT(cmdBuf->stateBits & CMD_BUF_STATE_BITS_BOUND_PIPELINE);
	VKA_ASSERT(pcSizes.size() == cmdBuf->state.pipelineLayoutDef.pcRanges.size());
    uint32_t readOffset  = 0;
    uint32_t writeOffset = 0;
	for (size_t i = 0; i < pcSizes.size(); i++)
	{
		cmdPushConstants(cmdBuf, cmdBuf->state.pipelineLayoutDef.pcRanges[i].stageFlags, writeOffset, pcSizes[i], (char *) data + readOffset);
		readOffset += pcSizes[i];
		writeOffset += pcSizes[i];
		writeOffset = alignUp(writeOffset, VKA_PUSH_CONSTANT_RANGE_ALLIGNMENT);        // align to 16 bytes just to be safe
	}
}

void cmdDispatch(CmdBuffer cmdBuf, glm::uvec3 workgroups)
{
    VKA_ASSERT(cmdBuf->stateBits & CMD_BUF_STATE_BITS_BOUND_PIPELINE);
    vkCmdDispatch(cmdBuf->getHandle(), workgroups.x, workgroups.y, workgroups.z);
}

void cmdBindVertexBuffers(CmdBuffer cmdBuf)
{
    std::vector<VkBuffer> handels;
    for (size_t i = 0; i < cmdBuf->state.vertexBuffers.size(); i++)
    {
        handels.push_back(cmdBuf->state.vertexBuffers[i]->getHandle());
    }
    std::vector<VkDeviceSize> offset(handels.size());
    vkCmdBindVertexBuffers(cmdBuf->getHandle(), 0, handels.size(), handels.data(), offset.data());
}

void cmdBindIndexBuffer(CmdBuffer cmdBuf, VkDeviceSize offset)
{
	vkCmdBindIndexBuffer(cmdBuf->getHandle(), cmdBuf->state.indexBuffer->getHandle(), offset * 4, VK_INDEX_TYPE_UINT32);
}

void cmdDrawIndexed(CmdBuffer cmdBuf, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t vertexOffset, uint32_t firstInstance)
{
    vkCmdDrawIndexed(cmdBuf->getHandle(), indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

void cmdDraw(CmdBuffer cmdBuf, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance)
{
    vkCmdDraw(cmdBuf->getHandle(), vertexCount, instanceCount, firstVertex, firstInstance);
}
}        // namespace vka