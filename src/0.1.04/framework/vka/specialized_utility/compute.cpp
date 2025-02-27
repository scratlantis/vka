#include "compute.h"
#include <vka/globals.h>
namespace vka
{
size_t getReduceValueSize(ReduceValueType type)
{
	switch (type)
	{
		case vka::REDUCE_VALUE_TYPE_FLOAT:
			return sizeof(float);
			break;
		case vka::REDUCE_VALUE_TYPE_UINT32:
			return sizeof(uint32_t);
			break;
		default:
			DEBUG_BREAK;
			return 0;
			break;
	}
}
ComputeCmd getCmdReduceVertical(Image src, Buffer dst, uint32_t segmentCount, ReduceOperation op)
{
	ComputeCmd cmd(glm::uvec2(src->getExtent2D().width, segmentCount), cVkaShaderPath + "reduce_img_to_buffer_vertical.comp",
	               {
	                   {"FORMAT", getGLSLFormat(src->getFormat())},
	                   {"REDUCE_OP", static_cast<uint32_t>(op)},
	               });
	cmd.pushDescriptor(src, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
	cmd.pushDescriptor(dst, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
	struct PushStruct
	{
		uint32_t width;
		uint32_t height;
		uint32_t segmentCount;
	} pc;
	pc.width        = src->getExtent2D().width;
	pc.height       = src->getExtent2D().height;
	pc.segmentCount = segmentCount;
	cmd.pushConstant(&pc, sizeof(PushStruct));
	return cmd;
}

ComputeCmd getCmdReduce(Buffer src, Buffer dst, uint32_t segmentCount, ReduceOperation op, ReduceValueType type)
{
	uint32_t   inputSize       = src->getSize() / getReduceValueSize(type);
	uint32_t   outputSize      = dst->getSize() / getReduceValueSize(type);
	uint32_t   invocationCount = outputSize;
	ComputeCmd cmd(invocationCount, cVkaShaderPath + "reduce_buffer_to_buffer.comp",
	               {
	                   {"REDUCE_OP", static_cast<uint32_t>(op)},
	                   {"REDUCE_VALUE_TYPE", static_cast<uint32_t>(type)},
	               });
	cmd.pushDescriptor(src, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
	cmd.pushDescriptor(dst, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);

	struct PushStruct
	{
		uint32_t inputSize;
		uint32_t outputSize;
	} pc;
	pc.inputSize  = inputSize;
	pc.outputSize = outputSize;
	cmd.pushConstant(&pc, sizeof(PushStruct));
	return cmd;
}
ComputeCmd getCmdDownSample(Image src, Image dst, ReduceOperation op)
{
	ComputeCmd cmd(dst->getExtent2D(), cVkaShaderPath + "downsample.comp",
	               {
	                   {"FORMAT_IN", getGLSLFormat(src->getFormat())},
	                   {"FORMAT_OUT", getGLSLFormat(dst->getFormat())},
	                   {"REDUCE_OP", static_cast<uint32_t>(op)},
	               });
	struct PushStruct
	{
		glm::uvec2 extentIN;
		glm::uvec2 extentOUT;
	} pc;
	pc.extentIN  = glm::uvec2(src->getExtent2D().width, src->getExtent2D().height);
	pc.extentOUT = glm::uvec2(dst->getExtent2D().width, dst->getExtent2D().height);
	cmd.pushConstant(&pc, sizeof(PushStruct));
	cmd.pushDescriptor(src, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
	cmd.pushDescriptor(dst, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
	return cmd;
}

ComputeCmd getCmdReduceHorizontal(Image src, Buffer dst, uint32_t dstOffset, ReduceOperation op)
{
	ComputeCmd cmd(src->getExtent2D().height, cVkaShaderPath + "reduce_img_horizontal.comp",
	               {
	                   {"FORMAT", getGLSLFormat(src->getFormat())},
	                   {"REDUCE_OP", static_cast<uint32_t>(op)},
	               });
	struct PushStruct
	{
		glm::uvec2 extentIN;
		glm::uvec2 offsetIN;
		uint32_t   offsetOUT;
	} pc;
	pc.extentIN  = glm::uvec2(src->getExtent2D().width, src->getExtent2D().height);
	pc.offsetIN  = glm::uvec2(0, 0);
	pc.offsetOUT = dstOffset;
	cmd.pushConstant(&pc, sizeof(PushStruct));
	cmd.pushDescriptor(src, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
	cmd.pushDescriptor(dst, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
	return cmd;
}

ComputeCmd getCmdImageToBufferHorizontal(Image src, Buffer dst, uint32_t dstOffset)
{
	ComputeCmd cmd(src->getExtent2D(), cVkaShaderPath + "img_to_buf_horizontal.comp",
	               {
	                   {"FORMAT", getGLSLFormat(src->getFormat())},
	               });
	struct PushStruct
	{
		glm::uvec2 extentIN;
		glm::uvec2 offsetIN;
		uint32_t   offsetOUT;
	} pc;
	pc.extentIN  = glm::uvec2(src->getExtent2D().width, src->getExtent2D().height);
	pc.offsetIN  = glm::uvec2(0, 0);
	pc.offsetOUT = dstOffset;
	cmd.pushConstant(&pc, sizeof(PushStruct));
	cmd.pushDescriptor(src, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
	cmd.pushDescriptor(dst, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
	return cmd;
}

void cmdComputeImgPdf(CmdBuffer cmdBuf, Image src, Buffer dst, uint32_t divisionsX, uint32_t divisionsY)
{
	uint32_t bufferSize = (divisionsX * divisionsY + divisionsY) * sizeof(float);
	dst->changeSize(bufferSize);
	dst->changeMemoryType(VMA_MEMORY_USAGE_GPU_ONLY);
	dst->addUsage(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
	dst->addUsage(VK_BUFFER_USAGE_TRANSFER_DST_BIT);
	dst->recreate();
	VKA_ASSERT((src->getUsage() & VK_IMAGE_USAGE_STORAGE_BIT) != 0);
	Image tempImg = createImage(gState.frame->stack, VK_FORMAT_R32_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT, VkExtent2D{divisionsX, divisionsY});
	getCmdDownSample(src, tempImg, REDUCE_OP_IMAGE_PDF).exec(cmdBuf);
	cmdBarrier(cmdBuf, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);
	getCmdReduceHorizontal(tempImg, dst, 0, REDUCE_OP_AVERAGE).exec(cmdBuf);
	getCmdImageToBufferHorizontal(tempImg, dst, divisionsY).exec(cmdBuf);
	cmdBarrier(cmdBuf, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);

	//float fval = 1000000.f / 16.f;
	//uint32_t val;
	//memcpy(&val, &fval, sizeof(uint32_t));
	//uint32_t firstPdfSize = divisionsY * sizeof(float);
	//
	//cmdBarrier(cmdBuf, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_TRANSFER_WRITE_BIT);
	//cmdFillBuffer(cmdBuf, dst, firstPdfSize, bufferSize - firstPdfSize, val);
	//cmdBarrier(cmdBuf, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);

	getCmdNormalizeBuffer(dst, 0, divisionsY, 1).exec(cmdBuf);
	getCmdNormalizeBuffer(dst, divisionsY, divisionsX, divisionsY).exec(cmdBuf);
}

ComputeCmd getCmdNormalizeBuffer(Buffer buf, uint32_t offset, uint32_t segmentSize, uint32_t segmentCount)
{
	ComputeCmd cmd(segmentCount, cVkaShaderPath + "normalize_buffer.comp");
	cmd.pushDescriptor(buf, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
	struct PushStruct
	{
		uint32_t offset;
		uint32_t segmentSize;
		uint32_t segmentCount;
	} pc;
	pc.segmentCount = segmentCount;
	pc.segmentSize  = segmentSize;
	pc.offset       = offset;
	cmd.pushConstant(&pc, sizeof(PushStruct));
	return cmd;
}

ComputeCmd getCmdPlot(Buffer src, Image dst, uint32_t offset, glm::vec3 color)
{
	ComputeCmd cmd(dst->getExtent2D(), cVkaShaderPath + "plot.comp",
	               {
	                   {"FORMAT", getGLSLFormat(dst->getFormat())},
	               });
	struct PushStruct
	{
		glm::vec3 color;
		glm::uint width;
		glm::uint height;
		glm::uint offset;
	} pc;
	pc.width  = dst->getExtent2D().width;
	pc.height = dst->getExtent2D().height;
	pc.offset = offset;
	pc.color  = color;
	cmd.pushConstant(&pc, sizeof(PushStruct));
	cmd.pushDescriptor(src, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
	cmd.pushDescriptor(dst, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
	return cmd;
}


ComputeCmd getCmdPerlinNoise(Image target, PerlinNoiseArgs args)
{
	ComputeCmd cmd(target->getExtent(), cVkaShaderPath + "perlin_noise_3D.comp");
	//VKA_ASSERT(target->getFormat() == VK_FORMAT_R32_SFLOAT)
	cmd.pushDescriptor(target, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
	struct PushStruct
	{
		uint32_t seed;
		float    scale;
		float    minVal;
		float    maxVal;
		float    frequency;
		float	 falloffAtEdge;
		float    blendCoef;
		uint32_t blendMode;
	} pc;
	pc.seed         = args.seed;
	pc.scale        = args.scale;
	pc.minVal       = args.min;
	pc.maxVal       = args.max;
	pc.frequency     = args.frequency;
	pc.falloffAtEdge = args.falloffAtEdge ? 1 : 0;
	pc.blendCoef     = args.blendCoef;
	pc.blendMode	 = args.blendMode;
	cmd.pushConstant(&pc, sizeof(PushStruct));
	return cmd;
}

void cmdLoadScalarField(CmdBuffer cmdBuf, Buffer src, Image dst, const ScalarFieldInfo &info)
{
	// should assert 3d image
	dst->changeExtent(info.extent);
	if (dst->recreate())
	{
		cmdTransitionLayout(cmdBuf, dst, VK_IMAGE_LAYOUT_GENERAL);
	}
	ComputeCmd cmd(dst->getExtent(), cVkaShaderPath + "loadScalarField.comp",
	               {
	                   {"FORMAT", getGLSLFormat(dst->getFormat())},
					   {"SCALAR_FIELD_FORMAT", static_cast<uint32_t>(info.format)},
	               });
	float      pc = info.minVal;
	cmd.pushConstant(&pc, sizeof(float));
	cmd.pushDescriptor(src, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
	cmd.pushDescriptor(dst, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
	cmd.exec(cmdBuf);
}

}        // namespace vka