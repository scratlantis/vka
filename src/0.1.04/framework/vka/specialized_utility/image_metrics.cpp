#include "image_metrics.h"
#include "compute.h"
#include "draw_2D.h"

namespace vka
{

void cmdComputeMSE(CmdBuffer cmdBuf, Image srcA, Image srcB, Buffer dst, MSEComputeResources *pMSEResources)
{
	pMSEResources->update(srcA->getExtent2D());
	pMSEResources->diffImage->setClearValue({0.0f, 0.0f, 0.0f, 0.0f});
	// cmdFill(cmdBuf, srcB, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, glm::vec4(1.0));
	getCmdNormalizeSquaredDiff(srcA, srcB, pMSEResources->diffImage, VK_IMAGE_LAYOUT_GENERAL, VkRect2D_OP(srcA->getExtent2D()), VkRect2D_OP(pMSEResources->diffImage->getExtent2D())).exec(cmdBuf);
	//getCmdNormalizeDiff(srcA, srcB, pMSEResources->diffImage, VK_IMAGE_LAYOUT_GENERAL, VkRect2D_OP(srcA->getExtent2D()), VkRect2D_OP(pMSEResources->diffImage->getExtent2D())) .exec(cmdBuf);
	cmdBarrier(cmdBuf, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);

	getCmdReduceVertical(pMSEResources->diffImage, pMSEResources->verticalAverage, 1, REDUCE_OP_AVERAGE).exec(cmdBuf);

	cmdBarrier(cmdBuf, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);

	getCmdReduce(pMSEResources->verticalAverage, pMSEResources->mse, srcA->getExtent2D().height, REDUCE_OP_AVERAGE, REDUCE_VALUE_TYPE_FLOAT).exec(cmdBuf);

	cmdBarrier(cmdBuf, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_TRANSFER_READ_BIT);

	cmdCopyBuffer(cmdBuf, pMSEResources->mse, dst);
}
void cmdComputeRMSE(CmdBuffer cmdBuf, Image srcA, Image srcB, Buffer dst, MSEComputeResources *pMSEResources)
{
	pMSEResources->update(srcA->getExtent2D());
	pMSEResources->diffImage->setClearValue({0.0f, 0.0f, 0.0f, 0.0f});
	// cmdFill(cmdBuf, srcB, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, glm::vec4(1.0));
	// getCmdNormalizeSquaredDiff(srcA, srcB, pMSEResources->diffImage, VK_IMAGE_LAYOUT_GENERAL, VkRect2D_OP(srcA->getExtent2D()), VkRect2D_OP(pMSEResources->diffImage->getExtent2D())).exec(cmdBuf);
	getCmdNormalizeDiff(srcA, srcB, pMSEResources->diffImage, VK_IMAGE_LAYOUT_GENERAL, VkRect2D_OP(srcA->getExtent2D()), VkRect2D_OP(pMSEResources->diffImage->getExtent2D())).exec(cmdBuf);
	cmdBarrier(cmdBuf, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);

	getCmdReduceVertical(pMSEResources->diffImage, pMSEResources->verticalAverage, 1, REDUCE_OP_AVERAGE).exec(cmdBuf);

	cmdBarrier(cmdBuf, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);

	getCmdReduce(pMSEResources->verticalAverage, pMSEResources->mse, srcA->getExtent2D().height, REDUCE_OP_AVERAGE, REDUCE_VALUE_TYPE_FLOAT).exec(cmdBuf);

	cmdBarrier(cmdBuf, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_TRANSFER_READ_BIT);

	cmdCopyBuffer(cmdBuf, pMSEResources->mse, dst);
}

}        // namespace vka