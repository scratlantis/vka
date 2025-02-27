#include "ImageEstimatorComparator.h"
#include <vka/advanced_utility/misc_utility.h>
#include <vka/globals.h>
#include <vka/specialized_utility/draw_2D.h>
namespace vka
{
ImageEstimatorComparator::ImageEstimatorComparator(VkFormat format, float relWidth, float relHeight)
{
	Image* imgs[4] = {&localTargetLeft, &localTargetRight, &localAccumulationTargetLeft, &localAccumulationTargetRight};
	for (auto img : imgs)
	{
		*img = createSwapchainAttachment(format,
		                                 VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		                                 VK_IMAGE_LAYOUT_GENERAL, relWidth, relHeight);
	}
	tqManagerLeft  = TimeQueryManager(gState.heap, 1);
	tqManagerRight = TimeQueryManager(gState.heap, 1);
	mseResources = MSEComputeResources(format, gState.heap);
	rmseResources = MSEComputeResources(format, gState.heap);

	mseBuffer    = createBuffer(gState.hostCachedHeap, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_GPU_ONLY, sizeof(float));
	rmseBuffer    = createBuffer(gState.hostCachedHeap, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_GPU_ONLY, sizeof(float));
	mseOverTimeBuffer = createBuffer(gState.heap, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_GPU_ONLY, 100000 * sizeof(float));
	rmseOverTimeBuffer = createBuffer(gState.heap, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_GPU_ONLY, 100000 * sizeof(float));
	isInitialized = true;
}

ImageEstimatorComparator::ImageEstimatorComparator(VkFormat format, VkExtent2D extent)
{
	Image *imgs[4] = {&localTargetLeft, &localTargetRight, &localAccumulationTargetLeft, &localAccumulationTargetRight};
	for (auto img : imgs)
	{
		*img = createImage(gState.heap, format, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, extent);
	}
	tqManagerLeft    = TimeQueryManager(gState.heap, 1);
	tqManagerRight   = TimeQueryManager(gState.heap, 1);
	mseResources = MSEComputeResources(format, gState.heap);
	rmseResources = MSEComputeResources(format, gState.heap);
	mseBuffer        = createBuffer(gState.hostCachedHeap, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_GPU_ONLY, sizeof(float));
	rmseBuffer        = createBuffer(gState.hostCachedHeap, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_GPU_ONLY, sizeof(float));
	mseOverTimeBuffer = createBuffer(gState.heap, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_GPU_ONLY, 100000 * sizeof(float));
	rmseOverTimeBuffer = createBuffer(gState.heap, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_GPU_ONLY, 100000 * sizeof(float));
	isInitialized = true;
}

void ImageEstimatorComparator::garbageCollect()
{
	if (!isInitialized) return;

	for (auto img : {localTargetLeft, localTargetRight, localAccumulationTargetLeft, localAccumulationTargetRight})
	{
		img->garbageCollect();
	}
	tqManagerLeft.garbageCollect();
	tqManagerRight.garbageCollect();
	mseResources.garbageCollect();
	rmseResources.garbageCollect();
	mseBuffer->garbageCollect();
	rmseBuffer->garbageCollect();
	mseOverTimeBuffer->garbageCollect();
	rmseOverTimeBuffer->garbageCollect();
}

void ImageEstimatorComparator::cmdReset(CmdBuffer cmdBuf, IECTarget target)
{
	Image localAccumulationTarget = target == IEC_TARGET_LEFT ? localAccumulationTargetLeft : localAccumulationTargetRight;
	TimeQueryManager &tqManager = target == IEC_TARGET_LEFT ? tqManagerLeft : tqManagerRight;
	Metrics &metrics = target == IEC_TARGET_LEFT ? metricsLeft : metricsRight;
	bool &timeQueryFinished = target == IEC_TARGET_LEFT ? timeQueryFinishedLeft : timeQueryFinishedRight;


	cmdFill(cmdBuf, localAccumulationTarget, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, glm::vec4(0.0));
	tqManager.cmdReset(cmdBuf);
	metrics.reset();
	timeQueryFinished = true;
	mse.clear();
}

void ImageEstimatorComparator::cmdReset(CmdBuffer cmdBuf, Image imgLeft, Image imgRight)
{
	if (imgLeft)
	{
		VKA_ASSERT(imgLeft->getFormat() == localTargetLeft->getFormat());
		VKA_ASSERT(VkExtent2D_OP(imgLeft->getExtent2D()) == VkExtent2D_OP(localTargetLeft->getExtent2D()));
		cmdCopyImage(cmdBuf, imgLeft, imgLeft->getLayout(), localAccumulationTargetLeft, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	}
	else
	{
		cmdFill(cmdBuf, localAccumulationTargetLeft, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, glm::vec4(0.0));
	}
	if (imgRight)
	{
		VKA_ASSERT(imgRight->getFormat() == localTargetRight->getFormat());
		VKA_ASSERT(VkExtent2D_OP(imgRight->getExtent2D()) == VkExtent2D_OP(localTargetRight->getExtent2D()));
		cmdCopyImage(cmdBuf, imgRight, imgRight->getLayout(), localAccumulationTargetRight, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	}
	else
	{
		cmdFill(cmdBuf, localAccumulationTargetRight, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, glm::vec4(0.0));
	}
	tqManagerLeft.cmdReset(cmdBuf);
	tqManagerRight.cmdReset(cmdBuf);
	timeQueryFinishedLeft = true;
	timeQueryFinishedRight = true;
	metricsLeft.reset();
	metricsRight.reset();
	mse.clear();
}



void ImageEstimatorComparator::cmdShow(CmdBuffer cmdBuf, Image target, VkRect2D_OP targetArea, VkImageLayout targetLayout, IECTarget src, IECToneMappingArgs toneMappingArgs)
{
	Image localAccumulationTarget = src == IEC_TARGET_LEFT ? localAccumulationTargetLeft : localAccumulationTargetRight;
	VkRect2D_OP srcArea = VkRect2D_OP(localAccumulationTarget->getExtent2D());
	MapImgArgs args{};
	args.normalize      = true;
	args.useTonemapping = toneMappingArgs.useTonemapping;
	args.whitePoint     = toneMappingArgs.whitePoint;
	args.exposure       = toneMappingArgs.exposure;
	args.useGammaCorrection = toneMappingArgs.useGammaCorrection;
	args.useScissors    = false;
	args.dstLayout      = targetLayout;
	target->setClearValue(ClearValue::black());
	getCmdMapImg(localAccumulationTarget, target, args).exec(cmdBuf);
}

void ImageEstimatorComparator::showSplitView(CmdBuffer cmdBuf, Image target, float splittCoef, VkRect2D_OP targetArea, IECToneMappingArgs toneMappingArgs)
{

	VkRect2D_OP srcAreaLeft = VkRect2D_OP(localAccumulationTargetLeft->getExtent2D());
	srcAreaLeft.extent.width *= splittCoef;
	VkRect2D_OP srcAreaRight = VkRect2D_OP(localAccumulationTargetRight->getExtent2D());
	srcAreaRight.offset.x += srcAreaLeft.extent.width;
	srcAreaRight.extent.width -= srcAreaLeft.extent.width;

	VkRect2D_OP dstAreaLeft = targetArea;
	dstAreaLeft.extent.width *= splittCoef;
	VkRect2D_OP dstAreaRight = targetArea;
	dstAreaRight.offset.x += dstAreaLeft.extent.width;
	dstAreaRight.extent.width -= dstAreaLeft.extent.width;

	// Bar in the middle
	if (dstAreaLeft.extent.width != 0) dstAreaLeft.extent.width -= 1;
	if (dstAreaRight.extent.width != 0) dstAreaRight.extent.width -= 1;
	dstAreaRight.offset.x += 1;

	MapImgArgs args{};
	args.normalize = true;
	args.useTonemapping = toneMappingArgs.useTonemapping;
	args.whitePoint = toneMappingArgs.whitePoint;
	args.exposure = toneMappingArgs.exposure;
	args.useGammaCorrection = toneMappingArgs.useGammaCorrection;
	args.useScissors    = true;
	args.dstLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	if (dstAreaRight.extent.width == 0 || dstAreaRight.extent.width == 0)
	{
		args.srcArea = srcAreaLeft;
		args.dstArea = dstAreaLeft;
		getCmdMapImg(localAccumulationTargetLeft, target, args).exec(cmdBuf);
	}
	else if (dstAreaLeft.extent.width > 0 && srcAreaLeft.extent.width > 0)
	{
		srcAreaLeft.extent.width -= 1;
		srcAreaRight.extent.width -= 1;
		srcAreaRight.offset.x += 1;

		args.srcArea = srcAreaLeft;
		args.dstArea = dstAreaLeft;
		getCmdMapImg(localAccumulationTargetLeft, target, args).exec(cmdBuf);

		args.srcArea = srcAreaRight;
		args.dstArea = dstAreaRight;
		getCmdMapImg(localAccumulationTargetRight, target, args).exec(cmdBuf);
	}
	else
	{
		srcAreaRight.extent.width -= 1;
		srcAreaRight.offset.x += 1;

		args.srcArea = srcAreaRight;
		args.dstArea = dstAreaRight;
		getCmdMapImg(localAccumulationTargetRight, target, args).exec(cmdBuf);
	}
}
void ImageEstimatorComparator::showDiff(CmdBuffer cmdBuf, Image target, VkRect2D_OP targetArea)
{
	getCmdNormalizeDiff(localAccumulationTargetLeft, localAccumulationTargetRight, target,
	                    VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VkRect2D_OP(localAccumulationTargetLeft->getExtent2D()), targetArea)
	    .exec(cmdBuf);
}
void ImageEstimatorComparator::showDiff(CmdBuffer cmdBuf, Image target)
{
	getCmdNormalizeDiff(localAccumulationTargetLeft, localAccumulationTargetRight, target,
	                    VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VkRect2D_OP(localAccumulationTargetLeft->getExtent2D()), VkRect2D_OP(target->getExtent2D()))
	    .exec(cmdBuf);
}

Buffer ImageEstimatorComparator::getMSEBuf()
{
	return mseOverTimeBuffer;
}
Buffer ImageEstimatorComparator::getRMSEBuf()
{
	return rmseOverTimeBuffer;
}
float ImageEstimatorComparator::getMSE()
{
	Buffer hostMseBuf;
	float  mse = 0;
	if (gState.hostCache->fetch(mseBuffer, hostMseBuf))
	{
		mse = *static_cast<float *>(hostMseBuf->map());
	}
	//mse *= 1.0 / (localAccumulationTargetLeft->getExtent2D().width * localAccumulationTargetLeft->getExtent2D().height);
	return mse;
}
float *ImageEstimatorComparator::getMSEData()
{
	return mse.data();
}
uint32_t ImageEstimatorComparator::getMSEDataSize()
{
	return mse.size();
}
}        // namespace vka