#include "sorting.h"
#include <vka/globals.h>
namespace vka
{
	uint32_t NUMBER_OF_WORKGROUPS = 256;

	ComputeCmd getCmdRadixSort(Buffer bufferIn, Buffer bufferOut, Buffer histogramIn, uint32_t shift,
                               Buffer permutationIn, Buffer permutationOut, uint32_t count)
	{
	    uint32_t       elementCount             = count;
	    ComputeCmd     cmd                      = ComputeCmd(elementCount, NUMBER_OF_WORKGROUPS, cVkaShaderPath + "multi_radixsort.comp", COMPUTE_CMD_FLAG_BIT_DYNAMIC);
		const uint32_t NUM_BLOCKS_PER_WORKGROUP = 32;

		struct PushConstants {
			uint32_t g_num_elements;
			uint32_t g_shift;
			uint32_t g_num_workgroups;
			uint32_t g_num_blocks_per_workgroup;
		} pc;
		pc.g_num_elements = elementCount;
		pc.g_num_workgroups = cmd.workGroupCount.x;
		pc.g_num_blocks_per_workgroup = NUM_BLOCKS_PER_WORKGROUP;
		pc.g_shift = shift;
		cmd.pushConstant(&pc, sizeof(PushConstants));
		cmd.pushDescriptor(bufferIn, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		cmd.pushDescriptor(bufferOut, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		cmd.pushDescriptor(histogramIn, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		if(permutationIn != nullptr && permutationOut != nullptr)
		{
			cmd.pushDescriptor(permutationIn, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
			cmd.pushDescriptor(permutationOut, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
			cmd.pipelineDef.shaderDef.args.push_back({ "STORE_PERMUTATION",""});
		}
		return cmd;
	};
    ComputeCmd getCmdRadixSort(Buffer bufferIn, Buffer bufferOut, Buffer histogramIn, uint32_t shift, uint32_t count)
	{
	    return getCmdRadixSort(bufferIn, bufferOut, histogramIn, shift, nullptr, nullptr, count);
	};

	ComputeCmd getCmdRadixSortHistogram(Buffer bufferIn, Buffer histogramIn, uint32_t shift, uint32_t count)
	{
	    uint32_t       elementCount             = count;
	    ComputeCmd     cmd                      = ComputeCmd(elementCount, 256, cVkaShaderPath + "multi_radixsort_histograms.comp", COMPUTE_CMD_FLAG_BIT_DYNAMIC);
		const uint32_t NUM_BLOCKS_PER_WORKGROUP = 32;

		struct PushConstants {
			uint32_t g_num_elements;
			uint32_t g_shift;
			uint32_t g_num_workgroups;
			uint32_t g_num_blocks_per_workgroup;
		} pc;
		pc.g_num_elements = elementCount;
		pc.g_num_workgroups = cmd.workGroupCount.x;
		pc.g_num_blocks_per_workgroup = NUM_BLOCKS_PER_WORKGROUP;
		pc.g_shift = shift;
		cmd.pushConstant(&pc, sizeof(PushConstants));
		cmd.pushDescriptor(bufferIn, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		cmd.pushDescriptor(histogramIn, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		return cmd;
	};

	void cmdRadixSort(CmdBuffer cmdBuf, Buffer buffer, uint32_t count, Buffer pingPongBuf, Buffer histogramBuf)
	{
		if (pingPongBuf == nullptr)
		{
			pingPongBuf = createBuffer(gState.frame->stack, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
		}
		if (histogramBuf == nullptr)
		{
			histogramBuf = createBuffer(gState.frame->stack, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
		}
		pingPongBuf->addUsage(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
		pingPongBuf->changeSize(buffer->getSize());
		pingPongBuf->recreate();
		histogramBuf->addUsage(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
		cmdInitBuffer(cmdBuf, histogramBuf, 0U, NUMBER_OF_WORKGROUPS * 256 * sizeof(uint32_t));
		cmdBarrier(cmdBuf, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
			VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);

		Buffer& bufA = buffer;
		Buffer& bufB = pingPongBuf;
		const uint32_t NUM_PASSES = 4;
		for (uint32_t pass = 0; pass < NUM_PASSES; pass++)
		{
			uint32_t shift = pass * 8;
		    getCmdRadixSortHistogram(bufA, histogramBuf, shift, count).exec(cmdBuf);
			cmdBarrier(cmdBuf, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
				VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);

			getCmdRadixSort(bufA, bufB, histogramBuf, shift, count).exec(cmdBuf);
			cmdBarrier(cmdBuf, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
				VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);

			// Swap buffers
			std::swap(bufA, bufB);
		}
	}

	void cmdRadixSortPermutation(CmdBuffer cmdBuf, Buffer buffer, Buffer permutationBuffer, uint32_t count,
                                 Buffer pingPongBuf, Buffer permutationPingPongBuf, Buffer histogramBuf)
	{
		uint32_t bufferSize = count * sizeof(uint32_t);
		permutationBuffer->addUsage(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
	    permutationBuffer->changeSize(bufferSize);
		permutationBuffer->recreate();

		if (pingPongBuf == nullptr)
		{
			pingPongBuf = createBuffer(gState.frame->stack, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
		}
		if (permutationPingPongBuf == nullptr)
		{
			permutationPingPongBuf = createBuffer(gState.frame->stack, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
		}
		if (histogramBuf == nullptr)
		{
			histogramBuf = createBuffer(gState.frame->stack, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
		}

		pingPongBuf->addUsage(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
	    pingPongBuf->changeSize(bufferSize);
		pingPongBuf->recreate();

		permutationPingPongBuf->addUsage(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
	    permutationPingPongBuf->changeSize(bufferSize);
		permutationPingPongBuf->recreate();

		histogramBuf->addUsage(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
		cmdInitBuffer(cmdBuf, histogramBuf, 0U, NUMBER_OF_WORKGROUPS * 256 * sizeof(uint32_t));
		cmdBarrier(cmdBuf, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
			VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);

		Buffer& bufA = buffer;
		Buffer& bufB = pingPongBuf;
		Buffer& permA = permutationBuffer;
		Buffer& permB = permutationPingPongBuf;

		const uint32_t NUM_PASSES = 4;
		for (uint32_t pass = 0; pass < NUM_PASSES; pass++)
		{
			uint32_t shift = pass * 8;
		    getCmdRadixSortHistogram(bufA, histogramBuf, shift, count).exec(cmdBuf);
			cmdBarrier(cmdBuf, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
				VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);

			getCmdRadixSort(bufA, bufB, histogramBuf, shift, permA, permB, count).exec(cmdBuf);
			cmdBarrier(cmdBuf, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
				VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);

			// Swap buffers
			std::swap(bufA, bufB);
			std::swap(permA, permB);
		}
	}

}