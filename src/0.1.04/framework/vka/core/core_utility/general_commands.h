#pragma once
#include "types.h"
#include <vka/core/resources/IResourcePool.h>
#include <vka/core/resources/Descriptor.h>

namespace vka
{


struct ImageSubresourceRange
{
	uint32_t mipLevel;
	uint32_t baseArrayLayer;
	uint32_t layerCount;
};

void cmdClearState(CmdBuffer cmdBuf);
// Buffer
void cmdCopyBuffer(CmdBuffer cmdBuf, BufferRef src, BufferRef dst);
void cmdCopyBufferRegion(CmdBuffer cmdBuf, BufferRef src, BufferRef dst, uint32_t srcOffset, uint32_t dstOffset, VkDeviceSize size = VK_WHOLE_SIZE);
void cmdUpload(CmdBuffer cmdBuf, Buffer buf);
void cmdWriteCopy(CmdBuffer cmdBuf, Buffer buf, const void *data, VkDeviceSize size);
void cmdUploadCopy(CmdBuffer cmdBuf, Buffer src, Buffer dst);
void cmdFillBuffer(CmdBuffer cmdBuf, Buffer dst, VkDeviceSize offset, VkDeviceSize size, uint32_t data);
void cmdFillBuffer(CmdBuffer cmdBuf, Buffer dst, uint32_t data);
// Image
void cmdImageMemoryBarrier(CmdBuffer cmdBuf, Image image, VkImageLayout newLayout, uint32_t baseLayer = 0, uint32_t layerCount = 1);
void cmdTransitionLayout(CmdBuffer cmdBuf, Image image, VkImageLayout newLayout, uint32_t baseLayer = 0, uint32_t layerCount = 1);
void cmdCopyBufferToImage(CmdBuffer cmdBuf, Buffer src, Image dst, uint32_t layer = 0, uint32_t mipLevel = 0);
void cmdUploadImageData(CmdBuffer cmdBuf, void *data, size_t dataSize, Image dst, VkImageLayout finalLayout, uint32_t layer = 0, uint32_t mipLevel = 0);
void cmdCopyImage(CmdBuffer cmdBuf, Image src, VkImageLayout srcNewLayout, Image dst, VkImageLayout dstNewLayout,
                  ImageSubresourceRange srcSubRange = {0, 0, 1}, ImageSubresourceRange dstSubRange = {0, 0, 1});
void cmdCopyImage(CmdBuffer cmdBuf, Image src, Image dst);

// Acceleration Structure
void cmdBuildAccelerationStructure(CmdBuffer cmdBuf, TLAS dst, Buffer instanceBuffer, Buffer scratchBuffer, VkAccelerationStructureKHR src = VK_NULL_HANDLE);
void cmdBuildAccelerationStructure(CmdBuffer cmdBuf, BLAS dst, Buffer scratchBuffer, VkAccelerationStructureKHR src = VK_NULL_HANDLE);

// General
void cmdBarrier(CmdBuffer cmdBuf, VkPipelineStageFlags srcStage, VkPipelineStageFlags dstStage, VkAccessFlags srcAccesFlags, VkAccessFlags dstAccesFlags);

// Render/Compute
void cmdStartRenderPass(CmdBuffer cmdBuf, VkRenderPass renderpass, VkFramebuffer framebuffer, VkRect2D renderArea, std::vector<VkClearValue> clearValues = {});
void cmdStartRenderPass(CmdBuffer cmdBuf, VkRenderPass renderpass, VkFramebuffer framebuffer, std::vector<VkClearValue> clearValues = {});
void cmdEndRenderPass(CmdBuffer cmdBuf);
void cmdBindPipeline(CmdBuffer cmdBuf);

void cmdPushDescriptors(CmdBuffer cmdBuf, uint32_t setIdx, std::vector<Descriptor> desc);

void cmdPushConstants(CmdBuffer cmdBuf, VkShaderStageFlags shaderStage, uint32_t offset, uint32_t size, const void *data);
void cmdPushConstants(CmdBuffer cmdBuf, const std::vector<uint32_t> &pcSizes, const void *data);
void cmdDispatch(CmdBuffer cmdBuf, glm::uvec3 workgroups);
void cmdBindVertexBuffers(CmdBuffer cmdBuf);
void cmdBindIndexBuffer(CmdBuffer cmdBuf, VkDeviceSize offset);
void cmdDrawIndexed(CmdBuffer cmdBuf, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t vertexOffset, uint32_t firstInstance);
void cmdDraw(CmdBuffer cmdBuf, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance);



}        // namespace vka